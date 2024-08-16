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
    struct pathnode_t;


    // -------------------------------------------------------------------------------------------
    //
    // pathdb_t implementations
    //
    void pathdb_t::init(alloc_t* allocator, u32 cap)
    {
        // allocate text entries using an average of 32 bytes per string excluding the name_t header
        // @todo; these should be virtual buffers
        m_text_data_cap  = cap * (32 + sizeof(str_t) + sizeof(name_t));
        m_text_data      = (name_t*)allocator->allocate(m_text_data_cap, sizeof(void*));
        m_text_data_size = sizeof(name_t);

        m_node_array      = (folder_t*)allocator->allocate(sizeof(folder_t) * cap, sizeof(void*));
        m_node_free_head  = nullptr;
        m_node_free_index = 1;

        m_nil_str         = (str_t*)m_text_data;
        m_nil_str->m_hash = 0;
        m_nil_str->m_len  = 0;

        m_nil_name                = (name_t*)m_text_data;
        m_nil_name->m_str         = m_nil_str;
        m_nil_name->m_children[0] = 0; // left
        m_nil_name->m_children[1] = 0; // right

        m_nil_node                = m_node_array;
        m_nil_node->m_children[0] = 0; // left
        m_nil_node->m_children[1] = 0; // right
        m_nil_node->m_parent      = 0;
        m_nil_node->m_name        = m_nil_str;

        m_strings_root = nullptr;
        m_nodes_root   = nullptr;
    }

    void pathdb_t::release(alloc_t* allocator)
    {
        allocator->deallocate(m_text_data);
        m_text_data      = nullptr;
        m_text_data_size = 0;
        m_text_data_cap  = 0;

        m_node_array      = nullptr;
        m_node_free_head  = nullptr;
        m_node_free_index = 0;

        m_nil_node = nullptr;
        m_nil_str  = nullptr;

        m_strings_root = nullptr;
        m_nodes_root   = nullptr;
    }

    static u32 hash(utf8::pcrune str, utf8::pcrune end)
    {
        u32 hash = 0;
        while (str < end)
        {
            hash = hash + *str++ * 31;
        }
        return hash;
    }

    pathname_t* pathdb_t::findOrInsert(crunes_t const& str)
    {
        // write this string to the text buffer as utf8
        name_t* str_entry = (name_t*)((u8*)m_text_data + m_text_data_size);
        str_entry->reset();

        // need a function to write crunes_t to a utf-8 buffer
        utf8::prune str8 = nullptr;
        utf8::prune end8 = nullptr;

        u32 const str_hash = hash(str8, end8);
        u32 const str_len  = end8 - str8;

        // See if we can find the string in the tree
        name_t* it = m_strings_root;
        while (it != m_nil_name)
        {
            s32 c = 0;
            if (str_hash == it->m_str->m_hash)
            {
                // binary comparison
                utf8::pcrune ostr8 = it->m_str->str();
                utf8::pcrune oend8 = it->m_str->str() + it->m_str->m_len;
                c                  = compare_buffers(str8, end8, ostr8, oend8);
                if (c == 0)
                    return it;
                c = (c + 1) >> 1;
            }
            else if (str_hash < it->m_str->m_hash)
            {
                c = 0;
            }
            else
            {
                c = 1;
            }
            it = (name_t*)((u8*)m_text_data + it->m_children[c]);
        }

        // not found, so create and add it
        it = str_entry;

        // add it to the tree

        return it;
    }

    // -------------------------------------------------------------------------------------------
    //
    // pathdevice_t implementations
    //
    void pathdevice_t::init(pathreg_t* owner)
    {
        m_root       = owner;
        m_alias      = owner->sNilStr;
        m_deviceName = owner->sNilStr;
        m_devicePath = owner->sNilNode;
        m_redirector = nullptr;
        m_fileDevice = nullptr;
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
        m_root->m_pathdb->attach(m_alias);
        m_root->m_pathdb->attach(m_deviceName);
        m_root->m_pathdb->attach(m_devicePath);
        if (m_redirector != nullptr)
            m_redirector->attach();
        return this;
    }

    bool pathdevice_t::detach()
    {
        m_root->release_name(m_alias);
        m_root->release_name(m_deviceName);
        m_root->release_path(m_devicePath);
        if (m_redirector != nullptr)
            m_redirector->detach();

        m_alias      = m_root->sNilStr;
        m_deviceName = m_root->sNilStr;
        m_devicePath = m_root->sNilNode;
        m_redirector = nullptr;
        m_fileDevice = nullptr;

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
        s32                 i      = 0;
        pathdevice_t const* device = this;
        pathdevice_t const* devices[32];
        do
        {
            devices[i++] = device;
            device       = device->m_redirector;
        } while (device != nullptr && i < 32);

        device = devices[--i];

        // should be the root device (has filedevice), so first emit the device name.
        // this device should not have any device path.
        m_root->m_pathdb->to_string(device->m_deviceName, str);

        // the rest of the devices are aliases and should be appending their paths
        while (--i >= 0)
        {
            device = devices[i];
            m_root->m_pathdb->to_string(device->m_deviceName, str);
        }
    }

    s32 pathdevice_t::to_strlen() const
    {
        s32                 i      = 0;
        pathdevice_t const* device = this;
        pathdevice_t const* devices[32];
        do
        {
            devices[i++] = device;
            device       = device->m_redirector;
        } while (device != nullptr && i < 32);

        device = devices[--i];

        // should be the root device (has filedevice), so first emit the device name.
        // this device should not have any device path.
        s32 len = device->m_deviceName->m_len;

        // the rest of the devices are aliases and should be appending their paths
        while (--i >= 0)
        {
            device = devices[i];
            len += device->m_devicePath->m_name->m_len;
        }
        return len;
    }
}; // namespace ncore
