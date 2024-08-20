#ifndef __C_PATH_PATH_DB_H__
#define __C_PATH_PATH_DB_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cpath/private/c_bst.h"
#include "cpath/private/c_pathstr_db.h"
#include "cpath/private/c_virtual_array.h"

namespace ncore
{
    class alloc_t;

    class filesystem_t;
    class filedevice_t;
    class pathreg_t;

    //==============================================================================
    // dirpath_t:
    //		- Relative:		"FolderA\FolderB\"
    //		- Absolute:		"Device:\FolderA\FolderB\"
    //
    // Root                     = "Device:\"
    // Parent                   = "Device:\FolderA\"
    // Dir                      = "\FolderA\FolderB\"
    //==============================================================================

    //==============================================================================
    // filepath_t:
    //		- Relative:		"FolderA\FolderB\Filename.ext"
    //		- Absolute:		"Device:\FolderA\FolderB\Filename.ext"
    //
    // Root                     = "Device:\"
    // Parent                   = "Device:\FolderA\"
    // Dir                      = "\FolderA\FolderB\Filename.ext"
    // Filename                 = "Filename.ext"
    // FilenameWithoutExtension = "Filename"
    //==============================================================================

    class filepath_t;
    class dirpath_t;
    struct filesysroot_t;
    struct pathdevice_t;

    // The whole path table should become a red-black tree and not a hash table.
    // Every 'folder' has siblings (files and folders), each 'folder' sibling again
    // has siblings (files and folders) and so on. The root folder is the only one
    // that has no siblings. The root folder is the only one that has a null parent.
    // This means using a red-black tree we can have trees within trees within trees, and
    // with a hash table it would be harder to handle 'sizing' of each sub table.
    //
    struct pathdb_t
    {
        pathdb_t();

        enum EType
        {
            kNil    = 0x00,
            kFolder = 0x01,
            kFile   = 0x02,
            kString = 0x03,
            kMask   = 0x0F,
        };

        struct folder_t
        {
            u32      m_parent;  // folder parent (index into m_folder_array)
            u32      m_name;    // folder name (index into m_str_array)
            rbnode_t m_files;   // Tree of files (tree node index)
            rbnode_t m_folders; // Tree of folders (tree node index)
            void     reset()
            {
                m_parent  = 0;
                m_name    = 0;
                m_files   = 0;
                m_folders = 0;
            }
        };

        void          init(pathstr_db_t* strings, alloc_t* allocator, u32 cap = 1024 * 1024);
        void          exit(alloc_t* allocator);
        folder_t*     attach(folder_t* node) { return node; }
        pathstr_t     attach(pathstr_t node) { return node; }
        void          detach(folder_t* node) {}
        pathstr_t     detach(pathstr_t node) {}
        pathdevice_t* attach(pathdevice_t* device) { return device; }
        void          detach(pathdevice_t* device) {}

        pathstr_t findOrInsert(crunes_t const& str);
        bool      remove(pathstr_t item);
        folder_t* findOrInsert(folder_t* parent, pathstr_t* str);
        bool      remove(folder_t* item);
        void      to_string(pathstr_t* str, runes_t& out_str) const;
        s32       to_strlen(folder_t* str) const;
        s32       compare_str(pathstr_t left, pathstr_t right) const { return m_strings->compare(left, right); }
        s32       compare_str(folder_t* left, folder_t* right) const { return compare_str(left->m_name, right->m_name); }

        pathstr_db_t*              m_strings;
        rbtree_t                   m_nodes;
        virtual_array_t<pathstr_t> m_file_array;   // Virtual memory array of pathstr_t[]
        virtual_array_t<folder_t>  m_folder_array; // Virtual memory array of folder_t[]
    };

    typedef u32 rbnode_t;

    struct pathdevice_t
    {
        inline pathdevice_t() : m_pathreg(nullptr), m_pathdb(nullptr), m_alias(0), m_deviceName(0), m_devicePath(0), m_redirector(0), m_userdata1(0), m_userdata2(0) {}

        void          init(pathreg_t* owner);
        pathdevice_t* construct(alloc_t* allocator, pathreg_t* owner);
        void          destruct(alloc_t* allocator, pathdevice_t*& device);
        pathdevice_t* attach();
        bool          detach();
        s32           compare(pathdevice_t* device) const;
        void          to_string(runes_t& str) const;
        s32           to_strlen() const;

        pathreg_t* m_pathreg;
        pathdb_t*  m_pathdb;
        pathstr_t  m_alias;        // an alias redirection (e.g. "data")
        pathstr_t  m_deviceName;   // "[appdir:\]data\bin.pc\", "[data:\]files\" to "[appdir:\]data\bin.pc\files\"
        rbnode_t   m_devicePath;   // "appdir:\[data\bin.pc\]", "data:\[files\]" to "appdir:\[data\bin.pc\files\]"
        s16        m_device_index; // index into m_pathreg->m_arr_devices
        s16        m_redirector;   // If device path can point to another pathdevice_t
        u32        m_userdata1;    //
        u32        m_userdata2;    //
    };
}; // namespace ncore

#endif
