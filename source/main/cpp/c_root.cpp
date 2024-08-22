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
#include "cpath/private/c_pathparser.h"

namespace ncore
{
    namespace npath
    {
        // -------------------------------------------------------------------------------------------
        //
        // root_t functions
        //
        /*
            Every folder has a name, has a parent folder (except for the root folder), and has a bst that
            contains all the child folders and files. The bst is sorted by the name of the file/folder.
        */
        device_t* root_t::sNilDevice;

        void root_t::init(alloc_t* allocator, u32 cap = 1024 * 1024)
        {
            m_allocator = allocator;

            m_max_devices = 64;
            m_num_devices = 1;
            m_arr_devices = (device_t**)m_allocator->allocate(m_max_devices * sizeof(device_t*));
            for (s32 i = 0; i < m_max_devices; ++i)
            {
                m_arr_devices[i] = m_allocator->construct<device_t>(m_allocator);
            }

            sNilDevice               = m_arr_devices[0];
            sNilDevice->m_root       = this;
            sNilDevice->m_alias      = 0;
            sNilDevice->m_deviceName = 0;
            sNilDevice->m_devicePath = 0;
            sNilDevice->m_redirector = 0;
            sNilDevice->m_userdata1  = 0;
            sNilDevice->m_userdata2  = 0;

            m_strings = m_allocator->construct<strings_t>(m_allocator);
            m_strings->init(m_allocator, cap);
            m_folders.init(m_allocator);
        }

        void root_t::exit(alloc_t* allocator)
        {
            for (s32 i = 0; i < m_num_devices; ++i)
            {
                release_pathstr(m_arr_devices[i]->m_alias);
                release_pathstr(m_arr_devices[i]->m_deviceName);
                release_pathdevice(m_arr_devices[i]->m_devicePath);
                m_arr_devices[i]->m_root       = nullptr;
                m_arr_devices[i]->m_alias      = 0;
                m_arr_devices[i]->m_deviceName = 0;
                m_arr_devices[i]->m_devicePath = 0;
                m_arr_devices[i]->m_redirector = 0;
                m_arr_devices[i]->m_userdata1  = 0;
                m_arr_devices[i]->m_userdata2  = 0;
            }
            m_num_devices = 0;
        }

        istring_t root_t::findOrInsert(crunes_t const& namestr)
        {
            istring_t name = 0;

            return name;
        }

        void root_t::register_name(crunes_t const& namestr, istring_t& outname)
        {
            // TODO register this string at m_strings
            outname = 0;
        }

        void root_t::register_fulldirpath(crunes_t const& fulldirpath, istring_t& outdevicename, inode_t& outnode)
        {
            pathparser_t parser;
            parser.parse(fulldirpath);

            outdevicename = 0;
            if (parser.has_device())
            {
                outdevicename = this->findOrInsert(parser.m_device);
            }

            outnode = 0;
            if (parser.has_path())
            {
                crunes_t folder      = parser.iterate_folder();
                inode_t  parent_node = 0;
                do
                {
                    istring_t folder_pathstr = this->findOrInsert(folder);
                    inode_t   folder_node    = this->findOrInsert(parent_node, folder_pathstr);
                    parent_node              = folder_node;
                } while (parser.next_folder(folder));
                outnode = parent_node;
            }
        }

        void root_t::register_dirpath(crunes_t const& dirpath, inode_t& outnode)
        {
            pathparser_t parser;
            parser.parse(dirpath);

            outnode = 0;
            if (parser.has_path())
            {
                crunes_t folder      = parser.iterate_folder();
                inode_t  parent_node = 0;
                do
                {
                    istring_t folder_pathstr = this->findOrInsert(folder);
                    inode_t   folder_node    = this->findOrInsert(parent_node, folder_pathstr);
                    parent_node              = folder_node;
                } while (parser.next_folder(folder));
                outnode = parent_node;
            }
        }

        void root_t::register_filename(crunes_t const& namestr, istring_t& out_filename, istring_t& out_extension)
        {
            crunes_t filename_str    = namestr;
            filename_str             = nrunes::findLastSelectUntil(filename_str, '.');
            crunes_t  extension_str  = nrunes::selectAfterExclude(namestr, filename_str);
            istring_t filename_name  = this->findOrInsert(filename_str);
            istring_t extension_name = this->findOrInsert(extension_str);
            out_filename             = filename_name;
            out_extension            = extension_name;
        }

        void root_t::register_fullfilepath(crunes_t const& fullfilepath, istring_t& out_device, inode_t& out_path, istring_t& out_filename, istring_t& out_extension)
        {
            pathparser_t parser;
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

        s16 root_t::find_device(istring_t devicename) const
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

        s16 root_t::register_device(istring_t devicename)
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

        inode_t root_t::get_parent_path(inode_t path)
        {
            // get folder_t*, then return the parent
            return 0;
        }

        void root_t::resolve(filepath_t const& fp, device_t*& device, inode_t& dir, istring_t& filename, istring_t& extension)
        {
            device    = get_pathdevice(fp);
            dir       = get_path(fp);
            filename  = get_filename(fp);
            extension = get_extension(fp);
        }

        void root_t::resolve(dirpath_t const& dp, device_t*& device, inode_t& dir)
        {
            device = get_pathdevice(dp);
            dir    = get_path(dp);
        }

        device_t* root_t::get_pathdevice(dirpath_t const& dirpath) { return dirpath.m_device; }
        device_t* root_t::get_pathdevice(filepath_t const& filepath) { return filepath.m_dirpath.m_device; }
        inode_t   root_t::get_path(dirpath_t const& dirpath) { return dirpath.m_path; }
        inode_t   root_t::get_path(filepath_t const& filepath) { return filepath.m_dirpath.m_path; }
        istring_t root_t::get_filename(filepath_t const& filepath) { return filepath.m_filename; }
        istring_t root_t::get_extension(filepath_t const& filepath) { return filepath.m_extension; }
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
        //     istring_t devicename = 0;
        //     inode_t   path       = 0;
        //     istring_t filename   = 0;
        //     istring_t extension  = 0;
        //     register_fullfilepath(str, devicename, path, filename, extension);

        //     s16 const  device  = register_device(devicename);
        //     device_t*  pdevice = get_device(device);
        //     filepath_t filepath(pdevice, path, filename, extension);
        //     fp = filepath;
        // }

        // void root_t::dirpath(const crunes_t& str, dirpath_t& dp)
        // {
        //     istring_t devicename = 0;
        //     inode_t   path       = 0;
        //     register_fulldirpath(str, devicename, path);
        //     s16 const device  = register_device(devicename);
        //     device_t* pdevice = get_device(device);
        //     dirpath_t dirpath(pdevice, path);
        //     dp = dirpath;
        // }

        bool root_t::has_device(const crunes_t& device_name)
        {
            istring_t devname = find_string(device_name);
            if (devname != 0)
            {
                s16 const dev = find_device(devname);
                return dev >= 0;
            }
            return false;
        }

        bool root_t::register_userdata1(const crunes_t& devpathstr, s32 userdata1)
        {
            istring_t devname = 0;
            inode_t   devpath = 0;
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
            istring_t devname = 0;
            inode_t   devpath = 0;
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
            istring_t aliasname;
            register_name(aliasstr, aliasname);

            istring_t devname = 0;
            inode_t   devpath = 0;
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
            crunes_t device_str;
            m_root->m_strings->to_string(device->m_deviceName, device_str);
            // TODO append this device's path to the string

            // the rest of the devices are aliases and should be appending their paths
            while (--i >= 0)
            {
                device = devices[i];
                m_root->m_strings->to_string(device->m_deviceName, device_str);
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
        //  - From pathreg_t* you can ask for the root directory of a device
        //    - pathreg_t* root = pathreg_t::root();
        //    - dirpath_t appdir = root->device_root("appdir");
        //  - So now with an existing dirpath_t dir, you could do the following:
        //    - dirpath_t bins = appdir.down("bin") // even if this folder doesn't exist, it will be 'added'
        //    - filepath_t coolexe = bins.file("cool.exe");
        //    - pathnode_t* datafilename; pathnode_t* dataextension;
        //    - root->filename("data.txt", datafilename, dataextension);
        //    - filepath_t datafilepath = bins.file(datafilename, dataextension);
        //    - filestream_t datastream = datafilepath.open(); // if this file doesn't exist, it will be created
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