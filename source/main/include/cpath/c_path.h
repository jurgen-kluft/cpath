#ifndef __C_PATH_PATH_REG_H__
#define __C_PATH_PATH_REG_H__
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
        struct paths_t
        {
            // -----------------------------------------------------------
            // device registration
            device_t* register_device(crunes_t const& devicename);
            node_t    allocate_folder(string_t name);

            // -----------------------------------------------------------
            void   register_filename(crunes_t const& filename, string_t& out_name, string_t& out_ext);

            // -----------------------------------------------------------
            dirpath_t  register_fulldirpath(crunes_t const& fulldirpath);
            filepath_t register_fullfilepath(crunes_t const& fullfilepath);

            // -----------------------------------------------------------
            string_t unregister_string(string_t str);

            // -----------------------------------------------------------
            string_t find_string(crunes_t const& str) const;
            string_t find_or_insert_string(crunes_t const& str);

            s8 compare_str(string_t left, string_t right) const;
            s8 compare_str(folder_t* left, folder_t* right) const;

            crunes_t get_crunes(string_t str) const;
            void     to_string(string_t str, runes_t& out_str) const;
            s32      to_strlen(string_t str) const;

            DCORE_CLASS_PLACEMENT_NEW_DELETE

            // -----------------------------------------------------------
            //
            alloc_t*   m_allocator;
            strings_t* m_strings;
            devices_t* m_devices;
            folders_t* m_folders;
        };

        paths_t* g_construct_paths(alloc_t* allocator, u32 max_items = 1024 * 1024 * 1024);
        void     g_destruct_paths(alloc_t* allocator, paths_t*& paths);

    } // namespace npath

} // namespace ncore

#endif // __C_PATH_PATH_REG_H__
