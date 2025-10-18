#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cpath/c_path.h"
#include "cpath/c_dirpath.h"
#include "cpath/c_filepath.h"
#include "cpath/c_device.h"

namespace ncore
{
    filepath_t::filepath_t(npath::device_t* d) : m_dirpath(d), m_filename(npath::c_empty_string), m_extension(npath::c_empty_string) {}
    filepath_t::filepath_t(const filepath_t& other) : m_dirpath(other.m_dirpath)
    {
        m_filename  = other.m_filename;
        m_extension = other.m_extension;
    }

    filepath_t::filepath_t(npath::device_t* device, npath::string_t filename, npath::string_t extension) : m_dirpath(device)
    {
        m_filename  = filename;
        m_extension = extension;
    }

    filepath_t::filepath_t(npath::device_t* device, npath::node_t path, npath::string_t filename, npath::string_t extension) : m_dirpath(device, path)
    {
        m_filename  = filename;
        m_extension = extension;
    }

    filepath_t::filepath_t(dirpath_t const& dirpath, npath::string_t filename, npath::string_t extension) : m_dirpath(dirpath)
    {
        m_filename  = filename;
        m_extension = extension;
    }

    filepath_t::~filepath_t() {}

    void filepath_t::clear()
    {
        // npath::paths_t* root = m_dirpath.m_device->m_owner;
        m_filename  = 0;
        m_extension = 0;
    }

    bool filepath_t::isRooted() const { return m_dirpath.isRooted(); }
    bool filepath_t::isEmpty() const { return m_dirpath.isEmpty() && m_filename == 0 && m_extension == 0; }

    void filepath_t::makeRelativeTo(const dirpath_t& dirpath) { m_dirpath = m_dirpath.makeRelative(dirpath); }
    void filepath_t::makeAbsoluteTo(const dirpath_t& dirpath) { m_dirpath = m_dirpath.makeAbsolute(dirpath); }

    dirpath_t filepath_t::dirpath() const { return m_dirpath; }

    void filepath_t::down(crunes_t const& folder) { m_dirpath.down(folder); }
    void filepath_t::up() {}

    // void filepath_t::to_string(runes_t& str) const
    // {
    //     npath::paths_t* root = m_dirpath.m_device->m_owner;

    //     m_dirpath.to_string(str);

    //     crunes_t filenamestr = root->get_crunes(m_filename);
    //     nrunes::concatenate(str, filenamestr);

    //     crunes_t extension_str = root->get_crunes(m_extension);
    //     nrunes::concatenate(str, extension_str);
    // }

    // s32 filepath_t::to_strlen() const
    // {
    //     npath::paths_t* root         = m_dirpath.m_device->m_owner;
    //     crunes_t       filenamestr  = root->get_crunes(m_filename);
    //     crunes_t       extensionstr = root->get_crunes(m_filename);
    //     return m_dirpath.to_strlen() + filenamestr.len() + extensionstr.len();
    // }

    s8 filepath_t::compare(const filepath_t& right) const
    {
        npath::paths_t* root = m_dirpath.m_device->m_owner;
        s8 const        fe   = root->compare_str(m_filename, right.m_filename);
        if (fe != 0)
            return fe;
        s8 const ce = root->compare_str(m_extension, right.m_extension);
        if (ce != 0)
            return ce;
        return m_dirpath.compare(right.m_dirpath);
    }

} // namespace ncore
