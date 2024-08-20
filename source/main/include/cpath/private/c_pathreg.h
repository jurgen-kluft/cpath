#ifndef __C_PATH_PATH_REG_H__
#define __C_PATH_PATH_REG_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_allocator.h"
#include "cbase/c_runes.h"

#include "cpath/private/c_pathstr_db.h"

namespace ncore
{
    class filepath_t;
    class dirpath_t;
    class pathdb_t;
    struct pathdevice_t;

    typedef u32 pathnode_t;

    class pathreg_t
    {
    public:
        void init(alloc_t* allocator);
        void exit(alloc_t* allocator);

        // -----------------------------------------------------------
        static void          resolve(filepath_t const&, pathdevice_t*& device, pathnode_t& dir, pathstr_t& filename, pathstr_t& extension);
        static void          resolve(dirpath_t const&, pathdevice_t*& device, pathnode_t& dir);
        static pathdevice_t* get_pathdevice(dirpath_t const& dirpath);
        static pathdevice_t* get_pathdevice(filepath_t const& filepath);
        static pathnode_t    get_path(dirpath_t const& dirpath);
        static pathnode_t    get_path(filepath_t const& filepath);
        static pathstr_t     get_filename(filepath_t const& filepath);
        static pathstr_t     get_extension(filepath_t const& filepath);
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
        pathstr_db_t   m_strings;
        pathdb_t*      m_pathdb;

        static pathdevice_t* sNilDevice;
        static pathstr_t*    sNilStr;
        static pathnode_t*   sNilNode;

        void filepath(const char* str, filepath_t&);
        void dirpath(const char* str, dirpath_t&);
        void filepath(const crunes_t& str, filepath_t&);
        void dirpath(const crunes_t& str, dirpath_t&);

        void register_fulldirpath(crunes_t const& fulldirpath, pathstr_t& out_devicename, pathnode_t& out_path);
        void register_fullfilepath(crunes_t const& fullfilepath, pathstr_t& out_devicename, pathnode_t& out_path, pathstr_t& out_filename, pathstr_t& out_extension);
        void register_dirpath(crunes_t const& dirpath, pathnode_t& out_path);
        void register_filename(crunes_t const& namestr, pathstr_t& filename, pathstr_t& extension);
        void register_name(crunes_t const& namestr, pathstr_t& name);
        bool register_userdata1(const crunes_t& devpathstr, s32 userdata1);
        bool register_userdata2(const crunes_t& devpathstr, s32 userdata2);
        bool register_alias(const crunes_t& aliasstr, const crunes_t& devpathstr);

        bool          has_device(const crunes_t& device_name);
        s16           register_device(crunes_t const& device);
        s16           register_device(pathstr_t device);
        pathdevice_t* get_device(s16 index);
        s16           find_device(pathstr_t devicename) const;

        void release_pathstr(pathstr_t name);
        void release_pathnode(pathnode_t path);
        void release_pathdevice(s16 dev);

        pathnode_t get_parent_path(pathnode_t path);
        pathstr_t  find_name(crunes_t const& namestr) const;

        DCORE_CLASS_PLACEMENT_NEW_DELETE
    };

} // namespace ncore

#endif // __C_PATH_PATH_REG_H__
