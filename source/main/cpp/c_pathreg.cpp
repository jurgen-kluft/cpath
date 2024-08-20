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
    /*
        Every folder has a name, has a parent folder (except for the root folder), and has a bst that
        contains all the child folders and files. The bst is sorted by the name of the file/folder.
    */
    pathdevice_t* pathreg_t::sNilDevice;

    void pathreg_t::init(alloc_t* allocator)
    {
        m_allocator = allocator;

        m_pathdb = m_allocator->construct<pathdb_t>(m_allocator);
        m_pathdb->init(&m_strings, m_allocator, 65536 * 32768);

        m_max_devices = 64;
        m_num_devices = 1;
        m_arr_devices = (pathdevice_t**)m_allocator->allocate(m_max_devices * sizeof(pathdevice_t*));
        for (s32 i = 0; i < m_max_devices; ++i)
        {
            m_arr_devices[i] = m_allocator->construct<pathdevice_t>(m_allocator);
        }

        sNilDevice               = m_arr_devices[0];
        sNilDevice->m_pathreg    = this;
        sNilDevice->m_pathdb     = m_pathdb;
        sNilDevice->m_alias      = 0;
        sNilDevice->m_deviceName = 0;
        sNilDevice->m_devicePath = 0;
        sNilDevice->m_redirector = 0;
        sNilDevice->m_userdata1  = 0;
        sNilDevice->m_userdata2  = 0;
    }

    void pathreg_t::exit(alloc_t* allocator)
    {
        for (s32 i = 0; i < m_num_devices; ++i)
        {
            release_pathstr(m_arr_devices[i]->m_alias);
            release_pathstr(m_arr_devices[i]->m_deviceName);
            release_pathdevice(m_arr_devices[i]->m_devicePath);
            m_arr_devices[i]->m_pathreg    = nullptr;
            m_arr_devices[i]->m_pathdb     = nullptr;
            m_arr_devices[i]->m_alias      = 0;
            m_arr_devices[i]->m_deviceName = 0;
            m_arr_devices[i]->m_devicePath = 0;
            m_arr_devices[i]->m_redirector = 0;
            m_arr_devices[i]->m_userdata1  = 0;
            m_arr_devices[i]->m_userdata2  = 0;
        }
        m_num_devices = 0;
        m_pathdb->exit(m_allocator);
    }

    void pathreg_t::release_pathstr(pathstr_t p)
    {
        if (p == 0)
            return;
        // We do not reference count the strings yet, so we cannot know if we should release a string
        // This could be implemented in the future
        m_pathdb->detach(p);
    }

    void pathreg_t::release_pathstr(pathstr_t p)
    {
        if (p == 0)
            return;
        m_pathdb->detach(p);
    }

    void pathreg_t::release_pathdevice(s16 dev)
    {
        if (dev < 0)
            return;
        m_pathdb->detach(dev);
    }

    pathstr_t pathreg_t::find_name(crunes_t const& namestr) const
    {
        pathstr_t name = 0;

        return name;
    }

    void pathreg_t::register_name(crunes_t const& namestr, pathstr_t& outname)
    {
        // TODO register this string at m_strings
        outname = 0;
    }

    void pathreg_t::register_fulldirpath(crunes_t const& fulldirpath, pathstr_t& outdevicename, pathnode_t& outnode)
    {
        pathparser_t parser;
        parser.parse(fulldirpath);

        outdevicename = 0;
        if (parser.has_device())
        {
            outdevicename = m_pathdb->findOrInsert(parser.m_device);
        }

        outnode = 0;
        if (parser.has_path())
        {
            crunes_t   folder      = parser.iterate_folder();
            pathnode_t parent_node = 0;
            do
            {
                pathstr_t  folder_pathstr = m_pathdb->findOrInsert(folder);
                pathnode_t folder_node    = m_pathdb->findOrInsert(parent_node, folder_pathstr);
                parent_node               = folder_node;
            } while (parser.next_folder(folder));
            outnode = parent_node;
        }
    }

    void pathreg_t::register_dirpath(crunes_t const& dirpath, pathnode_t& outnode)
    {
        pathparser_t parser;
        parser.parse(dirpath);

        outnode = 0;
        if (parser.has_path())
        {
            crunes_t   folder      = parser.iterate_folder();
            pathnode_t parent_node = 0;
            do
            {
                pathstr_t  folder_pathstr = m_pathdb->findOrInsert(folder);
                pathnode_t folder_node    = m_pathdb->findOrInsert(parent_node, folder_pathstr);
                parent_node               = folder_node;
            } while (parser.next_folder(folder));
            outnode = parent_node;
        }
    }

    void pathreg_t::register_filename(crunes_t const& namestr, pathstr_t& out_filename, pathstr_t& out_extension)
    {
        crunes_t filename_str    = namestr;
        filename_str             = nrunes::findLastSelectUntil(filename_str, '.');
        crunes_t  extension_str  = nrunes::selectAfterExclude(namestr, filename_str);
        pathstr_t filename_name  = m_pathdb->findOrInsert(filename_str);
        pathstr_t extension_name = m_pathdb->findOrInsert(extension_str);
        out_filename             = filename_name;
        out_extension            = extension_name;
    }

    void pathreg_t::register_fullfilepath(crunes_t const& fullfilepath, pathstr_t& out_device, pathnode_t& out_path, pathstr_t& out_filename, pathstr_t& out_extension)
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

    s16 pathreg_t::find_device(pathstr_t devicename) const
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

    s16 pathreg_t::register_device(pathstr_t devicename)
    {
        s16 device = find_device(devicename);
        if (device == -1)
        {
            if (m_num_devices < 64)
            {
                m_arr_devices[m_num_devices]->m_pathreg    = this;
                m_arr_devices[m_num_devices]->m_pathdb     = this->m_pathdb;
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

    pathdevice_t* pathreg_t::get_device(s16 index)
    {
        if (index == -1)
            return nullptr;
        return m_arr_devices[index];
    }

    pathnode_t pathreg_t::get_parent_path(pathnode_t path)
    {
        // get folder_t*, then return the parent
        return 0;
    }

    void pathreg_t::resolve(filepath_t const& fp, pathdevice_t*& device, pathnode_t& dir, pathstr_t& filename, pathstr_t& extension)
    {
        device    = get_pathdevice(fp);
        dir       = get_path(fp);
        filename  = get_filename(fp);
        extension = get_extension(fp);
    }

    void pathreg_t::resolve(dirpath_t const& dp, pathdevice_t*& device, pathnode_t& dir)
    {
        device = get_pathdevice(dp);
        dir    = get_path(dp);
    }

    pathdevice_t* pathreg_t::get_pathdevice(dirpath_t const& dirpath) { return dirpath.m_device; }
    pathdevice_t* pathreg_t::get_pathdevice(filepath_t const& filepath) { return filepath.m_dirpath.m_device; }
    pathnode_t    pathreg_t::get_path(dirpath_t const& dirpath) { return dirpath.m_path; }
    pathnode_t    pathreg_t::get_path(filepath_t const& filepath) { return filepath.m_dirpath.m_path; }
    pathstr_t     pathreg_t::get_filename(filepath_t const& filepath) { return filepath.m_filename; }
    pathstr_t     pathreg_t::get_extension(filepath_t const& filepath) { return filepath.m_extension; }
    pathreg_t*    pathreg_t::get_filesystem(dirpath_t const& dirpath) { return dirpath.m_device->m_pathreg; }
    pathreg_t*    pathreg_t::get_filesystem(filepath_t const& filepath) { return filepath.m_dirpath.m_device->m_pathreg; }

    void pathreg_t::filepath(const char* str, filepath_t& fp)
    {
        crunes_t filepathstr(str);
        filepath(filepathstr, fp);
    }

    void pathreg_t::dirpath(const char* str, dirpath_t& dp)
    {
        crunes_t dirpathstr(str);
        dirpath(dirpathstr, dp);
    }

    void pathreg_t::filepath(const crunes_t& str, filepath_t& fp)
    {
        pathstr_t  devicename = 0;
        pathnode_t path       = 0;
        pathstr_t  filename   = 0;
        pathstr_t  extension  = 0;
        register_fullfilepath(str, devicename, path, filename, extension);

        s16 const     device  = register_device(devicename);
        pathdevice_t* pdevice = get_device(device);
        filepath_t    filepath(pdevice, path, filename, extension);
        fp = filepath;
    }

    void pathreg_t::dirpath(const crunes_t& str, dirpath_t& dp)
    {
        pathstr_t  devicename = 0;
        pathnode_t path       = 0;
        register_fulldirpath(str, devicename, path);
        s16 const     device  = register_device(devicename);
        pathdevice_t* pdevice = get_device(device);
        dirpath_t     dirpath(pdevice, path);
        dp = dirpath;
    }

    bool pathreg_t::has_device(const crunes_t& device_name)
    {
        pathstr_t devname = find_name(device_name);
        if (devname != 0)
        {
            s16 const dev = find_device(devname);
            return dev >= 0;
        }
        return false;
    }

    bool pathreg_t::register_userdata1(const crunes_t& devpathstr, s32 userdata1)
    {
        pathstr_t  devname = 0;
        pathnode_t devpath = 0;
        register_fulldirpath(devpathstr, devname, devpath);

        s16 const     device = register_device(devname);
        pathdevice_t* dev    = get_device(device);
        dev->m_devicePath    = devpath;
        if (dev->m_userdata1 == -1)
        {
            dev->m_userdata1 = userdata1;
        }
        return true;
    }

    bool pathreg_t::register_userdata2(const crunes_t& devpathstr, s32 userdata2)
    {
        pathstr_t  devname = 0;
        pathnode_t devpath = 0;
        register_fulldirpath(devpathstr, devname, devpath);

        s16 const     device = register_device(devname);
        pathdevice_t* dev    = get_device(device);
        dev->m_devicePath    = devpath;
        if (dev->m_userdata2 == -1)
        {
            dev->m_userdata2 = userdata2;
        }
        return true;
    }

    bool pathreg_t::register_alias(const crunes_t& aliasstr, const crunes_t& devpathstr)
    {
        pathstr_t aliasname;
        register_name(aliasstr, aliasname);

        pathstr_t  devname = 0;
        pathnode_t devpath = 0;
        register_fulldirpath(devpathstr, devname, devpath);

        s16 const     device = register_device(aliasname);
        pathdevice_t* alias  = get_device(device);
        alias->m_alias       = aliasname;
        alias->m_deviceName  = devname;
        alias->m_devicePath  = devpath;
        alias->m_redirector  = register_device(devname);

        return true;
    }

} // namespace ncore
