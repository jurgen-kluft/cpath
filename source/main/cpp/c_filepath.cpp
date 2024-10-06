#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cpath/c_path.h"
#include "cpath/c_dirpath.h"
#include "cpath/c_filepath.h"
#include "cpath/private/c_parser.h"

namespace ncore
{
    filepath_t::filepath_t() : m_dirpath(), m_filename(0), m_extension(0) {}
    filepath_t::filepath_t(const filepath_t& other) : m_dirpath(other.m_dirpath)
    {
        m_filename  = m_dirpath.m_device->m_owner->attach_pathstr(other.m_filename);
        m_extension = m_dirpath.m_device->m_owner->attach_pathstr(other.m_extension);
    }

    filepath_t::filepath_t(npath::string_t filename, npath::string_t extension) : m_dirpath()
    {
        m_filename  = m_dirpath.m_device->m_owner->attach_pathstr(filename);
        m_extension = m_dirpath.m_device->m_owner->attach_pathstr(extension);
    }

    filepath_t::filepath_t(npath::device_t* device, npath::node_t path, npath::string_t filename, npath::string_t extension) : m_dirpath(device, path)
    {
        m_filename  = m_dirpath.m_device->m_owner->attach_pathstr(filename);
        m_extension = m_dirpath.m_device->m_owner->attach_pathstr(extension);
    }

    filepath_t::filepath_t(dirpath_t const& dirpath, npath::string_t filename, npath::string_t extension) : m_dirpath(dirpath)
    {
        m_filename  = m_dirpath.m_device->m_owner->attach_pathstr(filename);
        m_extension = m_dirpath.m_device->m_owner->attach_pathstr(extension);
    }

    filepath_t::~filepath_t()
    {
        if (m_dirpath.m_device != nullptr)
        {
            npath::instance_t* root = m_dirpath.m_device->m_owner;
            root->release_pathstr(m_filename);
            root->release_pathstr(m_extension);
        }
    }

    void filepath_t::clear()
    {
        m_dirpath.clear();
        npath::instance_t* root = m_dirpath.m_device->m_owner;
        root->release_pathstr(m_filename);
        root->release_pathstr(m_extension);
        m_filename  = 0;
        m_extension = 0;
    }

    bool filepath_t::isRooted() const { return m_dirpath.isRooted(); }
    bool filepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == 0 && m_extension == 0; }

    void filepath_t::makeRelativeTo(const dirpath_t& dirpath) { m_dirpath = m_dirpath.makeRelative(dirpath); }
    void filepath_t::makeAbsoluteTo(const dirpath_t& dirpath) { m_dirpath = m_dirpath.makeAbsolute(dirpath); }
    void filepath_t::setDirpath(dirpath_t const& dirpath) { m_dirpath = dirpath; }

    void filepath_t::setDevice(crunes_t const& devicename)
    {
        npath::instance_t* root = m_dirpath.m_device->m_owner;

        npath::parser_t parser;
        parser.parse(devicename);
        if (parser.has_device())
        {
            npath::string_t device_string = root->find_or_insert_string(parser.m_device);
            s16 const       idev          = root->register_device(device_string);
            m_dirpath.m_device            = root->get_device(idev);
        }
    }

    void filepath_t::setFilename(npath::string_t filename)
    {
        if (filename != m_filename)
        {
            npath::instance_t* root = m_dirpath.m_device->m_owner;
            root->release_pathstr(m_filename);
            m_filename = m_dirpath.m_device->m_owner->attach_pathstr(filename);
        }
    }

    void filepath_t::setFilename(crunes_t const& filenamestr)
    {
        npath::instance_t*  root          = m_dirpath.m_device->m_owner;
        npath::string_t out_filename  = 0;
        npath::string_t out_extension = 0;
        root->register_filename(filenamestr, out_filename, out_extension);
        root->release_pathstr(m_filename);
        root->release_pathstr(m_extension);
        m_filename  = m_dirpath.m_device->m_owner->attach_pathstr(out_filename);
        m_extension = m_dirpath.m_device->m_owner->attach_pathstr(out_extension);
    }

    void filepath_t::setExtension(npath::string_t extension)
    {
        if (extension != m_extension)
        {
            npath::instance_t* root = m_dirpath.m_device->m_owner;
            root->release_pathstr(m_extension);
            m_extension = m_dirpath.m_device->m_owner->attach_pathstr(extension);
        }
    }

    void filepath_t::setExtension(crunes_t const& extensionstr)
    {
        npath::instance_t*  root = m_dirpath.m_device->m_owner;
        npath::string_t extension = root->find_or_insert_string(extensionstr);
        setExtension(extension);
    }

    dirpath_t filepath_t::root() const { return m_dirpath.root(); }
    dirpath_t filepath_t::dirpath() const { return m_dirpath; }

    filepath_t filepath_t::filename() const { return filepath_t(m_filename, m_extension); }
    filepath_t filepath_t::relative() const
    {
        npath::instance_t* root = m_dirpath.m_device->m_owner;
        filepath_t     fp(nullptr, m_dirpath.m_path, m_filename, m_extension);
        return fp;
    }

    npath::node_t   filepath_t::dirnode() const { return m_dirpath.m_path; }
    npath::string_t filepath_t::filenamestr() const { return m_filename; }
    npath::string_t filepath_t::extensionstr() const { return m_extension; }

    void filepath_t::split(s32 pivot, dirpath_t& left, filepath_t& right) const
    {
        // left.clear();
        // right.clear();
        // m_dirpath.split(pivot, left, right.m_dirpath);
        // left.m_device = m_dirpath.m_device->attach();
        // right.m_dirpath.m_device = m_dirpath.m_device->attach();
        // right.m_filename = m_dirpath.m_device->m_owner->attach(m_filename);
        // right.m_extension = m_dirpath.m_device->m_owner->attach(m_extension);
    }

    void filepath_t::truncate(filepath_t& left, npath::node_t& folder) const
    {
        // left.clear();
        // folder = m_dirpath.basename();
        // npath::instance_t* root = m_dirpath.m_device->m_owner;
        // root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &left.m_dirpath.m_path, nullptr);
        // left.m_dirpath.m_device = m_dirpath.m_device->attach();
        // left.m_filename = m_dirpath.m_device->m_owner->attach(m_filename);
        // left.m_extension = m_dirpath.m_device->m_owner->attach(m_extension);
    }

    void filepath_t::truncate(npath::node_t& folder, filepath_t& filepath) const
    {
        // filepath.clear();
        // folder = m_dirpath.rootname();
        // npath::instance_t* root = m_dirpath.m_device->m_owner;
        // root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &filepath.m_dirpath.m_path, nullptr);
        // filepath.m_dirpath.m_device = m_dirpath.m_device->attach();
        // filepath.m_filename = m_dirpath.m_device->m_owner->attach(m_filename);
        // filepath.m_extension = m_dirpath.m_device->m_owner->attach(m_extension);
    }

    void filepath_t::combine(npath::node_t folder, filepath_t const& filepath) {}
    void filepath_t::combine(filepath_t const& filepath, npath::node_t folder) {}
    void filepath_t::down(crunes_t const& folder) { m_dirpath.down(folder); }

    void filepath_t::up() {}

    // void filepath_t::to_string(runes_t& str) const
    // {
    //     npath::instance_t* root = m_dirpath.m_device->m_owner;

    //     m_dirpath.to_string(str);

    //     crunes_t filenamestr = root->get_crunes(m_filename);
    //     nrunes::concatenate(str, filenamestr);

    //     crunes_t extension_str = root->get_crunes(m_filename);
    //     nrunes::concatenate(str, extension_str);
    // }

    // s32 filepath_t::to_strlen() const
    // {
    //     npath::instance_t* root         = m_dirpath.m_device->m_owner;
    //     crunes_t       filenamestr  = root->get_crunes(m_filename);
    //     crunes_t       extensionstr = root->get_crunes(m_filename);
    //     return m_dirpath.to_strlen() + filenamestr.len() + extensionstr.len();
    // }

    s32 filepath_t::compare(const filepath_t& right) const
    {
        npath::instance_t* root = m_dirpath.m_device->m_owner;
        s32 const      fe   = root->compare_str(m_filename, right.m_filename);
        if (fe != 0)
            return fe;
        s32 const ce = root->compare_str(m_extension, right.m_extension);
        if (ce != 0)
            return ce;
        return m_dirpath.compare(right.m_dirpath);
    }

    filepath_t& filepath_t::operator=(const filepath_t& fp)
    {
        m_dirpath           = fp.m_dirpath;
        npath::instance_t* root = m_dirpath.m_device->m_owner;
        root->release_pathstr(m_filename);
        root->release_pathstr(m_extension);
        m_filename  = fp.m_dirpath.m_device->m_owner->attach_pathstr(m_filename);
        m_extension = fp.m_dirpath.m_device->m_owner->attach_pathstr(m_extension);
        return *this;
    }

    bool operator==(const filepath_t& left, const filepath_t& right) { return left.compare(right) == 0; }
    bool operator!=(const filepath_t& left, const filepath_t& right) { return left.compare(right) != 0; }

    filepath_t operator+(const dirpath_t& dirpath, const filepath_t& filepath) { return filepath_t(dirpath, filepath.filenamestr(), filepath.extensionstr()); }

} // namespace ncore
