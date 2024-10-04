#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "cbase/c_integer.h"
#include "cbase/c_runes.h"

#include "cpath/c_path.h"
#include "cpath/c_filepath.h"
#include "cpath/c_dirpath.h"
#include "cpath/private/c_parser.h"

namespace ncore
{
    //==============================================================================
    // dirpath_t: "Device:\\Folder\Folder\"
    //==============================================================================

    dirpath_t::dirpath_t() : m_device(nullptr), m_path(0) {}

    dirpath_t::dirpath_t(dirpath_t const& other)
    {
        m_device = other.m_device->attach();
        m_path   = other.m_device->m_root->attach_pathnode(other.m_path);
    }
    dirpath_t::dirpath_t(npath::device_t* device)
    {
        m_device = device->attach();
        m_path   = 0;
    }
    dirpath_t::dirpath_t(npath::device_t* device, npath::node_t path)
    {
        m_device = m_device->m_root->attach_pathdevice(device);
        m_path   = m_device->m_root->attach_pathnode(path);
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
        m_device = nullptr;
        m_path   = 0;
    }

    bool dirpath_t::isEmpty() const { return m_device == nullptr && m_path == 0; }
    bool dirpath_t::isRoot() const { return m_device != nullptr && m_path != 0; }
    bool dirpath_t::isRooted() const { return m_device != nullptr; }

    dirpath_t dirpath_t::makeRelative(const dirpath_t& dirpath) const
    {
        // try and find an overlap of folder
        //   this    = a b c d [e f]
        //   dirpath = [e f] g h i j
        //   overlap = [e f]
        //   result  = g h i j

        // TODO
        return dirpath_t();
    }

    dirpath_t dirpath_t::makeAbsolute(const dirpath_t& dirpath) const
    {
        //   this    = a b c d [e f]
        //   dirpath = [e f] g h i j k
        //   overlap = [e f]
        //   result  = a b c d [e f] g h i j k

        // TODO
        return dirpath_t();
    }

    npath::string_t dirpath_t::devname() const { return m_device->m_deviceName; }

    npath::string_t dirpath_t::basename() const
    {
        npath::root_t*   root   = m_device->m_root;
        npath::folder_t* folder = root->m_folders.ptr_of(m_path);
        return folder->m_name;
    }

    npath::string_t dirpath_t::rootname() const
    {
        npath::root_t* root = m_device->m_root;
        npath::node_t  iter = m_path;

        npath::folder_t* folder = root->m_folders.ptr_of(iter);
        while (folder->m_parent != 0)
        {
            iter   = folder->m_parent;
            folder = root->m_folders.ptr_of(iter);
        }
        return folder->m_name;
    }

    dirpath_t dirpath_t::device() const { return dirpath_t(m_device); }

    dirpath_t dirpath_t::root() const
    {
        dirpath_t      dp(m_device);
        npath::root_t* root = m_device->m_root;
        npath::node_t  left = 0;
        dp.m_path           = m_device->m_root->attach_pathnode(left);
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

    s32 dirpath_t::depth() const
    {
        if (m_path == 0)
            return 0;
        s32              levels = 1;
        npath::root_t*   root   = m_device->m_root;
        npath::node_t    iter   = m_path;
        npath::folder_t* folder = root->m_folders.ptr_of(iter);
        while (folder->m_parent != 0)
        {
            iter   = folder->m_parent;
            folder = root->m_folders.ptr_of(iter);
            ++levels;
        }
        return levels;
    }

    dirpath_t dirpath_t::up() const
    {
        dirpath_t dp;
        dp.m_device = this->m_device;
        dp.m_base   = this->m_base;
        dp.m_path   = this->m_path;

        npath::root_t*   root   = m_device->m_root;
        npath::folder_t* folder = root->m_folders.ptr_of(m_path);
        dp.m_path               = folder->m_parent;
        return dp;
    }

    dirpath_t dirpath_t::down() const
    {
        // return the first child folder of this dirpath
        // TODO
        return dirpath_t();
    }

    void dirpath_t::down(crunes_t const& folder)
    {
        npath::root_t*  root       = m_device->m_root;
        npath::string_t folder_str = root->find_or_insert_string(folder);
        m_path                     = root->find_or_insert_path(m_path, folder_str);
    }

    dirpath_t dirpath_t::next() const
    {
        // return the next sibling
        // TODO
        return dirpath_t();
    }

    filepath_t dirpath_t::file(crunes_t const& filepath)
    {
        npath::root_t* root = m_device->m_root;

        npath::parser_t parser;
        parser.parse(filepath);

        if (parser.has_filename() && parser.has_extension())
        {
            npath::string_t filename  = root->find_or_insert_string(parser.m_filename);
            npath::string_t extension = root->find_or_insert_string(parser.m_extension);
            return filepath_t(m_device, m_path, filename, extension);
        }
        else if (parser.has_filename())
        {
            npath::string_t filename = root->find_or_insert_string(parser.m_filename);
            return filepath_t(m_device, m_path, filename, 0);
        }
        else if (parser.has_extension())
        {
            npath::string_t extension = root->find_or_insert_string(parser.m_extension);
            return filepath_t(m_device, m_path, 0, extension);
        }

        return filepath_t(m_device, m_path, 0, 0);
    }

    s32 dirpath_t::compare(const dirpath_t& other) const
    {
        s32 const de = other.m_device->compare(m_device);
        if (de != 0)
            return de;
        s32 const pe = other.m_device->m_root->compare_str(m_path, other.m_path);
        return pe;
    }

    // void dirpath_t::to_string(runes_t& str) const
    // {
    //     npath::root_t* root = m_device->m_root;
    //     m_device->to_string(str);
    //     root->to_string(m_path, str);
    // }

    // s32 dirpath_t::to_strlen() const
    // {
    //     npath::root_t* root = m_device->m_root;
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
        npath::root_t* root = m_device->m_root;

        root->release_pathdevice(m_device);
        root->release_pathstr(m_path);

        m_device = root->attach_pathdevice(other.m_device);
        m_path   = root->attach_pathnode(other.m_path);
        return *this;
    }

}; // namespace ncore
