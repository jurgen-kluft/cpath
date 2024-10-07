#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "cbase/c_integer.h"
#include "cbase/c_runes.h"

#include "cpath/c_path.h"
#include "cpath/c_filepath.h"
#include "cpath/c_dirpath.h"
#include "cpath/c_device.h"
#include "cpath/private/c_folders.h"

namespace ncore
{
    //==============================================================================
    // dirpath_t: "Device:\\Folder\Folder\"
    //==============================================================================

    dirpath_t::dirpath_t(dirpath_t const& other)
    {
        m_device = other.m_device;
        m_base   = other.m_base;
        m_path   = other.m_path;
    }

    dirpath_t::dirpath_t(npath::device_t* device)
    {
        m_device = device;
        m_base   = npath::c_empty_node;
        m_path   = npath::c_empty_node;
    }

    dirpath_t::dirpath_t(npath::device_t* device, npath::node_t base)
    {
        m_device = device;
        m_base   = base;
        m_path   = base;
    }

    dirpath_t::dirpath_t(npath::device_t* device, npath::node_t base, npath::node_t path)
    {
        m_device = device;
        m_base   = base;
        m_path   = path;
    }

    dirpath_t::~dirpath_t() {}

    void dirpath_t::clear()
    {
        npath::paths_t* root = m_device->m_owner;
        m_device             = root->m_devices->get_default_device();
        m_base               = npath::c_empty_node;
        m_path               = npath::c_empty_node;
    }

    bool dirpath_t::isEmpty() const { return m_base == npath::c_empty_node && m_path == npath::c_empty_node; }
    bool dirpath_t::isRoot() const { return m_device != nullptr && m_device->m_path == m_path && m_base == m_path; }
    bool dirpath_t::isRooted() const { return m_device != nullptr; }

    dirpath_t dirpath_t::makeRelative(const dirpath_t& dirpath) const
    {
        // try and find an overlap of folder
        //   this    = [a b c d e f]     base = f, path = f
        //   dirpath = [e f g h i j]     base = j, path = j
        //   overlap = [e f]
        //   result  = [e f g h i j]     base = j, path = j

        // TODO

        return dirpath_t(m_device);
    }

    dirpath_t dirpath_t::makeAbsolute(const dirpath_t& dirpath) const
    {
        //   this    = [D] [a b c d] [e f]      base = d, path = f
        //   dirpath = [E] [g h a b c]          base = c, path = c
        //   result  = [E] [g h a b c d] [e f]  base = d, path = f

        // TODO

        return dirpath_t(m_device);
    }

    npath::string_t dirpath_t::devname() const { return m_device->m_owner->m_folders->m_array.ptr_of(m_device->m_path)->m_name; }

    npath::string_t dirpath_t::basename() const
    {
        npath::paths_t*  root   = m_device->m_owner;
        npath::folder_t* folder = root->m_folders->m_array.ptr_of(m_path);
        return folder->m_name;
    }

    npath::string_t dirpath_t::rootname() const
    {
        npath::paths_t* root = m_device->m_owner;
        npath::node_t   iter = m_path;

        npath::folder_t* folder = root->m_folders->m_array.ptr_of(iter);
        while (folder->m_parent != 0)
        {
            iter   = folder->m_parent;
            folder = root->m_folders->m_array.ptr_of(iter);
        }
        return folder->m_name;
    }

    dirpath_t dirpath_t::device() const { return dirpath_t(m_device); }

    dirpath_t dirpath_t::root() const
    {
        dirpath_t       dp(m_device);
        npath::paths_t* root = m_device->m_owner;
        dp.m_base            = m_base;
        return dp;
    }

    dirpath_t dirpath_t::parent() const
    {
        npath::paths_t* root = m_device->m_owner;
        dirpath_t       dp(m_device);
        dp.m_base = m_path;
        dp.m_path = m_path;
        return dp;
    }

    s32 dirpath_t::depth() const
    {
        if (m_path == 0)
            return 0;
        s32              levels = 1;
        npath::paths_t*  root   = m_device->m_owner;
        npath::node_t    iter   = m_path;
        npath::folder_t* folder = root->m_folders->m_array.ptr_of(iter);
        while (folder->m_parent != 0)
        {
            iter   = folder->m_parent;
            folder = root->m_folders->m_array.ptr_of(iter);
            ++levels;
        }
        return levels;
    }

    dirpath_t dirpath_t::up() const
    {
        npath::paths_t*  root   = m_device->m_owner;
        npath::folder_t* folder = root->m_folders->m_array.ptr_of(m_path);
        npath::node_t    path   = folder->m_parent;
        return dirpath_t(m_device, m_base, path);
    }

    dirpath_t dirpath_t::down() const
    {
        npath::paths_t* const root = m_device->m_owner;
        npath::node_t const   base = m_device->get_first_child_dir(m_path);
        npath::node_t const   path = m_base;
        return dirpath_t(m_device, base, path);
    }

    dirpath_t dirpath_t::down(crunes_t const& folder) const
    {
        npath::paths_t*     root       = m_device->m_owner;
        npath::string_t     folder_str = root->find_or_insert_string(folder);
        npath::node_t const path       = m_device->add_dir(m_path, folder_str);
        return dirpath_t(m_device, m_base, path);
    }

    filepath_t dirpath_t::filename(crunes_t const& filename) const
    {
        // TODO

        return filepath_t(m_device);
    }

    static s8 s_compare_devices(npath::device_t* deviceA, npath::device_t* deviceB)
    {
        if (deviceA->m_index < deviceB->m_index)
            return -1;
        if (deviceA->m_index > deviceB->m_index)
            return 1;
        return 0;
    }

    s32 dirpath_t::compare(const dirpath_t& other) const
    {
        s8 const de = s_compare_devices(m_device, other.m_device);
        if (de != 0)
            return de;
        s32 const pe = other.m_device->m_owner->compare_str(m_path, other.m_path);
        return pe;
    }

    // void dirpath_t::to_string(runes_t& str) const
    // {
    //     npath::paths_t* root = m_device->m_owner;
    //     m_device->to_string(str);
    //     root->to_string(m_path, str);
    // }

    // s32 dirpath_t::to_strlen() const
    // {
    //     npath::paths_t* root = m_device->m_owner;
    //     s32            len  = m_device->to_strlen();
    //     len += root->to_strlen(m_path);
    //     return len;
    // }

    void dirpath_t::relative_path_to_string(runes_t& str) const {}
    s32  dirpath_t::relative_path_to_strlen() const { return 0; }

    // (device = "E", base = "documents\old\inventory\", path = "books\sci-fi\") -> "E:\documents\"
    void dirpath_t::root_path_to_string(runes_t& str) const {}
    s32  dirpath_t::root_path_to_strlen() const { return 0; }

    // (device = "E", base = "documents\old\inventory\", path = "books\sci-fi\") -> "E:\documents\old\inventory\"
    void dirpath_t::base_path_to_string(runes_t& str) const {}
    s32  dirpath_t::base_path_to_strlen() const { return 0; }

    // (device = "E", base = "documents\old\inventory\", path = "books\sci-fi\") -> "E:\documents\old\inventory\books\sci-fi\"
    void dirpath_t::full_path_to_string(runes_t& str) const {}
    s32  dirpath_t::full_path_to_strlen() const { return 0; }

    dirpath_t& dirpath_t::operator=(dirpath_t const& other)
    {
        m_device = other.m_device;
        m_base   = other.m_base;
        m_path   = other.m_path;
        return *this;
    }

}; // namespace ncore
