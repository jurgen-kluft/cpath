#ifndef __C_PATH_FOLDER_FILE_H__
#define __C_PATH_FOLDER_FILE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cbase/c_allocator.h"
#include "cbase/c_tree32.h"

#include "cpath/private/c_strings.h"
#include "cpath/private/c_memory.h"
#include "cpath/c_types.h"

namespace ncore
{
    namespace npath
    {
        struct folder_t
        {
            ifolder_t m_parent;  // folder parent (index into m_folder_array)
            string_t  m_name;    // folder name
            node_t    m_folders; // sub folders (tree root node)
            void      reset()
            {
                m_parent  = c_invalid_folder;
                m_name    = c_empty_string;
                m_folders = c_invalid_node;
            }
        };

        struct folders_t
        {
            ntree32::tree_t           m_tree;
            u32                       m_count;
            vpool_t<ntree32::nnode_t> m_nodes;
            vpool_t<folder_t>         m_array;
            DCORE_CLASS_PLACEMENT_NEW_DELETE
        };

        folders_t* g_construct_folders(alloc_t* allocator, u32 max_items);
        void       g_destruct_folders(alloc_t* allocator, folders_t*& folders);
        node_t     g_allocate_folder(folders_t* folders, string_t name);

        // typedef u32 ifile_t;
        // struct file_t
        // {
        //     string_t m_filename;  // file name
        //     string_t m_extension; // file extension
        // };

        // struct files_t
        // {
        //     ntree32::tree_t             m_tree; // And extensions
        //     u32                         m_count;
        //     vpool_t<ntree32::nnode_t> m_nodes;
        //     vpool_t<file_t>           m_array; // A file consists of a filename and an extension
        //     DCORE_CLASS_PLACEMENT_NEW_DELETE
        // };

        // files_t* g_construct_files(alloc_t* allocator, u32 max_items);
        // void     g_destruct_files(alloc_t* allocator, files_t*& files);
        // node_t   g_allocate_file(files_t* folders, string_t filename, string_t extension);

    } // namespace npath
} // namespace ncore

#endif
