#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "cbase/c_integer.h"
#include "cbase/c_runes.h"

#include "cpath/c_filepath.h"
#include "cpath/c_dirpath.h"
#include "cpath/private/c_root.h"

namespace ncore
{
    //==============================================================================
    // dirpath_t: "Device:\\Folder\Folder\"
    //==============================================================================

    dirpath_t::dirpath_t() : m_device(npath::root_t::sNilDevice), m_path(npath::root_t::sNilNode) {}

    dirpath_t::dirpath_t(dirpath_t const& other)
    {
        m_device = other.m_device->attach();
        m_path   = other.m_device->m_root->attach(other.m_path);
    }
    dirpath_t::dirpath_t(npath::device_t* device)
    {
        m_device = device->attach();
        m_path   = 0;
    }
    dirpath_t::dirpath_t(npath::device_t* device, npath::inode_t path)
    {
        m_device = m_device->m_root->attach(device);
        m_path   = m_device->m_root->attach(path);
    }

    dirpath_t::~dirpath_t()
    {
        npath::root_t* root = m_device->m_root;
        root->release_pathdevice(m_device);
        root->release_pathstr(m_path);
    }

    void dirpath_t::clear()
    {
        npath::root_t* root = m_device->m_root;
        root->release_pathdevice(m_device);
        root->release_pathstr(m_path);
        m_device = npath::root_t::sNilDevice;
        m_path   = npath::root_t::sNilNode;
    }

    bool dirpath_t::isEmpty() const { return m_device == npath::root_t::sNilDevice && m_path == npath::root_t::sNilNode; }
    bool dirpath_t::isRoot() const { return m_device != npath::root_t::sNilDevice && m_path != npath::root_t::sNilNode; }
    bool dirpath_t::isRooted() const { return m_device != npath::root_t::sNilDevice; }

    void dirpath_t::makeRelativeTo(const dirpath_t& dirpath)
    {
        // try and find an overlap of folder
        //   this    = a b c d [e f]
        //   dirpath = [e f] g h i j
        //   overlap = [e f]
        //   result  = g h i j

        // TODO
    }

    void dirpath_t::makeAbsoluteTo(const dirpath_t& dirpath)
    {
        //   this    = a b c d [e f]
        //   dirpath = [e f] g h i j k
        //   overlap = [e f]
        //   result  = a b c d [e f] g h i j k

        // TODO
    }

    void dirpath_t::getSubDir(const dirpath_t& parentpath, const dirpath_t& path, dirpath_t& out_subpath)
    {
        //   parent  = [a b c d e f]
        //   path    = [a b c d e f] g h i j
        //   overlap = [a b c d e f]
        //   subpath = g h i j

        // TODO
    }

    dirpath_t dirpath_t::device() const { return dirpath_t(m_device); }

    dirpath_t dirpath_t::root() const
    {
        dirpath_t      dp(m_device);
        npath::root_t* root = m_device->m_root;
        npath::inode_t left = 0;
        dp.m_path           = m_device->m_root->attach(left);
        return dp;
    }

    dirpath_t dirpath_t::parent() const
    {
        npath::root_t* root = m_device->m_root;
        dirpath_t      dp(m_device);
        dp.m_path = root->get_parent_path(m_path);
        dp.m_path = m_device->m_root->attach_pathstr(dp.m_path);
        return dp;
    }

    npath::istring_t dirpath_t::basename() const
    {
        npath::root_t*   root   = m_device->m_root;
        npath::folder_t* folder = root->m_folders.ptr_of(m_path);
        return folder->m_name;
    }

    npath::istring_t dirpath_t::rootname() const
    {
        npath::root_t* root = m_device->m_root;
        npath::inode_t iter = m_path;

        npath::folder_t* folder = root->m_folders.ptr_of(iter);
        while (folder->m_parent != 0)
        {
            iter   = folder->m_parent;
            folder = root->m_folders.ptr_of(iter);
        }
        return folder->m_name;
    }

    npath::istring_t dirpath_t::devname() const { return m_device->m_deviceName; }

    s32 dirpath_t::getLevels() const
    {
        if (m_path == 0)
            return 0;
        s32              levels = 1;
        npath::root_t*   root   = m_device->m_root;
        npath::inode_t   iter   = m_path;
        npath::folder_t* folder = root->m_folders.ptr_of(iter);
        while (folder->m_parent != 0)
        {
            iter   = folder->m_parent;
            folder = root->m_folders.ptr_of(iter);
            ++levels;
        }
        return levels;
    }

    void dirpath_t::down(crunes_t const& folder)
    {
        npath::root_t*   root       = m_device->m_root;
        npath::istring_t folder_str = root->findOrInsert(folder);
        m_path                      = root->findOrInsert(m_path, folder_str);
    }

    void dirpath_t::up()
    {
        npath::root_t* root = m_device->m_root;
        npath::folder_t* folder = root->m_folders.ptr_of(m_path);
        root->release_pathstr(m_path);
        m_path = folder->m_parent;
    }

    s32 dirpath_t::compare(const dirpath_t& other) const
    {
        s32 const de = other.m_device->compare(m_device);
        if (de != 0)
            return de;
        s32 const pe = other.m_device->m_root->compare_str(m_path, other.m_path);
        return pe;
    }

    void dirpath_t::to_string(runes_t& str) const
    {
        npath::root_t* root = m_device->m_root;
        m_device->to_string(str);
        root->to_string(m_path, str);
    }

    s32 dirpath_t::to_strlen() const
    {
        npath::root_t* root = m_device->m_root;
        s32            len  = m_device->to_strlen();
        len += root->to_strlen(m_path);
        return len;
    }

    dirpath_t& dirpath_t::operator=(dirpath_t const& other)
    {
        npath::root_t* root = m_device->m_root;

        root->release_pathdevice(m_device);
        root->release_pathstr(m_path);

        m_device = root->attach(other.m_device);
        m_path   = root->attach(other.m_path);
        return *this;
    }

    dirpath_t operator+(const dirpath_t& left, const dirpath_t& right)
    {
        dirpath_t dp(left);

        return dp;
    }

}; // namespace ncore
