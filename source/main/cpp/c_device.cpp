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

        // --------------------------------------------------------------------------------------------------------------
        // --------------------------------------------------------------------------------------------------------------
        // device
        // --------------------------------------------------------------------------------------------------------------
        // --------------------------------------------------------------------------------------------------------------

        // Once all devices are registered we can connect them through their path
        void device_t::finalize(devices_t* devices)
        {
            if (m_redirector == c_invalid_device)
            {
                // This is the root device, so it should have a path.
                ASSERT(m_path != c_invalid_node);
                // The path should be a folder_t* with no parent.
                folder_t* folder = m_owner->m_folders->m_array.ptr_of(m_path);
                ASSERT(folder->m_parent == c_invalid_node);
            }
            else
            {
                // The redirector should be a device_t*.
                device_t* redirector = devices->get_device(m_redirector);

                // Navigate up to the top directory of this path, we are going to connect it to the redirector.
                folder_t* folder = m_owner->m_folders->m_array.ptr_of(m_path);
                if (folder != nullptr)
                {
                    while (folder->m_parent != c_invalid_node)
                        folder = m_owner->m_folders->m_array.ptr_of(folder->m_parent);

                    // Chain the path of this device to the redirector.
                    folder->m_parent = redirector->m_path;
                }
                else
                {
                    // This alias doesn't have a path, so we are going to chain the redirector's path to this device.
                    m_path = redirector->m_path;
                }
            }
        }

        // example: projects\binary_reader\bin\, "projects\" -> "binary_reader\" -> "bin\"
        static bool s_next_folder(crunes_t& folder, char slash)
        {
            folder.m_str = folder.m_end;
            folder.m_end = folder.m_eos;
            folder       = nrunes::findSelectUntilIncluded(folder, slash);
            return !is_empty(folder);
        }

        void device_t::register_dirpath(crunes_t const& _dirpath_first_folder, dirpath_t& out_dirpath)
        {
            if (!is_empty(_dirpath_first_folder))
            {
                crunes_t folder      = _dirpath_first_folder;
                node_t   parent_node = c_invalid_node;
                do
                {
                    string_t folder_pathstr = m_owner->find_or_insert_string(folder);
                    node_t   folder_node    = add_dir(parent_node, folder_pathstr);
                    parent_node             = folder_node;
                } while (s_next_folder(folder, '/'));
            }
        }

        void device_t::register_filepath(crunes_t const& _filepath_first_folder, filepath_t& out_filepath)
        {
            if (!is_empty(_filepath_first_folder))
            {
                crunes_t folder      = _filepath_first_folder;
                node_t   parent_node = c_invalid_node;
                do
                {
                    string_t folder_pathstr = m_owner->find_or_insert_string(folder);
                    node_t   folder_node    = add_dir(parent_node, folder_pathstr);
                    parent_node             = folder_node;
                } while (s_next_folder(folder, '/'));

                // Here we are likely left with the filename and extension
                crunes_t fullfilename = make_crunes(folder);
                fullfilename.m_str    = fullfilename.m_end;
                fullfilename.m_end    = fullfilename.m_eos;
                crunes_t filename     = nrunes::findSelectUntil(fullfilename, '.');
                crunes_t extension    = nrunes::selectAfterExclude(fullfilename, filename);
            }
        }

        void device_t::to_string(runes_t& tr) const
        {
            s32             i            = 0;
            idevice_t       device_index = m_index;
            device_t const* devices[32];
            do
            {
                device_t* device             = m_owner->m_devices->m_arr_devices[device_index];
                devices[i++]                 = device;
                idevice_t const device_index = device->m_redirector;
            } while (device_index != c_invalid_device && i < 32);

            device_t const* device = devices[--i];

            // should be the root device (has filedevice), so first emit the device name.
            // this device should not have any device path.
            crunes_t device_str;
            m_owner->m_strings->view_string(device->name(), device_str);
            // TODO append this device's path to the string

            // the rest of the devices are aliases and should be appending their paths
            while (--i >= 0)
            {
                device = devices[i];
                m_owner->m_strings->view_string(device->name(), device_str);
                // TODO append this device's path to the string
            }
        }

        s32 device_t::to_strlen() const
        {
            // our 'path' are connected up to the main device, so we only need to walk the path
            s32       strlen = 0;
            folder_t* folder = m_owner->m_folders->m_array.ptr_of(m_path);
            while (folder->m_parent != c_invalid_node)
            {
                strlen += m_owner->m_strings->get_len(folder->m_name);
            }

            return strlen;
        }

        static s8 s_compare_str_with_folder(u32 find_str, u32 _node_folder, void const* user_data)
        {
            paths_t const* const  root        = (paths_t const*)user_data;
            folder_t const* const node_folder = root->m_folders->m_array.ptr_of(_node_folder);
            return root->m_strings->compare(find_str, node_folder->m_name);
        }

        static s8 s_compare_folder_with_folder(u32 _find_folder, u32 _node_folder, void const* user_data)
        {
            paths_t const* const  root        = (paths_t const*)user_data;
            folder_t const* const find_folder = root->m_folders->m_array.ptr_of(_find_folder);
            folder_t const* const node_folder = root->m_folders->m_array.ptr_of(_node_folder);
            return root->m_strings->compare(find_folder->m_name, node_folder->m_name);
        }

        node_t device_t::add_dir(node_t parent, string_t str)
        {
            // Parent node is a folder_t*, so we need to get the folder_t* and then
            // use the tree of folders to see if there is a folder_t* that holds 'str'.
            // If there is no such folder, then we need to create a new folder_t* and
            // insert it into the tree of folders of parent.
            folder_t* folder     = m_owner->m_folders->m_array.ptr_of(parent);
            node_t    temp_node  = m_owner->m_folders->m_count + 1;
            node_t    found_node = c_invalid_node;
            if (ntree32::insert(m_owner->m_folders->m_tree, folder->m_folders, temp_node, str, s_compare_str_with_folder, this, found_node))
            {
                m_owner->m_folders->m_count += 1;
                m_owner->m_folders->m_array.ensure_capacity(found_node);
                folder_t* new_folder  = m_owner->m_folders->m_array.ptr_of(found_node);
                new_folder->m_name    = str;
                new_folder->m_folders = c_invalid_node;
                new_folder->m_parent  = parent;
            }
            return found_node;
        }

        node_t device_t::get_parent_path(node_t path) const
        {
            folder_t* folder = m_owner->m_folders->m_array.ptr_of(path);
            return folder->m_parent;
        }

        node_t device_t::get_first_child_dir(node_t path) const
        {
            folder_t* folder     = m_owner->m_folders->m_array.ptr_of(path);
            node_t    first_child = ntree32::c_invalid_node;
            if (folder->m_folders != ntree32::c_invalid_node)
            {
                ntree32::iterator_t it = ntree32::iterate(m_owner->m_folders->m_tree, folder->m_folders);
                first_child             = it.m_it;
            }

            return first_child;
        }

        // --------------------------------------------------------------------------------------------------------------
        // --------------------------------------------------------------------------------------------------------------
        // devices
        // --------------------------------------------------------------------------------------------------------------
        // --------------------------------------------------------------------------------------------------------------

        static s8 s_compare_name_with_device(u32 name, u32 device, void const* user_data)
        {
            devices_t const* devices = (devices_t const*)user_data;
            device_t const*  dev     = devices->m_arr_devices[device];
            return dev->m_owner->compare_str(name, dev->name());
        }

        idevice_t devices_t::find_device(string_t devicename) const
        {
            ntree32::node_t found = c_invalid_node;
            if (ntree32::find(m_device_tree, m_device_tree_root, devicename, s_compare_name_with_device, this, found))
            {
                return (idevice_t)found;
            }
            return c_invalid_device;
        }

        idevice_t devices_t::register_device(string_t devicename)
        {
            idevice_t device = find_device(devicename);
            if (device == c_invalid_device)
            {
                ntree32::node_t temp     = m_max_devices + 1;
                ntree32::node_t inserted = ntree32::c_invalid_node;
                if (ntree32::insert(m_device_tree, m_device_tree_root, temp, devicename, s_compare_name_with_device, this, inserted))
                {
                    // node_t    path_node = m_owner->m_folder_tree.new_node();
                    // folder_t* folder    = m_owner->m_folder_array.ptr_of(path_node);
                    // folder->reset();
                    // folder->m_name = devicename;
                    // m_owner->m_folder_count += 1;
                    node_t path_node                      = m_owner->allocate_folder(devicename);
                    m_arr_devices[inserted]->m_owner      = m_owner;
                    m_arr_devices[inserted]->m_name       = devicename;
                    m_arr_devices[inserted]->m_path       = path_node;
                    m_arr_devices[inserted]->m_index      = inserted;
                    m_arr_devices[inserted]->m_redirector = c_invalid_device;
                    m_arr_devices[inserted]->m_userdata1  = 0;
                    m_arr_devices[inserted]->m_userdata2  = 0;

                    return (idevice_t)inserted;
                }
            }
            return device;
        }

        device_t* devices_t::get_device(idevice_t index) const
        {
            if (index == c_invalid_device)
                return nullptr;
            return m_arr_devices[index];
        }

        device_t* devices_t::get_default_device() const { return m_arr_devices[0]; }

        device_t* g_construct_device(alloc_t* allocator, paths_t* owner, idevice_t index)
        {
            device_t* device = g_construct<device_t>(allocator);
            device->m_owner      = owner;
            device->m_name       = c_invalid_string;
            device->m_path       = c_invalid_node;
            device->m_index      = index;
            device->m_redirector = c_invalid_device;
            device->m_userdata1  = 0;
            device->m_userdata2  = 0;
            return device;
        }

        void g_destruct_device(alloc_t* allocator, device_t*& device)
        {
            g_destruct(allocator, device);
            device = nullptr;
        }

        devices_t* g_construct_devices(alloc_t* allocator, paths_t* owner, strings_t* strings)
        {
            devices_t* devices = g_construct<devices_t>(allocator);
            devices->m_owner = owner;
            devices->m_strings = strings;

            s32 const c_extra_devices = 2; // For 'find' and 'temp' slots
            devices->m_num_devices    = 1;
            devices->m_max_devices    = 62;
            devices->m_arr_devices    = g_allocate_array<device_t*>(allocator, devices->m_max_devices + c_extra_devices);
            for (idevice_t i = 0; i < devices->m_max_devices; ++i)
                devices->m_arr_devices[i] = g_construct_device(allocator, owner, i);
            for (idevice_t i = devices->m_max_devices; i < devices->m_max_devices + c_extra_devices; ++i)
                devices->m_arr_devices[i] = nullptr;
            devices->m_device_nodes = g_allocate_array<ntree32::nnode_t>(allocator, devices->m_max_devices + c_extra_devices);
            ntree32::setup_tree(devices->m_device_tree, devices->m_device_nodes);
            devices->m_device_tree_root = c_invalid_node;

            device_t* default_device = devices->m_arr_devices[0];

            return devices;
        }

        void g_destruct_devices(alloc_t* allocator, devices_t*& devices)
        {
            for (idevice_t i = 0; i < devices->m_max_devices; ++i)
                g_destruct(allocator, devices->m_arr_devices[i]);
            g_deallocate_array(allocator, devices->m_arr_devices);
            g_deallocate_array(allocator, devices->m_device_nodes);
            ntree32::teardown_tree(devices->m_device_tree);
            g_destruct(allocator, devices);
            devices = nullptr;
        }

    } // namespace npath
} // namespace ncore
