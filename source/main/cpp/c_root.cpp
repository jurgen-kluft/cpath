#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/c_dirpath.h"
#include "cpath/c_filepath.h"
#include "cpath/private/c_root.h"
#include "cpath/private/c_strings.h"
#include "cpath/private/c_parser.h"

namespace ncore
{
    namespace npath
    {
        device_t* root_t::sNilDevice;

        void root_t::init(alloc_t* allocator, u32 max_items)
        {
            m_allocator = allocator;

            m_max_devices = 64;
            m_num_devices = 1;
            m_arr_devices = (device_t**)m_allocator->allocate(m_max_devices * sizeof(device_t*));
            for (s32 i = 0; i < m_max_devices; ++i)
            {
                m_arr_devices[i] = (device_t*)m_allocator->allocate(sizeof(device_t), sizeof(void*));
            }

            sNilDevice               = m_arr_devices[0];
            sNilDevice->m_root       = this;
            sNilDevice->m_alias      = 0;
            sNilDevice->m_deviceName = 0;
            sNilDevice->m_devicePath = 0;
            sNilDevice->m_redirector = 0;
            sNilDevice->m_userdata1  = 0;
            sNilDevice->m_userdata2  = 0;

            m_strings = (strings_t*)m_allocator->allocate(sizeof(strings_t), sizeof(void*));
            m_strings->init(max_items);

            m_nodes = (tree_t*)m_allocator->allocate(sizeof(tree_t), sizeof(void*));
            m_nodes->init();

            m_folders.init();
        }

        void root_t::exit(alloc_t* allocator)
        {
            sNilDevice = nullptr;
            for (s32 i = 0; i < m_num_devices; ++i)
            {
                allocator->deallocate(m_arr_devices[i]);
            }
            allocator->deallocate(m_arr_devices);
            m_num_devices = 0;

            m_folders.exit();
            m_nodes->exit();
            m_strings->exit();

            allocator->deallocate(m_nodes);
            allocator->deallocate(m_strings);
        }

        string_t root_t::find_string(const crunes_t& namestr) const
        {
            string_t name = 0;

            return name;
        }

        string_t root_t::find_or_insert_string(crunes_t const& namestr)
        {
            string_t name = 0;

            return name;
        }

        crunes_t root_t::get_crunes(string_t _str) const
        {
            utf8::pcrune str;
            u32          len;
            m_strings->view_string(_str, str, len);
            return crunes_t(str, 0, len, len);
        }

        void root_t::to_string(string_t str, runes_t& out_str) const
        {
            utf8::pcrune r;
            u32          len;
            m_strings->view_string(str, r, len);
            crunes_t cr(r, 0, len, len);
            nrunes::concatenate(out_str, cr);
        }

        s32 root_t::to_strlen(string_t str) const
        {
            s32 len = m_strings->get_len(str);
            return len;
        }

        s32 root_t::compare_str(string_t left, string_t right) const { return m_strings->compare(left, right); }

        void root_t::register_name(crunes_t const& namestr, string_t& outname)
        {
            // TODO register this string at m_strings
            outname = 0;
        }

        void root_t::register_fulldirpath(crunes_t const& fulldirpath, string_t& outdevicename, node_t& outnode)
        {
            npath::parser_t parser;
            parser.parse(fulldirpath);

            outdevicename = 0;
            if (parser.has_device())
            {
                outdevicename = this->find_or_insert_string(parser.m_device);
            }

            outnode = 0;
            if (parser.has_path())
            {
                crunes_t folder      = parser.iterate_folder();
                node_t   parent_node = 0;
                do
                {
                    string_t folder_pathstr = this->find_or_insert_string(folder);
                    node_t   folder_node    = this->find_or_insert_path(parent_node, folder_pathstr);
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
                node_t   parent_node = 0;
                do
                {
                    string_t folder_pathstr = this->find_or_insert_string(folder);
                    node_t   folder_node    = this->find_or_insert_path(parent_node, folder_pathstr);
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
            string_t filename_name  = this->find_or_insert_string(filename_str);
            string_t extension_name = this->find_or_insert_string(extension_str);
            out_filename            = filename_name;
            out_extension           = extension_name;
        }

        void root_t::register_fullfilepath(crunes_t const& fullfilepath, string_t& out_device, node_t& out_path, string_t& out_filename, string_t& out_extension)
        {
            npath::parser_t parser;
            parser.parse(fullfilepath);

            out_device = 0;
            out_path   = 0;
            if (parser.has_device())
            {
                register_fulldirpath(parser.deviceAndPath(), out_device, out_path);
            }
            else if (parser.has_path())
            {
                register_dirpath(parser.path(), out_path);
            }

            out_filename = 0;
            if (parser.has_filename())
            {
                register_name(parser.m_filename, out_filename);
            }

            out_extension = 0;
            if (parser.has_extension())
            {
                register_name(parser.m_extension, out_extension);
            }
        }

        s16 root_t::find_device(string_t devicename) const
        {
            for (s16 i = 0; i < m_num_devices; ++i)
            {
                if (m_arr_devices[i]->m_deviceName == devicename)
                {
                    return i;
                }
            }
            return -1;
        }

        s16 root_t::register_device(string_t devicename)
        {
            s16 device = find_device(devicename);
            if (device == -1)
            {
                if (m_num_devices < 64)
                {
                    m_arr_devices[m_num_devices]->m_root       = this;
                    m_arr_devices[m_num_devices]->m_alias      = 0;
                    m_arr_devices[m_num_devices]->m_deviceName = devicename;
                    m_arr_devices[m_num_devices]->m_devicePath = 0;
                    m_arr_devices[m_num_devices]->m_redirector = 0;
                    m_arr_devices[m_num_devices]->m_userdata1  = 0;
                    m_arr_devices[m_num_devices]->m_userdata2  = 0;
                    device                                     = m_num_devices;
                    m_num_devices++;
                }
            }
            return device;
        }

        device_t* root_t::get_device(s16 index)
        {
            if (index == -1)
                return nullptr;
            return m_arr_devices[index];
        }

        node_t root_t::get_parent_path(node_t path)
        {
            // get folder_t*, then return the parent
            return 0;
        }

        void root_t::resolve(filepath_t const& fp, device_t*& device, node_t& dir, string_t& filename, string_t& extension)
        {
            device    = get_pathdevice(fp);
            dir       = get_path(fp);
            filename  = get_filename(fp);
            extension = get_extension(fp);
        }

        void root_t::resolve(dirpath_t const& dp, device_t*& device, node_t& dir)
        {
            device = get_pathdevice(dp);
            dir    = get_path(dp);
        }

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

        //     s16 const  device  = register_device(devicename);
        //     device_t*  pdevice = get_device(device);
        //     filepath_t filepath(pdevice, path, filename, extension);
        //     fp = filepath;
        // }

        // void root_t::dirpath(const crunes_t& str, dirpath_t& dp)
        // {
        //     string_t devicename = 0;
        //     node_t   path       = 0;
        //     register_fulldirpath(str, devicename, path);
        //     s16 const device  = register_device(devicename);
        //     device_t* pdevice = get_device(device);
        //     dirpath_t dirpath(pdevice, path);
        //     dp = dirpath;
        // }

        bool root_t::has_device(const crunes_t& device_name)
        {
            string_t devname = find_string(device_name);
            if (devname != 0)
            {
                s16 const dev = find_device(devname);
                return dev >= 0;
            }
            return false;
        }

        bool root_t::register_userdata1(const crunes_t& devpathstr, s32 userdata1)
        {
            string_t devname = 0;
            node_t   devpath = 0;
            register_fulldirpath(devpathstr, devname, devpath);

            s16 const device  = register_device(devname);
            device_t* dev     = get_device(device);
            dev->m_devicePath = devpath;
            if (dev->m_userdata1 == -1)
            {
                dev->m_userdata1 = userdata1;
            }
            return true;
        }

        bool root_t::register_userdata2(const crunes_t& devpathstr, s32 userdata2)
        {
            string_t devname = 0;
            node_t   devpath = 0;
            register_fulldirpath(devpathstr, devname, devpath);

            s16 const device  = register_device(devname);
            device_t* dev     = get_device(device);
            dev->m_devicePath = devpath;
            if (dev->m_userdata2 == -1)
            {
                dev->m_userdata2 = userdata2;
            }
            return true;
        }

        bool root_t::register_alias(const crunes_t& aliasstr, const crunes_t& devpathstr)
        {
            string_t aliasname;
            register_name(aliasstr, aliasname);

            string_t devname = 0;
            node_t   devpath = 0;
            register_fulldirpath(devpathstr, devname, devpath);

            s16 const device    = register_device(aliasname);
            device_t* alias     = get_device(device);
            alias->m_alias      = aliasname;
            alias->m_deviceName = devname;
            alias->m_devicePath = devpath;
            alias->m_redirector = register_device(devname);

            return true;
        }

        // -------------------------------------------------------------------------------------------
        //
        // device_t implementations
        //
        void device_t::init(root_t* owner)
        {
            m_root       = owner;
            m_alias      = 0;
            m_deviceName = 0;
            m_devicePath = 0;
            m_redirector = 0;
            m_userdata1  = 0;
            m_userdata2  = 0;
        }

        device_t* device_t::construct(alloc_t* allocator, root_t* owner)
        {
            void*     device_mem = allocator->allocate(sizeof(device_t), sizeof(void*));
            device_t* device     = static_cast<device_t*>(device_mem);
            device->init(owner);
            return device;
        }

        void device_t::destruct(alloc_t* allocator, device_t*& device)
        {
            allocator->deallocate(device);
            device = nullptr;
        }

        device_t* device_t::attach()
        {
            m_root->attach_pathstr(m_alias);
            m_root->attach_pathstr(m_deviceName);
            m_root->attach_pathstr(m_devicePath);
            if (m_redirector != -1)
            {
                device_t* redirector = m_root->m_arr_devices[m_redirector];
                redirector->attach();
            }
            return this;
        }

        bool device_t::detach()
        {
            m_root->release_pathstr(m_alias);
            m_root->release_pathstr(m_deviceName);
            m_root->release_pathstr(m_devicePath);
            m_alias      = 0;
            m_deviceName = 0;
            m_devicePath = 0;

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
            s16             device_index = m_device_index;
            device_t const* devices[32];
            do
            {
                device_t* device       = m_root->m_arr_devices[device_index];
                devices[i++]           = device;
                s16 const device_index = device->m_redirector;
            } while (device_index > 0 && i < 32);

            device_t const* device = devices[--i];

            // should be the root device (has filedevice), so first emit the device name.
            // this device should not have any device path.
            utf8::pcrune device_str;
            u32          device_strlen;
            m_root->m_strings->view_string(device->m_deviceName, device_str, device_strlen);
            crunes_t device_str_crunes(device_str, 0, device_strlen, device_strlen);
            // TODO append this device's path to the string

            // the rest of the devices are aliases and should be appending their paths
            while (--i >= 0)
            {
                device = devices[i];
                m_root->m_strings->view_string(device->m_deviceName, device_str, device_strlen);
                device_str_crunes = crunes_t(device_str, 0, device_strlen, device_strlen);
                // TODO append this device's path to the string
            }
        }

        s32 device_t::to_strlen() const
        {
            s32             i            = 0;
            s16             device_index = m_device_index;
            device_t const* devices[32];
            do
            {
                device_t* device       = m_root->m_arr_devices[device_index];
                devices[i++]           = device;
                s16 const device_index = device->m_redirector;
            } while (device_index > 0 && i < 32);

            device_t const* device = devices[--i];

            // should be the root device (has filedevice), so first emit the device name.
            // this device should not have any device path.

            s32 len = m_root->m_strings->get_len(device->m_deviceName);
            len += 2; // for the ":\" or ":\"

            // the rest of the devices are aliases and should be appending their paths
            while (--i >= 0)
            {
                device = devices[i];
                len += m_root->m_strings->get_len(device->m_devicePath);
                len += 1; // for the "\"
            }
            return len;
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
