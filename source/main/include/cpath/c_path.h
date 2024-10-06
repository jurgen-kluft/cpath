#ifndef __C_PATH_PATH_REG_H__
#define __C_PATH_PATH_REG_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cpath/private/c_freelist.h"
#include "cpath/c_filepath.h"

namespace ncore
{
    class alloc_t;

    namespace npath
    {
        const s16 c_invalid_device = -1;
        const u32 c_invalid_string = 0xFFFFFFFF;
        const u32 c_invalid_node   = 0xFFFFFFFF;
        const u32 c_invalid_folder = 0xFFFFFFFF;

        struct device_t;
        typedef s16 idevice_t;

        struct strings_t;
        typedef u32 string_t;

        struct tree_t;
        typedef u32 node_t;

        typedef u32 ifolder_t;
        struct folder_t
        {
            ifolder_t m_parent;  // folder parent (index into m_folder_array)
            string_t  m_name;    // folder name
            node_t    m_folders; // Tree of folders (tree root node)
            node_t    m_files;   // Tree of files (tree root node)
            void      reset()
            {
                m_parent  = c_invalid_folder;
                m_name    = c_invalid_string;
                m_folders = c_invalid_node;
                m_files   = c_invalid_node;
            }
        };

        struct root_t
        {
            void init(alloc_t* allocator, u32 cap = 1024 * 1024);
            void exit(alloc_t* allocator);

            // -----------------------------------------------------------
            void register_fulldirpath(crunes_t const& fulldirpath, string_t& out_devicename, node_t& out_path);
            void register_fullfilepath(crunes_t const& fullfilepath, string_t& out_devicename, node_t& out_path, string_t& out_filename, string_t& out_extension);

            // -----------------------------------------------------------
            void      register_dirpath(crunes_t const& dirpath, node_t& out_path);
            void      register_filename(crunes_t const& namestr, string_t& filename, string_t& extension);
            bool      register_alias(const crunes_t& aliasstr, const crunes_t& devpathstr);
            idevice_t register_device(string_t device);
            bool      has_device(const crunes_t& device_name);
            idevice_t find_device(string_t devicename) const;
            device_t* get_device(string_t devicename) const;
            device_t* get_device(idevice_t index) const;

            device_t* get_pathdevice(dirpath_t const& dirpath);
            device_t* get_pathdevice(filepath_t const& filepath);
            node_t    get_path(dirpath_t const& dirpath);
            node_t    get_path(filepath_t const& filepath);
            string_t  get_filename(filepath_t const& filepath);
            string_t  get_extension(filepath_t const& filepath);
            root_t*   get_root(dirpath_t const& dirpath);
            root_t*   get_root(filepath_t const& filepath);

            string_t  attach_pathstr(string_t name);
            node_t    attach_pathnode(node_t path);
            idevice_t attach_pathdevice(idevice_t idevice);
            device_t* attach_pathdevice(device_t* device);

            string_t  release_pathstr(string_t name);
            node_t    release_pathnode(node_t path);
            idevice_t release_pathdevice(idevice_t idevice);
            idevice_t release_pathdevice(device_t* device);

            string_t find_string(crunes_t const& str) const;
            string_t find_or_insert_string(crunes_t const& str);
            crunes_t get_crunes(string_t str) const;
            void     to_string(string_t str, runes_t& out_str) const;
            s32      to_strlen(string_t str) const;
            s32      compare_str(string_t left, string_t right) const;
            s32      compare_str(folder_t* left, folder_t* right) const { return compare_str(left->m_name, right->m_name); }

            node_t get_parent_path(node_t path);
            node_t get_first_childpath(node_t path);
            node_t get_next_childpath(node_t path);
            node_t find_or_insert_path(node_t parent, string_t str);
            s32    get_path_strlen(node_t path) const;
            bool   remove_path(node_t item);

            // -----------------------------------------------------------
            //
            u32                  m_max_path_objects;
            char                 m_default_slash;
            alloc_t*             m_allocator;
            strings_t*           m_strings;
            tree_t*              m_nodes;
            freelist_t<folder_t> m_folders;
            devices_t*           m_devices;
        };

        struct devices_t
        {
            device_t**                m_arr_devices;
            ntree32::tree_t::nnode_t* m_device_nodes;
            u8*                       m_device_node_colors;
            ntree32::tree_t           m_device_tree;
            node_t                    m_device_tree_root;
            s32                       m_max_devices;
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

            DCORE_CLASS_PLACEMENT_NEW_DELETE

            root_t*   m_root;
            string_t  m_alias;        // an alias redirection (e.g. "data")
            string_t  m_deviceName;   // "[appdir:\]data\bin.pc\", "[data:\]files\" to "[appdir:\]data\bin.pc\files\"
            node_t    m_devicePath;   // "appdir:\[data\bin.pc\]", "data:\[files\]" to "appdir:\[data\bin.pc\files\]"
            idevice_t m_device_index; // index into m_pathreg->m_arr_devices
            idevice_t m_redirector;   // If device path can point to another device_t
            s32       m_userdata1;    //
            s32       m_userdata2;    //
        };

    } // namespace npath

    // The whole path table is a hierarchical red-black tree.
    // Every 'folder' holds 2 tree roots (files and folders), each 'folder' again
    // has 2 trees (files and folders) and so on. The root folder is the only one
    // that has a 0/null parent.

} // namespace ncore

#endif // __C_PATH_PATH_REG_H__
