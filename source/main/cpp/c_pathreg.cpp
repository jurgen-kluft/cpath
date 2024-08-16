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
#include "cpath/private/c_pathparser.h"

namespace ncore
{
    // -------------------------------------------------------------------------------------------
    //
    // pathreg_t functions
    //
    pathdevice_t* pathreg_t::sNilDevice;
    pathname_t*   pathreg_t::sNilName;
    pathstr_t*    pathreg_t::sNilStr;
    pathnode_t*   pathreg_t::sNilNode;

    void pathreg_t::init(alloc_t* allocator)
    {
        m_allocator = allocator;

        m_pathdb = m_allocator->construct<pathdb_t>(m_allocator);
        m_pathdb->init(m_allocator, 65536 * 32768);

        sNilName = m_pathdb->get_nil_name();
        sNilStr  = m_pathdb->get_nil_str();
        sNilNode = m_pathdb->get_nil_node();

        m_max_devices = 64;
        m_num_devices = 1;
        m_arr_devices = (pathdevice_t**)m_allocator->allocate(m_max_devices * sizeof(pathdevice_t*));
        for (s32 i = 0; i < m_max_devices; ++i)
        {
            m_arr_devices[i] = m_allocator->construct<pathdevice_t>(m_allocator);
        }

        sNilDevice               = m_arr_devices[0];
        sNilDevice->m_root       = this;
        sNilDevice->m_alias      = sNilStr;
        sNilDevice->m_deviceName = sNilStr;
        sNilDevice->m_devicePath = sNilNode;
        sNilDevice->m_redirector = nullptr;
        sNilDevice->m_fileDevice = nullptr;
    }

    void pathreg_t::exit(alloc_t* allocator)
    {
        for (s32 i = 0; i < m_num_devices; ++i)
        {
            release_name(m_arr_devices[i]->m_alias);
            release_name(m_arr_devices[i]->m_deviceName);
            release_path(m_arr_devices[i]->m_devicePath);
            m_arr_devices[i]->m_root       = nullptr;
            m_arr_devices[i]->m_alias      = nullptr;
            m_arr_devices[i]->m_deviceName = nullptr;
            m_arr_devices[i]->m_devicePath = nullptr;
            m_arr_devices[i]->m_fileDevice = nullptr;
            m_arr_devices[i]->m_redirector = nullptr;
        }
        m_num_devices = 0;

        m_pathdb->release(m_allocator);
    }

    void pathreg_t::release_name(pathstr_t* p)
    {
        if (p == sNilStr)
            return;
        // m_pathdb->detach(p);
    }

    void pathreg_t::release_name(pathstr_t* p)
    {
        if (p == sNilStr)
            return;
        m_pathdb->detach(p);
    }

    void pathreg_t::release_filename(pathstr_t* p)
    {
        if (p == sNilStr)
            return;
        m_pathdb->detach(p);
    }

    void pathreg_t::release_extension(pathstr_t* p)
    {
        if (p == sNilStr)
            return;
        m_pathdb->detach(p);
    }

    void pathreg_t::release_path(pathnode_t* p)
    {
        if (p == sNilNode)
            return;
        m_pathdb->detach(p);
    }

    void pathreg_t::release_device(pathdevice_t* dev)
    {
        if (dev == sNilDevice)
            return;
        m_pathdb->detach(dev);
    }

    pathstr_t* pathreg_t::find_name(crunes_t const& namestr) const
    {
        pathstr_t* name = nullptr;

        return name;
    }

    void pathreg_t::register_name(crunes_t const& namestr, pathstr_t*& outname) { outname = nullptr; }

    void pathreg_t::register_fulldirpath(crunes_t const& fulldirpath, pathstr_t*& outdevicename, pathnode_t*& outnode)
    {
        pathparser_t parser;
        parser.parse(fulldirpath);

        outdevicename = sNilStr;
        if (parser.has_device())
        {
            pathname_t* name = m_pathdb->findOrInsert(parser.m_device);
            outdevicename    = name->m_str;
        }

        outnode = sNilNode;
        if (parser.has_path())
        {
            crunes_t    folder      = parser.iterate_folder();
            pathnode_t* parent_node = nullptr;
            do
            {
                pathname_t* folder_pathstr = m_pathdb->findOrInsert(folder);
                pathnode_t* folder_node    = m_pathdb->findOrInsert(parent_node, folder_pathstr);
                parent_node                = folder_node;
            } while (parser.next_folder(folder));
            outnode = parent_node;
        }
    }

    void pathreg_t::register_dirpath(crunes_t const& dirpath, pathnode_t*& outnode)
    {
        pathparser_t parser;
        parser.parse(dirpath);

        outnode = sNilNode;
        if (parser.has_path())
        {
            crunes_t    folder      = parser.iterate_folder();
            pathnode_t* parent_node = nullptr;
            do
            {
                pathname_t* folder_pathstr = m_pathdb->findOrInsert(folder);
                pathnode_t* folder_node    = m_pathdb->findOrInsert(parent_node, folder_pathstr);
                parent_node                = folder_node;
            } while (parser.next_folder(folder));
            outnode = parent_node;
        }
    }

    void pathreg_t::register_filename(crunes_t const& namestr, pathstr_t*& out_filename, pathstr_t*& out_extension)
    {
        crunes_t filename_str      = namestr;
        filename_str               = nrunes::findLastSelectUntil(filename_str, '.');
        crunes_t    extension_str  = nrunes::selectAfterExclude(namestr, filename_str);
        pathname_t* filename_name  = m_pathdb->findOrInsert(filename_str);
        pathname_t* extension_name = m_pathdb->findOrInsert(extension_str);
        out_filename               = filename_name->m_str;
        out_extension              = extension_name->m_str;
    }

    void pathreg_t::register_fullfilepath(crunes_t const& fullfilepath, pathstr_t*& out_device, pathnode_t*& out_path, pathstr_t*& out_filename, pathstr_t*& out_extension)
    {
        pathparser_t parser;
        parser.parse(fullfilepath);

        out_device = sNilStr;
        out_path   = sNilNode;
        if (parser.has_device())
        {
            register_fulldirpath(parser.deviceAndPath(), out_device, out_path);
        }
        else if (parser.has_path())
        {
            register_dirpath(parser.path(), out_path);
        }

        out_filename = sNilStr;
        if (parser.has_filename())
        {
            register_name(parser.m_filename, out_filename);
        }

        out_extension = sNilStr;
        if (parser.has_extension())
        {
            register_name(parser.m_extension, out_extension);
        }
    }

    pathdevice_t* pathreg_t::find_device(pathstr_t* devicename) const
    {
        for (s32 i = 0; i < m_num_devices; ++i)
        {
            if (m_arr_devices[i]->m_deviceName == devicename)
            {
                return (pathdevice_t*)&m_arr_devices[i];
            }
        }
        return nullptr;
    }

    pathdevice_t* pathreg_t::register_device(pathstr_t* devicename)
    {
        pathdevice_t* device = find_device(devicename);
        if (device == nullptr)
        {
            if (m_num_devices < 64)
            {
                m_arr_devices[m_num_devices]->m_root       = this;
                m_arr_devices[m_num_devices]->m_alias      = sNilStr;
                m_arr_devices[m_num_devices]->m_deviceName = devicename;
                m_arr_devices[m_num_devices]->m_devicePath = sNilNode;
                m_arr_devices[m_num_devices]->m_redirector = nullptr;
                m_arr_devices[m_num_devices]->m_fileDevice = nullptr;
                device                                     = m_arr_devices[m_num_devices];
                m_num_devices++;
            }
            else
            {
                device = sNilDevice;
            }
        }
        return device;
    }

    pathnode_t* pathreg_t::get_parent_path(pathnode_t* path) { return path->m_parent; }

} // namespace ncore
