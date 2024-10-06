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

namespace ncore
{
    namespace npath
    {
        void root_t::init(alloc_t* allocator, u32 max_items)
        {
            m_allocator = allocator;

            m_devices = g_construct<devices_t>(m_allocator);

            s32 const c_extra_devices = 2; // For 'find' and 'temp' slots
            m_devices->m_max_devices  = 62;
            m_devices->m_arr_devices  = g_allocate_array<device_t*>(m_allocator, m_devices->m_max_devices + c_extra_devices);
            for (s32 i = 0; i < m_devices->m_max_devices; ++i)
                m_devices->m_arr_devices[i] = g_construct<device_t>(m_allocator);
            for (s32 i = m_devices->m_max_devices; i < m_devices->m_max_devices + c_extra_devices; ++i)
                m_devices->m_arr_devices[i] = nullptr;
            m_devices->m_device_nodes       = g_allocate_array<ntree32::tree_t::nnode_t>(m_allocator, m_devices->m_max_devices + c_extra_devices);
            m_devices->m_device_node_colors = g_allocate_array<u8>(m_allocator, (m_devices->m_max_devices + c_extra_devices + 31) >> 5);
            ntree32::setup_tree(m_devices->m_device_tree, m_devices->m_device_nodes, m_devices->m_device_node_colors);

            m_strings = g_construct<strings_t>(m_allocator);
            m_strings->init(max_items);

            m_folder_nodes = g_construct<tree_t>(m_allocator);
            m_folder_nodes->init();

            m_folders.init(max_items / 1024, max_items);
        }

        void root_t::exit(alloc_t* allocator)
        {
            for (s32 i = 0; i < m_devices->m_max_devices; ++i)
                g_destruct(m_allocator, m_devices->m_arr_devices[i]);
            g_deallocate_array(m_allocator, m_devices->m_arr_devices);
            g_deallocate_array(m_allocator, m_devices->m_device_nodes);
            g_deallocate_array(m_allocator, m_devices->m_device_node_colors);
            ntree32::teardown_tree(m_devices->m_device_tree);

            m_folders.exit();
            m_folder_nodes->exit();
            m_strings->exit();

            g_destruct(m_allocator, m_folder_nodes);
            g_destruct(m_allocator, m_strings);
        }

        string_t root_t::find_string(const crunes_t& namestr) const
        {
            string_t name = m_strings->find(namestr);
            return name;
        }

        string_t root_t::find_or_insert_string(crunes_t const& namestr)
        {
            string_t name = m_strings->find(namestr);
            if (name == c_invalid_string)
            {
                name = m_strings->insert(namestr);
            }
            return name;
        }

        crunes_t root_t::get_crunes(string_t _str) const
        {
            utf8::pcrune str;
            u32          len;
            m_strings->view_string(_str, str, len);
            return make_crunes(str, 0, len, len);
        }

        void root_t::to_string(string_t str, runes_t& out_str) const
        {
            utf8::pcrune r;
            u32          len;
            m_strings->view_string(str, r, len);
            crunes_t cr = make_crunes(r, 0, len, len);
            nrunes::concatenate(out_str, cr);
        }

        s32 root_t::to_strlen(string_t str) const
        {
            s32 len = m_strings->get_len(str);
            return len;
        }

        s32 root_t::compare_str(string_t left, string_t right) const { return m_strings->compare(left, right); }

        void root_t::register_fulldirpath(crunes_t const& fulldirpath, string_t& outdevicename, node_t& outnode)
        {
            npath::parser_t parser;
            parser.parse(fulldirpath);

            outdevicename = 0;
            if (parser.has_device())
            {
                outdevicename = find_or_insert_string(parser.m_device);
            }

            outnode = 0;
            if (parser.has_path())
            {
                crunes_t folder      = parser.iterate_folder();
                node_t   parent_node = 0;
                do
                {
                    string_t folder_pathstr = find_or_insert_string(folder);
                    node_t   folder_node    = find_or_insert_path(parent_node, folder_pathstr);
                    parent_node             = folder_node;
                } while (parser.next_folder(folder));
                outnode = parent_node;
            }
        }

        void root_t::register_dirpath(crunes_t const& dirpath, node_t& outnode)
        {
            npath::parser_t parser;
            parser.parse(dirpath);

            outnode = 0;
            if (parser.has_path())
            {
                crunes_t folder      = parser.iterate_folder();
                node_t   parent_node = c_invalid_node;
                do
                {
                    string_t folder_pathstr = find_or_insert_string(folder);
                    node_t   folder_node    = find_or_insert_path(parent_node, folder_pathstr);
                    parent_node             = folder_node;
                } while (parser.next_folder(folder));
                outnode = parent_node;
            }
        }

        void root_t::register_filename(crunes_t const& namestr, string_t& out_filename, string_t& out_extension)
        {
            crunes_t filename_str   = namestr;
            filename_str            = nrunes::findLastSelectUntil(filename_str, '.');
            crunes_t extension_str  = nrunes::selectAfterExclude(namestr, filename_str);
            string_t filename_name  = find_or_insert_string(filename_str);
            string_t extension_name = find_or_insert_string(extension_str);
            out_filename            = filename_name;
            out_extension           = extension_name;
        }

        void root_t::register_fullfilepath(crunes_t const& fullfilepath, string_t& out_device, node_t& out_path, string_t& out_filename, string_t& out_extension)
        {
            npath::parser_t parser;
            parser.parse(fullfilepath);

            out_device = c_invalid_device;
            out_path   = c_invalid_node;
            if (parser.has_device())
            {
                register_fulldirpath(parser.deviceAndPath(), out_device, out_path);
            }
            else if (parser.has_path())
            {
                register_dirpath(parser.path(), out_path);
            }

            out_filename = c_invalid_string;
            if (parser.has_filename())
            {
                out_filename = find_or_insert_string(parser.m_filename);
            }

            out_extension = c_invalid_string;
            if (parser.has_extension())
            {
                out_extension = find_or_insert_string(parser.m_extension);
            }
        }

        static s8 s_device_compare(u32 find_item, u32 node_item, void const* user_data)
        {
            devices_t const* data   = (devices_t const*)user_data;
            device_t*        device = data->m_arr_devices[node_item];
            if (device->m_deviceName == find_item)
                return 0;
            return find_item < device->m_deviceName ? -1 : 1;
        }

        idevice_t root_t::find_device(string_t devicename) const
        {
            ntree32::node_t found = c_invalid_node;
            if (ntree32::find(m_devices->m_device_tree, m_devices->m_device_tree_root, devicename, s_device_compare, m_devices, found))
            {
                return (idevice_t)found;
            }
            return c_invalid_device;
        }

        idevice_t root_t::register_device(string_t devicename)
        {
            idevice_t device = find_device(devicename);
            if (device == c_invalid_device)
            {
                ntree32::node_t temp     = m_devices->m_max_devices + 1;
                ntree32::node_t inserted = ntree32::c_invalid_node;
                if (ntree32::insert(m_devices->m_device_tree, temp, m_devices->m_device_tree_root, devicename, s_device_compare, m_devices, inserted))
                {
                    m_devices->m_arr_devices[inserted]->m_root       = this;
                    m_devices->m_arr_devices[inserted]->m_alias      = c_invalid_string;
                    m_devices->m_arr_devices[inserted]->m_deviceName = devicename;
                    m_devices->m_arr_devices[inserted]->m_devicePath = c_invalid_node;
                    m_devices->m_arr_devices[inserted]->m_redirector = c_invalid_device;
                    m_devices->m_arr_devices[inserted]->m_userdata1  = 0;
                    m_devices->m_arr_devices[inserted]->m_userdata2  = 0;
                    return (idevice_t)inserted;
                }
            }
            return device;
        }

        device_t* root_t::get_device(string_t devicename) const
        {
            idevice_t const device = find_device(devicename);
            if (device != c_invalid_device)
            {
                return m_devices->m_arr_devices[device];
            }
            return nullptr;
        }

        device_t* root_t::get_device(idevice_t index) const
        {
            if (index == c_invalid_device)
                return nullptr;
            return m_devices->m_arr_devices[index];
        }

        string_t  root_t::attach_pathstr(string_t name) { return name; }
        node_t    root_t::attach_pathnode(node_t path) { return path; }
        idevice_t root_t::attach_pathdevice(idevice_t idevice) { return idevice; }
        device_t* root_t::attach_pathdevice(device_t* device) { return device; }
        string_t  root_t::release_pathstr(string_t name) { return c_invalid_string; }
        node_t    root_t::release_pathnode(node_t path) { return c_invalid_node; }
        idevice_t root_t::release_pathdevice(idevice_t idevice) { return c_invalid_device; }
        idevice_t root_t::release_pathdevice(device_t* device) { return c_invalid_device; }

        device_t* root_t::get_pathdevice(dirpath_t const& dirpath) { return dirpath.m_device; }
        device_t* root_t::get_pathdevice(filepath_t const& filepath) { return filepath.m_dirpath.m_device; }
        node_t    root_t::get_path(dirpath_t const& dirpath) { return dirpath.m_path; }
        node_t    root_t::get_path(filepath_t const& filepath) { return filepath.m_dirpath.m_path; }
        string_t  root_t::get_filename(filepath_t const& filepath) { return filepath.m_filename; }
        string_t  root_t::get_extension(filepath_t const& filepath) { return filepath.m_extension; }
        root_t*   root_t::get_root(dirpath_t const& dirpath) { return dirpath.m_device->m_root; }
        root_t*   root_t::get_root(filepath_t const& filepath) { return filepath.m_dirpath.m_device->m_root; }

        // void root_t::filepath(const char* str, filepath_t& fp)
        // {
        //     crunes_t filepathstr(str);
        //     filepath(filepathstr, fp);
        // }

        // void root_t::dirpath(const char* str, dirpath_t& dp)
        // {
        //     crunes_t dirpathstr(str);
        //     dirpath(dirpathstr, dp);
        // }

        // void root_t::filepath(const crunes_t& str, filepath_t& fp)
        // {
        //     string_t devicename = 0;
        //     node_t   path       = 0;
        //     string_t filename   = 0;
        //     string_t extension  = 0;
        //     register_fullfilepath(str, devicename, path, filename, extension);

        //     idevice_t const  device  = register_device(devicename);
        //     device_t*  pdevice = get_device(device);
        //     filepath_t filepath(pdevice, path, filename, extension);
        //     fp = filepath;
        // }

        // void root_t::dirpath(const crunes_t& str, dirpath_t& dp)
        // {
        //     string_t devicename = 0;
        //     node_t   path       = 0;
        //     register_fulldirpath(str, devicename, path);
        //     idevice_t const device  = register_device(devicename);
        //     device_t* pdevice = get_device(device);
        //     dirpath_t dirpath(pdevice, path);
        //     dp = dirpath;
        // }

        bool root_t::has_device(const crunes_t& device_name)
        {
            string_t devname = find_string(device_name);
            if (devname != c_invalid_string)
            {
                idevice_t const dev = find_device(devname);
                return dev != c_invalid_device;
            }
            return false;
        }

        bool root_t::register_alias(const crunes_t& aliasstr, const crunes_t& devpathstr)
        {
            string_t aliasname = find_or_insert_string(aliasstr);
            string_t devname   = c_invalid_string;
            node_t   devpath   = c_invalid_node;
            register_fulldirpath(devpathstr, devname, devpath);

            idevice_t const device = register_device(aliasname);
            device_t*       alias  = get_device(device);
            alias->m_alias         = aliasname;
            alias->m_deviceName    = devname;
            alias->m_devicePath    = devpath;
            alias->m_redirector    = register_device(devname);

            return true;
        }

        // -------------------------------------------------------------------------------------------
        //
        // device_t implementations
        //
        void device_t::init(root_t* owner)
        {
            m_root       = owner;
            m_alias      = c_invalid_string;
            m_deviceName = c_invalid_string;
            m_devicePath = c_invalid_node;
            m_redirector = c_invalid_device;
            m_userdata1  = 0;
            m_userdata2  = 0;
        }

        device_t* device_t::construct(alloc_t* allocator, root_t* owner)
        {
            // void*     device_mem = allocator->allocate(sizeof(device_t), sizeof(void*));
            // device_t* device     = static_cast<device_t*>(device_mem);
            device_t* device = g_construct<device_t>(allocator);
            device->init(owner);
            return device;
        }

        void device_t::destruct(alloc_t* allocator, device_t*& device)
        {
            g_destruct(allocator, device);
            device = nullptr;
        }

        device_t* device_t::attach()
        {
            m_root->attach_pathstr(m_alias);
            m_root->attach_pathstr(m_deviceName);
            m_root->attach_pathstr(m_devicePath);
            if (m_redirector != -1)
            {
                device_t* redirector = m_root->m_devices->m_arr_devices[m_redirector];
                redirector->attach();
            }
            return this;
        }

        bool device_t::detach()
        {
            m_root->release_pathstr(m_alias);
            m_root->release_pathstr(m_deviceName);
            m_root->release_pathstr(m_devicePath);
            m_alias      = c_invalid_string;
            m_deviceName = c_invalid_string;
            m_devicePath = c_invalid_node;

            m_root->release_pathdevice(m_redirector);
            m_userdata1 = 0;
            m_userdata2 = 0;

            return false;
        }

        s32 device_t::compare(device_t* device) const
        {
            if (m_deviceName == device->m_deviceName)
                return 0;
            else if (m_deviceName < device->m_deviceName)
                return -1;
            return 1;
        }

        void device_t::to_string(runes_t& str) const
        {
            s32             i            = 0;
            idevice_t       device_index = m_device_index;
            device_t const* devices[32];
            do
            {
                device_t* device             = m_root->m_devices->m_arr_devices[device_index];
                devices[i++]                 = device;
                idevice_t const device_index = device->m_redirector;
            } while (device_index != c_invalid_device && i < 32);

            device_t const* device = devices[--i];

            // should be the root device (has filedevice), so first emit the device name.
            // this device should not have any device path.
            utf8::pcrune device_str;
            u32          device_strlen;
            m_root->m_strings->view_string(device->m_deviceName, device_str, device_strlen);
            crunes_t device_str_crunes = make_crunes(device_str, 0, device_strlen, device_strlen);
            // TODO append this device's path to the string

            // the rest of the devices are aliases and should be appending their paths
            while (--i >= 0)
            {
                device = devices[i];
                m_root->m_strings->view_string(device->m_deviceName, device_str, device_strlen);
                device_str_crunes = make_crunes(device_str, 0, device_strlen, device_strlen);
                // TODO append this device's path to the string
            }
        }

        s32 device_t::to_strlen() const
        {
            s32       i            = 0;
            idevice_t device_index = m_device_index;
            idevice_t devices[32];
            do
            {
                device_t* device = m_root->m_devices->m_arr_devices[device_index];
                devices[i++]     = device_index;
                device_index     = device->m_redirector;
            } while (device_index != c_invalid_device && i < 32);

            device_t const* device = m_root->m_devices->m_arr_devices[devices[--i]];

            // should be the root device (has filedevice), so first emit the device name.
            // this device should not have any device path.

            s32 len = m_root->m_strings->get_len(device->m_deviceName);
            len += 2; // for the ":\" or ":\"

            // the rest of the devices are aliases and should be appending their paths
            while (--i >= 0)
            {
                device = m_root->m_devices->m_arr_devices[devices[i]];
                len += m_root->m_strings->get_len(device->m_devicePath);
                len += 1; // for the "\"
            }
            return len;
        }

        static s8 s_compare_str_with_folder(u32 find_str, u32 _node_folder, void const* user_data)
        {
            root_t const* const   root        = (root_t const*)user_data;
            folder_t const* const node_folder = root->m_folders.ptr_of(_node_folder);
            return root->m_strings->compare(find_str, node_folder->m_name);
        }

        static s8 s_compare_folder_with_folder(u32 _find_folder, u32 _node_folder, void const* user_data)
        {
            root_t const* const   root        = (root_t const*)user_data;
            folder_t const* const find_folder = root->m_folders.ptr_of(_find_folder);
            folder_t const* const node_folder = root->m_folders.ptr_of(_node_folder);
            return root->m_strings->compare(find_folder->m_name, node_folder->m_name);
        }

        node_t root_t::find_or_insert_path(node_t parent, string_t str)
        {
            // Parent node is a folder_t*, so we need to get the folder_t* and then
            // use the tree of folders to see if there is a folder_t* that holds 'str'.
            // If there is no such folder, then we need to create a new folder_t* and
            // insert it into the tree of folders of parent.
            folder_t* folder = m_folders.ptr_of(parent);
            node_t found = c_invalid_node;
            if (m_folder_nodes->insert(folder->m_folders, str, s_compare_str_with_folder, this, found))
            {
                m_folders.ensure_capacity(found);

                folder_t* new_folder  = m_folders.ptr_of(found);
                new_folder->m_name    = str;
                new_folder->m_folders = c_invalid_node;
                new_folder->m_files   = c_invalid_node;
                new_folder->m_parent  = parent;
            }
            return found;
        }

        node_t root_t::get_parent_path(node_t path) const
        {
            folder_t* folder = m_folders.ptr_of(path);
            return folder->m_parent;
        }

        node_t root_t::get_first_childpath(node_t path) const
        {
            // todo
            return c_invalid_node;
        }

        node_t root_t::get_next_childpath(node_t path) const
        {
            // todo
            return c_invalid_node;
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

    } // namespace npath
} // namespace ncore
