#ifndef __C_PATH_PATH_REG_H__
#define __C_PATH_PATH_REG_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_allocator.h"
#include "cbase/c_runes.h"

namespace ncore
{
    class filepath_t;
    class dirpath_t;
    struct pathdevice_t;
    struct pathnode_t;
    struct pathname_t;
    struct pathstr_t;
    class pathdb_t;

    class pathreg_t
    {
    public:
        void init(alloc_t* allocator);
        void exit(alloc_t* allocator);

        // -----------------------------------------------------------
        static void          resolve(filepath_t const&, pathdevice_t*& device, pathnode_t*& dir, pathstr_t*& filename, pathstr_t*& extension);
        static void          resolve(dirpath_t const&, pathdevice_t*& device, pathnode_t*& dir);
        static pathdevice_t* get_pathdevice(dirpath_t const& dirpath);
        static pathdevice_t* get_pathdevice(filepath_t const& filepath);
        static pathnode_t*   get_path(dirpath_t const& dirpath);
        static pathnode_t*   get_path(filepath_t const& filepath);
        static pathstr_t*    get_filename(filepath_t const& filepath);
        static pathstr_t*    get_extension(filepath_t const& filepath);
        static pathreg_t*    get_filesystem(dirpath_t const& dirpath);
        static pathreg_t*    get_filesystem(filepath_t const& filepath);

        // -----------------------------------------------------------
        //
        u32            m_max_path_objects;
        char           m_default_slash;
        alloc_t*       m_allocator;
        s32            m_num_devices;
        s32            m_max_devices;
        pathdevice_t** m_arr_devices;
        pathdb_t*      m_pathdb; // all data for pathnode_t, pathnode_t, pathnode_t, etc.

        static pathdevice_t* sNilDevice;
        static pathname_t*   sNilName;
        static pathstr_t*    sNilStr;
        static pathnode_t*   sNilNode;

        void filepath(const char* str, filepath_t&);
        void dirpath(const char* str, dirpath_t&);
        void filepath(const crunes_t& str, filepath_t&);
        void dirpath(const crunes_t& str, dirpath_t&);

        void register_fulldirpath(crunes_t const& fulldirpath, pathstr_t*& out_devicename, pathnode_t*& out_path);
        void register_fullfilepath(crunes_t const& fullfilepath, pathstr_t*& out_devicename, pathnode_t*& out_path, pathstr_t*& out_filename, pathstr_t*& out_extension);
        void register_dirpath(crunes_t const& dirpath, pathnode_t*& out_path);
        void register_filename(crunes_t const& namestr, pathstr_t*& filename, pathstr_t*& extension);
        void register_name(crunes_t const& namestr, pathstr_t*& name);

        pathdevice_t* register_device(crunes_t const& device);
        pathdevice_t* register_device(pathstr_t* device);

        void release_name(pathstr_t* name);
        void release_filename(pathstr_t* name);
        void release_extension(pathstr_t* name);
        void release_path(pathnode_t* path);
        void release_device(pathdevice_t* dev);

        pathnode_t* get_parent_path(pathnode_t* path);

        bool has_device(const crunes_t& device_name);

        pathstr_t*    find_name(crunes_t const& namestr) const;
        pathdevice_t* find_device(pathstr_t* devicename) const;

        DCORE_CLASS_PLACEMENT_NEW_DELETE
    };

} // namespace ncore

#endif
