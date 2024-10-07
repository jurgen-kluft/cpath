#ifndef __C_PATH_DEVICE_H__
#define __C_PATH_DEVICE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "cbase/c_tree32.h"

#include "cpath/c_types.h"

namespace ncore
{
    namespace npath
    {
        struct device_t
        {
            device_t(paths_t* owner, string_t name, node_t path, idevice_t index);

            inline string_t name() const { return m_name; }

            void finalize(devices_t* devices);

            void register_dirpath(crunes_t const& dirpath, dirpath_t& out_dirpath);
            void register_filepath(crunes_t const& filepath, filepath_t& out_filepath);

            node_t get_parent_path(node_t path) const;
            node_t get_first_child_dir(node_t path) const;
            node_t get_first_child_file(node_t path) const;
            node_t add_dir(node_t current_dir, string_t dir);
            node_t add_file(node_t current_dir, string_t file);

            void to_string(runes_t& str) const;
            s32  to_strlen() const;
            s32  to_strlen(node_t path) const;

            DCORE_CLASS_PLACEMENT_NEW_DELETE

            paths_t* m_owner;
            string_t    m_name;       // name (e.g. "appdir")
            node_t      m_path;       // [folder_t] path (e.g. "data\bin.pc" or "e:\")
            idevice_t   m_index;      // index into m_pathreg->m_arr_devices
            idevice_t   m_redirector; // -> device("e:\")
            s32         m_userdata1;  //
            s32         m_userdata2;  //
        };

        struct devices_t
        {
            idevice_t find_device(string_t device_name) const;
            idevice_t register_device(string_t device_name);
            device_t* get_device(idevice_t index) const;
            device_t* get_default_device() const;

            paths_t*       m_owner;
            strings_t*        m_strings;
            device_t**        m_arr_devices;
            ntree32::nnode_t* m_device_nodes;
            ntree32::tree_t   m_device_tree;
            ntree32::node_t   m_device_tree_root;
            s32               m_num_devices;
            s32               m_max_devices;
            DCORE_CLASS_PLACEMENT_NEW_DELETE
        };

    } // namespace npath

    // The whole path table is a hierarchical red-black tree.
    // Every 'folder' holds 2 tree roots (files and folders), each 'folder' again
    // has 2 trees (files and folders) and so on. The root folder is the only one
    // that has a 0/null parent.

} // namespace ncore

#endif // __C_PATH_PATH_REG_H__
