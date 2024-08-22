#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cpath/private/c_root.h"
#include "cpath/private/c_pathparser.h"
#include "cpath/c_dirpath.h"
#include "cpath/c_filepath.h"

namespace ncore
{
    filepath_t::filepath_t() : m_dirpath(), m_filename(npath::root_t::sNilStr), m_extension(npath::root_t::sNilStr) {}
    filepath_t::filepath_t(const filepath_t& other) : m_dirpath(other.m_dirpath)
    {
        m_filename  = m_dirpath.m_device->m_root->attach_pathstr(other.m_filename);
        m_extension = m_dirpath.m_device->m_root->attach_pathstr(other.m_extension);
    }

    filepath_t::filepath_t(npath::istring_t filename, npath::istring_t extension) : m_dirpath()
    {
        m_filename  = m_dirpath.m_device->m_root->attach_pathstr(filename);
        m_extension = m_dirpath.m_device->m_root->attach_pathstr(extension);
    }

    filepath_t::filepath_t(npath::device_t* device, npath::inode_t path, npath::istring_t filename, npath::istring_t extension) : m_dirpath(device, path)
    {
        m_filename  = m_dirpath.m_device->m_root->attach_pathstr(filename);
        m_extension = m_dirpath.m_device->m_root->attach_pathstr(extension);
    }

    filepath_t::filepath_t(dirpath_t const& dirpath, npath::istring_t filename, npath::istring_t extension) : m_dirpath(dirpath)
    {
        m_filename  = m_dirpath.m_device->m_root->attach_pathstr(filename);
        m_extension = m_dirpath.m_device->m_root->attach_pathstr(extension);
    }

    filepath_t::~filepath_t()
    {
        npath::root_t* root = m_dirpath.m_device->m_root;
        root->release_pathstr(m_filename);
        root->release_pathstr(m_extension);
    }

    void filepath_t::clear()
    {
        m_dirpath.clear();
        npath::root_t* root = m_dirpath.m_device->m_root;
        root->release_pathstr(m_filename);
        root->release_pathstr(m_extension);
        m_filename  = npath::root_t::sNilStr;
        m_extension = npath::root_t::sNilStr;
    }

    bool filepath_t::isRooted() const { return m_dirpath.isRooted(); }
    bool filepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == npath::root_t::sNilStr && m_extension == npath::root_t::sNilStr; }

    void filepath_t::makeRelativeTo(const dirpath_t& dirpath) { m_dirpath.makeRelativeTo(dirpath); }
    void filepath_t::makeAbsoluteTo(const dirpath_t& dirpath) { m_dirpath.makeAbsoluteTo(dirpath); }
    void filepath_t::setDirpath(dirpath_t const& dirpath) { m_dirpath = dirpath; }

    void filepath_t::setDevice(crunes_t const& devicename)
    {
        npath::root_t* root = m_dirpath.m_device->m_root;

        pathparser_t parser;
        parser.parse(devicename);
        if (parser.has_device())
        {
            s16 const idev     = root->register_device(parser.m_device);
            m_dirpath.m_device = root->get_device(idev);
        }
    }

    void filepath_t::setFilename(npath::istring_t filename)
    {
        if (filename != m_filename)
        {
            npath::root_t* root = m_dirpath.m_device->m_root;
            root->release_pathstr(m_filename);
            m_filename = m_dirpath.m_device->m_root->attach_pathstr(filename);
        }
    }

    void filepath_t::setFilename(crunes_t const& filenamestr)
    {
        npath::root_t*   root          = m_dirpath.m_device->m_root;
        npath::istring_t out_filename  = 0;
        npath::istring_t out_extension = 0;
        root->register_filename(filenamestr, out_filename, out_extension);
        root->release_pathstr(m_filename);
        root->release_pathstr(m_extension);
        m_filename  = m_dirpath.m_device->m_root->attach_pathstr(out_filename);
        m_extension = m_dirpath.m_device->m_root->attach_pathstr(out_extension);
    }

    void filepath_t::setExtension(npath::istring_t extension)
    {
        if (extension != m_extension)
        {
            npath::root_t* root = m_dirpath.m_device->m_root;
            root->release_pathstr(m_extension);
            m_extension = m_dirpath.m_device->m_root->attach_pathstr(extension);
        }
    }

    void filepath_t::setExtension(crunes_t const& extensionstr)
    {
        npath::root_t*   root = m_dirpath.m_device->m_root;
        npath::istring_t extension;
        root->register_name(extensionstr, extension);
        setExtension(extension);
    }

    dirpath_t filepath_t::root() const { return m_dirpath.root(); }
    dirpath_t filepath_t::dirpath() const { return m_dirpath; }

    filepath_t filepath_t::filename() const { return filepath_t(m_filename, m_extension); }
    filepath_t filepath_t::relative() const
    {
        npath::root_t* root = m_dirpath.m_device->m_root;
        filepath_t     fp(root->sNilDevice, m_dirpath.m_path, m_filename, m_extension);
        return fp;
    }

    npath::inode_t   filepath_t::dirnode() const { return m_dirpath.m_path; }
    npath::istring_t filepath_t::filenamestr() const { return m_filename; }
    npath::istring_t filepath_t::extensionstr() const { return m_extension; }

    void filepath_t::split(s32 pivot, dirpath_t& left, filepath_t& right) const
    {
        // left.clear();
        // right.clear();
        // m_dirpath.split(pivot, left, right.m_dirpath);
        // left.m_device = m_dirpath.m_device->attach();
        // right.m_dirpath.m_device = m_dirpath.m_device->attach();
        // right.m_filename = m_dirpath.m_device->m_root->attach(m_filename);
        // right.m_extension = m_dirpath.m_device->m_root->attach(m_extension);
    }

    void filepath_t::truncate(filepath_t& left, npath::inode_t& folder) const
    {
        // left.clear();
        // folder = m_dirpath.basename();
        // npath::root_t* root = m_dirpath.m_device->m_root;
        // root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &left.m_dirpath.m_path, nullptr);
        // left.m_dirpath.m_device = m_dirpath.m_device->attach();
        // left.m_filename = m_dirpath.m_device->m_root->attach(m_filename);
        // left.m_extension = m_dirpath.m_device->m_root->attach(m_extension);
    }

    void filepath_t::truncate(npath::inode_t& folder, filepath_t& filepath) const
    {
        // filepath.clear();
        // folder = m_dirpath.rootname();
        // npath::root_t* root = m_dirpath.m_device->m_root;
        // root->get_split_path(m_dirpath.m_path, m_dirpath.m_path->m_len - 1, &filepath.m_dirpath.m_path, nullptr);
        // filepath.m_dirpath.m_device = m_dirpath.m_device->attach();
        // filepath.m_filename = m_dirpath.m_device->m_root->attach(m_filename);
        // filepath.m_extension = m_dirpath.m_device->m_root->attach(m_extension);
    }

    void filepath_t::combine(npath::inode_t folder, filepath_t const& filepath) {}

    void filepath_t::combine(filepath_t const& filepath, npath::inode_t folder) {}

    void filepath_t::down(crunes_t const& folder) { m_dirpath.down(folder); }

    void filepath_t::up() {}

    void filepath_t::to_string(runes_t& str) const
    {
        npath::root_t* root = m_dirpath.m_device->m_root;

        m_dirpath.to_string(str);

        npath::string_t const* filenamestr = root->get_string(m_filename);
        crunes_t               filename_str(filenamestr->m_str, 0, filenamestr->m_len, filenamestr->m_len);
        nrunes::concatenate(str, filename_str);

        npath::string_t const* extensionstr = root->get_string(m_filename);
        crunes_t               extension_str(extensionstr->m_str, 0, extensionstr->m_len, extensionstr->m_len);
        nrunes::concatenate(str, extension_str);
    }

    s32 filepath_t::to_strlen() const
    {
        npath::root_t* root = m_dirpath.m_device->m_root;
        npath::string_t const* filenamestr = root->get_string(m_filename);
        npath::string_t const* extensionstr = root->get_string(m_filename);
        return m_dirpath.to_strlen() + filenamestr->m_len + extensionstr->m_len;
    }

    s32 filepath_t::compare(const filepath_t& right) const
    {
        npath::root_t* root = m_dirpath.m_device->m_root;
        s32 const fe = root->compare_str(m_filename, right.m_filename);
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
        npath::root_t* root = m_dirpath.m_device->m_root;
        root->release_pathstr(m_filename);
        root->release_pathstr(m_extension);
        m_filename  = fp.m_dirpath.m_device->m_root->attach_pathstr(m_filename);
        m_extension = fp.m_dirpath.m_device->m_root->attach_pathstr(m_extension);
        return *this;
    }

    bool operator==(const filepath_t& left, const filepath_t& right) { return left.compare(right) == 0; }

    bool operator!=(const filepath_t& left, const filepath_t& right) { return left.compare(right) != 0; }

    filepath_t operator+(const dirpath_t& dirpath, const filepath_t& filepath) { return filepath_t(dirpath, filepath.filenamestr(), filepath.extensionstr()); }

} // namespace ncore
