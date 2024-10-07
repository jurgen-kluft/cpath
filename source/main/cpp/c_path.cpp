#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/c_path.h"
#include "cpath/c_dirpath.h"
#include "cpath/c_filepath.h"
#include "cpath/private/c_strings.h"
#include "cpath/private/c_folders.h"
#include "cpath/c_device.h"

namespace ncore
{
    namespace npath
    {
        paths_t* g_construct_paths(alloc_t* allocator, u32 max_items)
        {
            paths_t* paths     = g_construct<paths_t>(allocator);
            paths->m_allocator = allocator;

            paths->m_strings        = g_construct_strings(allocator);
            string_t default_string = paths->m_strings->insert(make_crunes("nil"));
            ASSERT(default_string == c_empty_string);

            paths->m_devices = g_construct_devices(allocator, paths, paths->m_strings);
            paths->m_folders = g_construct_folders(allocator, max_items);

            return paths;
        }

        void g_destruct_paths(alloc_t* allocator, paths_t*& paths)
        {
            g_destruct_devices(allocator, paths->m_devices);
            g_destruct_folders(allocator, paths->m_folders);
            g_destruct_strings(allocator, paths->m_strings);
            g_destruct(allocator, paths);
        }

        device_t* paths_t::register_device(crunes_t const& devicename)
        {
            string_t  devicestr = m_strings->insert(devicename);
            idevice_t idevice   = m_devices->register_device(devicestr);
            return m_devices->get_device(idevice);
        }

        node_t paths_t::allocate_folder(string_t name) { return g_allocate_folder(m_folders, name); }

        string_t paths_t::find_string(const crunes_t& namestr) const
        {
            string_t name = m_strings->find(namestr);
            return name;
        }

        string_t paths_t::find_or_insert_string(crunes_t const& namestr)
        {
            string_t name = m_strings->find(namestr);
            if (name == c_invalid_string)
            {
                name = m_strings->insert(namestr);
            }
            return name;
        }

        crunes_t paths_t::get_crunes(string_t _str) const
        {
            crunes_t str;
            m_strings->view_string(_str, str);
            return str;
        }

        void paths_t::to_string(string_t _str, runes_t& _out_str) const
        {
            crunes_t str;
            m_strings->view_string(_str, str);
            nrunes::concatenate(_out_str, str);
        }

        s32 paths_t::to_strlen(string_t str) const
        {
            s32 len = m_strings->get_len(str);
            return len;
        }

        s8 paths_t::compare_str(string_t left, string_t right) const { return m_strings->compare(left, right); }
        s8 paths_t::compare_str(folder_t* left, folder_t* right) const { return m_strings->compare(left->m_name, right->m_name); }

        static bool s_next_folder(crunes_t& folder, char slash)
        {
            folder.m_str = folder.m_end;
            folder.m_end = folder.m_eos;
            folder       = nrunes::findSelectUntilIncluded(folder, slash);
            return !is_empty(folder);
        }

        dirpath_t paths_t::register_fulldirpath(crunes_t const& _fulldirpath)
        {
            // extract device, then init a 'crunes_t path' that contains everything after the device
            // select the first 'folder' from this 'path' and call device->register_dirpath(folder, out_dirpath)
            crunes_t fulldirpath = _fulldirpath;
            fulldirpath.m_eos    = fulldirpath.m_end;

            crunes_t devicestr = nrunes::findSelectUntilIncluded(fulldirpath, ':');

            if (is_empty(devicestr))
            {
                return dirpath_t(this->m_devices->get_default_device());
            }

            device_t* device = register_device(devicestr);
            if (device == nullptr)
            {
                return dirpath_t(this->m_devices->get_default_device());
            }

            crunes_t path = nrunes::selectAfterExclude(fulldirpath, devicestr);
            nrunes::trimLeft(path, '/');

            crunes_t firstFolder = nrunes::findSelectUntilIncluded(path, '/');
            node_t   path_node   = c_invalid_node;
            if (!is_empty(firstFolder))
            {
                crunes_t folder      = firstFolder;
                node_t   parent_node = c_invalid_node;
                do
                {
                    string_t folder_pathstr = find_or_insert_string(folder);
                    node_t   folder_node    = device->add_dir(parent_node, folder_pathstr);
                    parent_node             = folder_node;
                    path_node               = folder_node;
                } while (s_next_folder(folder, '/'));
            }
            return dirpath_t(device, path_node);
        }

        filepath_t paths_t::register_fullfilepath(crunes_t const& fullfilepath)
        {
            // extract device, then init a 'crunes_t path' that contains everything after the device
            // select the first 'folder' from this 'path' and call device->register_filepath(folder, out_filepath)

            return filepath_t(this->m_devices->get_default_device());
        }

        //  Objectives:
        //  - Sharing of underlying strings
        //  - Easy manipulation of dirpath, can easily go to parent or child directory (if it exists), without doing any allocations
        //  - You can prime it, which then results in no or a lot less allocations when you are using existing filepaths and/or dirpaths
        //  - Combining dirpath with filepath becomes very straightforward
        //  - No need to deal with forward or backward slash
        //
        //  Use cases:
        //    - dirpath_t appdir = root->device_root("appdir");
        //    - dirpath_t bins = appdir.down("bin") // even if this folder doesn't exist, it will be 'registered' but not 'created on disk'
        //    - filepath_t coolexe = bins.file("cool.exe");
        //    - npath::string_t datafilename; npath::string_t dataextension;
        //    - root->filename("data.txt", datafilename, dataextension);
        //    - filepath_t datafilepath = bins.file(datafilename, dataextension);
        //    - filestream_t datastream = open_filestream(datafilepath); // if this file doesn't exist, it will be created
        //    - .. read from the datastream
        //    - datastream.close();

        // So how do we go about implementing this?
        // The main goal is to share strings, so we are going to need a rb-tree to keep track of all the directory strings, filename strings, and extension strings.
        // When we add a directory to this tree, we will need to split the directory string into parts, and then add each part to the tree.
        // Example:
        //  - "E:\documents\books\sci-fi\", the following parts will be added to the tree:
        //    -> "E:\"
        //    -> "documents\"
        //    -> "books\"
        //    -> "sci-fi\"
        //
        // So when we add a directory like "E:\documents\music", we will need to split the string into parts, and then add each part to the tree starting from the root.
        // This means that we need to be able to refer to a parent, so that when we add "E:\documents\music", we can find "E:\" and then "documents\" and then add "music"
        // with a reference to "documents\".
        //
        // The whole path table is a hierarchical red-black tree.
        // Every 'folder' holds 2 tree roots (files and folders), each 'folder' again
        // has 2 trees (files and folders) and so on. The root folder is the only one
        // that has a 0/null parent.

    } // namespace npath
} // namespace ncore
