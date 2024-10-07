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
#include "cpath/private/c_parser.h"
#include "cpath/private/c_folder_file.h"
#include "cpath/c_device.h"

namespace ncore
{
    namespace npath
    {
        static void s_init_devices(alloc_t* allocator, devices_t* devices, instance_t* instance, strings_t* strings)
        {
            s32 const c_extra_devices = 2; // For 'find' and 'temp' slots
            devices->m_num_devices    = 1;
            devices->m_max_devices    = 62;
            devices->m_arr_devices    = g_allocate_array<device_t*>(allocator, devices->m_max_devices + c_extra_devices);
            for (idevice_t i = 0; i < devices->m_max_devices; ++i)
                devices->m_arr_devices[i] = g_construct<device_t>(allocator, instance, c_empty_string, c_empty_node, i);
            for (idevice_t i = devices->m_max_devices; i < devices->m_max_devices + c_extra_devices; ++i)
                devices->m_arr_devices[i] = nullptr;
            devices->m_device_nodes = g_allocate_array<ntree32::nnode_t>(allocator, devices->m_max_devices + c_extra_devices);
            ntree32::setup_tree(devices->m_device_tree, devices->m_device_nodes);
            devices->m_device_tree_root = c_invalid_node;

            device_t* default_device = devices->m_arr_devices[0];
        }

        static void s_exit_devices(alloc_t* allocator, devices_t*& devices)
        {
            for (s32 i = 0; i < devices->m_max_devices; ++i)
                g_destruct(allocator, devices->m_arr_devices[i]);
            g_deallocate_array(allocator, devices->m_arr_devices);
            g_deallocate_array(allocator, devices->m_device_nodes);
            ntree32::teardown_tree(devices->m_device_tree);
            g_destruct(allocator, devices);
            devices = nullptr;
        }

        void instance_t::init(alloc_t* allocator, u32 max_items)
        {
            m_allocator = allocator;

            m_strings = g_construct_strings(allocator);
            string_t default_string = m_strings->insert(make_crunes("nil"));
            ASSERT(default_string == c_empty_string);

            m_folders = g_construct_folders(allocator, max_items);
            m_files   = g_construct_files(allocator, max_items);

            m_devices = g_construct<devices_t>(m_allocator);
            s_init_devices(allocator, m_devices, this, m_strings);
        }

        void instance_t::exit(alloc_t* allocator)
        {
            s_exit_devices(allocator, m_devices);

            g_destruct_files(allocator, m_files);
            g_destruct_folders(allocator, m_folders);
            g_destruct_strings(allocator, m_strings);
            g_destruct(m_allocator, m_strings);
        }

        device_t* instance_t::register_device(crunes_t const& devicename)
        {
            string_t  devicestr = m_strings->insert(devicename);
            idevice_t idevice   = m_devices->register_device(devicestr);
            return m_devices->get_device(idevice);
        }

        node_t instance_t::allocate_folder(string_t name)
        {
            return g_allocate_folder(m_folders, name);
        }

        string_t instance_t::find_string(const crunes_t& namestr) const
        {
            string_t name = m_strings->find(namestr);
            return name;
        }

        string_t instance_t::find_or_insert_string(crunes_t const& namestr)
        {
            string_t name = m_strings->find(namestr);
            if (name == c_invalid_string)
            {
                name = m_strings->insert(namestr);
            }
            return name;
        }

        crunes_t instance_t::get_crunes(string_t _str) const
        {
            crunes_t str;
            m_strings->view_string(_str, str);
            return str;
        }

        void instance_t::to_string(string_t _str, runes_t& _out_str) const
        {
            crunes_t str;
            m_strings->view_string(_str, str);
            nrunes::concatenate(_out_str, str);
        }

        s32 instance_t::to_strlen(string_t str) const
        {
            s32 len = m_strings->get_len(str);
            return len;
        }

        s8 instance_t::compare_str(string_t left, string_t right) const { return m_strings->compare(left, right); }

        dirpath_t instance_t::register_fulldirpath(crunes_t const& fulldirpath)
        {
            // extract device, then init a 'crunes_t path' that contains everything after the device
            // select the first 'folder' from this 'path' and call device->register_dirpath(folder, out_dirpath)

            return dirpath_t(this->m_devices->get_default_device());
        }

        filepath_t instance_t::register_fullfilepath(crunes_t const& fullfilepath)
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
