#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/c_dirpath.h"
#include "cpath/c_filepath.h"
#include "cpath/private/c_pathdb.h"
#include "cpath/private/c_pathreg.h"

namespace ncore
{
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

    class pathreg_t;
    struct pathdevice_t;

    // -------------------------------------------------------------------------------------------
    //
    // pathdevice_t implementations
    //
    void pathdevice_t::init(pathreg_t* owner)
    {
        m_pathreg    = owner;
        m_pathdb     = owner->m_pathdb;
        m_alias      = 0;
        m_deviceName = 0;
        m_devicePath = 0;
        m_redirector = 0;
        m_userdata1  = 0;
        m_userdata2  = 0;
    }

    pathdevice_t* pathdevice_t::construct(alloc_t* allocator, pathreg_t* owner)
    {
        void*         device_mem = allocator->allocate(sizeof(pathdevice_t), sizeof(void*));
        pathdevice_t* device     = static_cast<pathdevice_t*>(device_mem);
        device->init(owner);
        return device;
    }

    void pathdevice_t::destruct(alloc_t* allocator, pathdevice_t*& device)
    {
        allocator->deallocate(device);
        device = nullptr;
    }

    pathdevice_t* pathdevice_t::attach()
    {
        m_pathdb->attach(m_alias);
        m_pathdb->attach(m_deviceName);
        m_pathdb->attach(m_devicePath);
        if (m_redirector != -1)
        {
            pathdevice_t* redirector = m_pathreg->m_arr_devices[m_redirector];
            redirector->attach();
        }
        return this;
    }

    bool pathdevice_t::detach()
    {
        m_alias      = m_pathdb->detach(m_alias);
        m_deviceName = m_pathdb->detach(m_deviceName);
        m_devicePath = m_pathdb->detach(m_devicePath);

        m_pathreg->release_pathdevice(m_redirector);
        m_userdata1 = 0;
        m_userdata2 = 0;

        return false;
    }

    s32 pathdevice_t::compare(pathdevice_t* device) const
    {
        if (m_deviceName == device->m_deviceName)
            return 0;
        else if (m_deviceName < device->m_deviceName)
            return -1;
        return 1;
    }

    void pathdevice_t::to_string(runes_t& str) const
    {
        s32                 i            = 0;
        s16                 device_index = m_device_index;
        pathdevice_t const* devices[32];
        do
        {
            pathdevice_t* device   = m_pathreg->m_arr_devices[device_index];
            devices[i++]           = device;
            s16 const device_index = device->m_redirector;
        } while (device_index > 0 && i < 32);

        pathdevice_t const* device = devices[--i];

        // should be the root device (has filedevice), so first emit the device name.
        // this device should not have any device path.
        crunes_t device_str;
        m_pathdb->m_strings->to_string(device->m_deviceName, device_str);
        // TODO append this device's path to the string

        // the rest of the devices are aliases and should be appending their paths
        while (--i >= 0)
        {
            device = devices[i];
            m_pathdb->m_strings->to_string(device->m_deviceName, device_str);
            // TODO append this device's path to the string
        }
    }

    s32 pathdevice_t::to_strlen() const
    {
        s32                 i            = 0;
        s16                 device_index = m_device_index;
        pathdevice_t const* devices[32];
        do
        {
            pathdevice_t* device   = m_pathreg->m_arr_devices[device_index];
            devices[i++]           = device;
            s16 const device_index = device->m_redirector;
        } while (device_index > 0 && i < 32);

        pathdevice_t const* device = devices[--i];

        // should be the root device (has filedevice), so first emit the device name.
        // this device should not have any device path.

        s32 len = m_pathdb->m_strings->get_len(device->m_deviceName);
        len += 2; // for the ":\" or ":\"

        // the rest of the devices are aliases and should be appending their paths
        while (--i >= 0)
        {
            device = devices[i];
            len += m_pathdb->m_strings->get_len(device->m_devicePath);
            len += 1; // for the "\"
        }
        return len;
    }
}; // namespace ncore
