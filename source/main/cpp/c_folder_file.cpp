#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/c_path.h"
#include "cpath/private/c_strings.h"
#include "cpath/private/c_folder_file.h"

namespace ncore
{
    namespace npath
    {
        folders_t* g_construct_folders(alloc_t* allocator, u32 max_items)
        {
            folders_t* f = g_construct<folders_t>(allocator);
            f->m_count   = 1;
            f->m_array.init(8192, max_items);
            f->m_nodes.init(8192, max_items);
            ntree32::setup_tree(f->m_tree, (ntree32::nnode_t*)f->m_nodes.ptr());
            ntree32::node_t default_folder_node = f->m_tree.new_node();
            ASSERT(default_folder_node == c_empty_folder);
            folder_t* default_folder = f->m_array.ptr_of(c_empty_folder);
            default_folder->reset();
            return f;
        }

        void g_destruct_folders(alloc_t* allocator, folders_t*& folders)
        {
            folders->m_array.exit();
            folders->m_nodes.exit();
            ntree32::teardown_tree(folders->m_tree);
            g_destruct(allocator, folders);
            folders = nullptr;
        }

        node_t g_allocate_folder(folders_t* folders, string_t name)
        {
            node_t    path_node = folders->m_tree.new_node();
            folder_t* folder    = folders->m_array.ptr_of(path_node);
            folder->m_parent    = c_invalid_folder;
            folder->m_name      = name;
            folder->m_folders   = c_invalid_node;
            folder->m_files     = c_invalid_node;
            folders->m_count += 1;
            return path_node;
        }


        // files_t* g_construct_files(alloc_t* allocator, u32 max_items)
        // {
        //     files_t* f = g_construct<files_t>(allocator);
        //     f->m_count   = 1;
        //     f->m_array.init(8192, max_items);
        //     f->m_nodes.init(8192, max_items);
        //     ntree32::setup_tree(f->m_tree, (ntree32::nnode_t*)f->m_nodes.ptr());
        //     ntree32::node_t default_folder_node = f->m_tree.new_node();
        //     ASSERT(default_folder_node == c_empty_folder);
        //     file_t* default_file = f->m_array.ptr_of(c_empty_folder);
        //     default_file->m_filename  = c_empty_string;
        //     default_file->m_extension = c_empty_string;
        //     return f;
        // }

        // void g_destruct_files(alloc_t* allocator, files_t*& files)
        // {
        //     files->m_array.exit();
        //     files->m_nodes.exit();
        //     ntree32::teardown_tree(files->m_tree);
        //     g_destruct(allocator, files);
        //     files = nullptr;
        // }

        // node_t     g_allocate_file(files_t* files, string_t filename, string_t extension)
        // {
        //     node_t    path_node = files->m_tree.new_node();
        //     file_t*   file      = files->m_array.ptr_of(path_node);
        //     file->m_filename    = filename;
        //     file->m_extension   = extension;
        //     files->m_count += 1;
        //     return path_node;
        // }

    } // namespace npath
} // namespace ncore
