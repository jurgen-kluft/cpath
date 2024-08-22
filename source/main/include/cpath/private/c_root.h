#ifndef __C_PATH_PATH_REG_H__
#define __C_PATH_PATH_REG_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_allocator.h"
#include "cbase/c_runes.h"

#include "cpath/private/c_strings.h"
#include "cpath/private/c_freelist.h"
#include "cpath/private/c_tree.h"

namespace ncore
{
    class filepath_t;
    class dirpath_t;

    namespace npath
    {
        struct device_t;

        typedef u32 ifolder_t;
        struct folder_t
        {
            istring_t m_parent;  // folder parent (index into m_folder_array)
            istring_t m_name;    // folder name (index into m_strings)
            inode_t   m_files;   // Tree of files (tree node index)
            inode_t   m_folders; // Tree of folders (tree node index)
            void      reset()
            {
                m_parent  = 0;
                m_name    = 0;
                m_files   = 0;
                m_folders = 0;
            }
        };

        struct root_t
        {
            void init(alloc_t* allocator, u32 cap = 1024 * 1024);
            void exit(alloc_t* allocator);

            // -----------------------------------------------------------
            void resolve(filepath_t const&, device_t*& device, ifolder_t& dir, istring_t& filename, istring_t& extension);
            void resolve(dirpath_t const&, device_t*& device, ifolder_t& dir);

            // -----------------------------------------------------------
            void register_fulldirpath(crunes_t const& fulldirpath, istring_t& out_devicename, ifolder_t& out_path);
            void register_fullfilepath(crunes_t const& fullfilepath, istring_t& out_devicename, ifolder_t& out_path, istring_t& out_filename, istring_t& out_extension);
            void register_dirpath(crunes_t const& dirpath, ifolder_t& out_path);
            void register_filename(crunes_t const& namestr, istring_t& filename, istring_t& extension);
            void register_name(crunes_t const& namestr, istring_t& name);
            bool register_userdata1(const crunes_t& devpathstr, s32 userdata1);
            bool register_userdata2(const crunes_t& devpathstr, s32 userdata2);
            bool register_alias(const crunes_t& aliasstr, const crunes_t& devpathstr);

            device_t* get_pathdevice(dirpath_t const& dirpath);
            device_t* get_pathdevice(filepath_t const& filepath);
            inode_t   get_path(dirpath_t const& dirpath);
            inode_t   get_path(filepath_t const& filepath);
            istring_t get_filename(filepath_t const& filepath);
            istring_t get_extension(filepath_t const& filepath);
            root_t*   get_root(dirpath_t const& dirpath);
            root_t*   get_root(filepath_t const& filepath);

            bool      has_device(const crunes_t& device_name);
            s16       register_device(crunes_t const& device);
            s16       register_device(istring_t device);
            s16       find_device(istring_t devicename) const;
            device_t* get_device(s16 index);

            void attach_pathstr(istring_t name);
            void attach_pathnode(inode_t path);
            void attach_pathdevice(s16 idevice);
            void attach_pathdevice(device_t* device);

            istring_t release_pathstr(istring_t name);
            inode_t   release_pathnode(inode_t path);
            s16       release_pathdevice(s16 idevice);
            s16       release_pathdevice(device_t* device);

            ifolder_t get_parent_path(ifolder_t path);
            istring_t find_string(crunes_t const& str);
            istring_t findOrInsert(crunes_t const& str);
            bool      remove(istring_t item);

            ifolder_t findOrInsert(ifolder_t parent, istring_t str);
            bool      remove(folder_t* item);
            void      to_string(istring_t* str, runes_t& out_str) const;
            s32       to_strlen(folder_t* str) const;
            s32       compare_str(istring_t left, istring_t right) const { return m_strings->compare(left, right); }
            s32       compare_str(folder_t* left, folder_t* right) const { return compare_str(left->m_name, right->m_name); }

            DCORE_CLASS_PLACEMENT_NEW_DELETE

            // -----------------------------------------------------------
            //
            u32                  m_max_path_objects;
            char                 m_default_slash;
            alloc_t*             m_allocator;
            s32                  m_num_devices;
            s32                  m_max_devices;
            device_t**           m_arr_devices;
            strings_t*           m_strings;
            tree_t*              m_nodes;
            freelist_t<folder_t> m_folders; // Virtual memory array of folder_t[]

            static device_t* sNilDevice;
            static inode_t   sNilNode;
            static istring_t sNilStr;
            static ifolder_t sNilFolder;
        };

        struct device_t
        {
            inline device_t() : m_root(nullptr), m_alias(0), m_deviceName(0), m_devicePath(0), m_redirector(0), m_userdata1(0), m_userdata2(0) {}

            void      init(root_t* owner);
            device_t* construct(alloc_t* allocator, root_t* owner);
            void      destruct(alloc_t* allocator, device_t*& device);
            device_t* attach();
            bool      detach();
            s32       compare(device_t* device) const;
            void      to_string(runes_t& str) const;
            s32       to_strlen() const;

            root_t*   m_root;
            istring_t m_alias;        // an alias redirection (e.g. "data")
            istring_t m_deviceName;   // "[appdir:\]data\bin.pc\", "[data:\]files\" to "[appdir:\]data\bin.pc\files\"
            inode_t   m_devicePath;   // "appdir:\[data\bin.pc\]", "data:\[files\]" to "appdir:\[data\bin.pc\files\]"
            s16       m_device_index; // index into m_pathreg->m_arr_devices
            s16       m_redirector;   // If device path can point to another device_t
            s32       m_userdata1;    //
            s32       m_userdata2;    //
        };

    } // namespace npath

    // The whole path table is a red-black tree.
    // Every 'folder' has siblings (files and folders), each 'folder' sibling again
    // has siblings (files and folders) and so on. The root folder is the only one
    // that has no siblings. The root folder is the only one that has a null parent.
    // This means using a red-black tree we can have trees within trees within trees.
    //

} // namespace ncore

#endif // __C_PATH_PATH_REG_H__
