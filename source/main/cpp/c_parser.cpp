#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/private/c_parser.h"

namespace ncore
{
    namespace npath
    {
        // -------------------------------------------------------------------------------------------
        // path parser
        //
        bool parser_t::has_device() const { return !is_empty(m_device); }
        bool parser_t::has_path() const { return !is_empty(m_path); }
        bool parser_t::has_filename() const { return !is_empty(m_filename); }
        bool parser_t::has_extension() const { return !is_empty(m_extension); }

        crunes_t parser_t::iterate_folder() const { return m_first_folder; }

        bool parser_t::parse(const crunes_t& fullpath)
        {
#if defined(OS_WINDOWS)
            return parse(fullpath, WINDOWS);
#elif defined(OS_MACOS)
            return parse(fullpath, MACOS);
#elif defined(OS_LINUX)
            return parse(fullpath, LINUX);
#else
            return false;
#endif
        }

        bool parser_t::parse(const crunes_t& fullpath, s8 os)
        {
            if (os == MACOS || os == LINUX)
            {
                // MacOS, path format is /Volume/Path/To/File
                if (nrunes::first_char(fullpath) == '/')
                {
                    // We will manually iterate the string to find the device, path, filename and extension


                    return true;
                }
                else
                {
                    m_device       = crunes_t();
                    m_path         = nrunes::findLastSelectUntilIncluded(fullpath, '/');
                    m_filename     = nrunes::selectAfterExclude(fullpath, m_path);
                    m_filename     = nrunes::findLastSelectUntil(m_filename, '.');
                    m_extension    = nrunes::selectAfterExclude(fullpath, m_filename);
                    m_first_folder = nrunes::findSelectUntilIncluded(m_path, '/');
                    return true;
                }
            }
            else if (os == WINDOWS)
            {
                m_device = nrunes::findSelectUntilIncluded(fullpath, ':');
                nrunes::selectMoreRight(m_device, '\\');
                crunes_t path = nrunes::selectAfterExclude(fullpath, m_device);
                nrunes::trimLeft(path, '/');
                m_path         = nrunes::findLastSelectUntilIncluded(fullpath, '/');
                m_filename     = nrunes::selectAfterExclude(fullpath, m_path);
                m_filename     = nrunes::findLastSelectUntil(m_filename, '.');
                m_extension    = nrunes::selectAfterExclude(fullpath, m_filename);
                m_first_folder = nrunes::findSelectUntilIncluded(m_path, '/');
                return true;
            }
            return false;
        }

        // example: e:\projects\binary_reader\bin\book.pdf, "e:\projects\binary_reader\bin\"
        crunes_t parser_t::deviceAndPath() const { return nrunes::selectFromToInclude(m_device, m_device, m_path); }

        // example: e:\projects\binary_reader\bin\book.pdf, "projects\binary_reader\bin\"
        crunes_t parser_t::path() const { return m_path; }

        // example: projects\binary_reader\bin\, "projects\" -> "binary_reader\" -> "bin\"
        bool parser_t::next_folder(crunes_t& folder, char slash) const
        {
            folder = nrunes::selectAfterExclude(m_path, folder);
            nrunes::trimLeft(folder, slash);
            folder = nrunes::findSelectUntil(folder, slash);
            return !is_empty(folder);
        }
    } // namespace npath
} // namespace ncore
