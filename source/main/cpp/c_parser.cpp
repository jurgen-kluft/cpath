#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
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
        bool parser_t::has_device() const { return !m_device.is_empty(); }
        bool parser_t::has_path() const { return !m_path.is_empty(); }
        bool parser_t::has_filename() const { return !m_filename.is_empty(); }
        bool parser_t::has_extension() const { return !m_extension.is_empty(); }

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

                crunes_t slashes("/", 0, 1, 1);
                if (nrunes::first_char(fullpath) == '/')
                {
                    crunes_t adjustedpath = fullpath.view(1);

                    m_device       = nrunes::findLastSelectUntil(adjustedpath, '/');
                    m_path         = nrunes::findLastSelectUntilIncluded(adjustedpath, slashes);
                    m_filename     = nrunes::selectAfterExclude(adjustedpath, m_path);
                    m_filename     = nrunes::findLastSelectUntil(m_filename, '.');
                    m_extension    = nrunes::selectAfterExclude(adjustedpath, m_filename);
                    m_first_folder = nrunes::findSelectUntilIncluded(m_path, slashes);
                    return true;
                }
                else
                {
                    m_device       = crunes_t();
                    m_path         = nrunes::findLastSelectUntilIncluded(fullpath, slashes);
                    m_filename     = nrunes::selectAfterExclude(fullpath, m_path);
                    m_filename     = nrunes::findLastSelectUntil(m_filename, '.');
                    m_extension    = nrunes::selectAfterExclude(fullpath, m_filename);
                    m_first_folder = nrunes::findSelectUntilIncluded(m_path, slashes);
                    return true;
                }
            }
            else if (os == WINDOWS)
            {
                crunes_t slashes("\\/", 0, 2, 2);
                crunes_t devicesep(":\\", 0, 2, 2);

                m_device          = nrunes::findSelectUntilIncludedAbortAtOneOf(fullpath, devicesep, slashes);
                crunes_t filepath = nrunes::selectAfterExclude(fullpath, m_device);
                m_path            = nrunes::findLastSelectUntilIncluded(filepath, slashes);
                m_filename        = nrunes::selectAfterExclude(fullpath, m_path);
                m_filename        = nrunes::findLastSelectUntil(m_filename, '.');
                m_extension       = nrunes::selectAfterExclude(fullpath, m_filename);
                m_first_folder    = nrunes::findSelectUntilIncluded(m_path, slashes);
                return true;
            }
            return false;
        }

        crunes_t parser_t::deviceAndPath() const
        {
            // example: e:\projects\binary_reader\bin\book.pdf, "e:\projects\binary_reader\bin\"
            return nrunes::selectFromToInclude(m_device, m_device, m_path);
        }

        crunes_t parser_t::path() const
        {
            // example: e:\projects\binary_reader\bin\book.pdf, "projects\binary_reader\bin\"
            return m_path;
        }

        bool parser_t::next_folder(crunes_t& folder, char slash) const
        {
            // example: projects\binary_reader\bin\, "projects\" -> "binary_reader\" -> "bin\"
            folder = nrunes::selectAfterExclude(m_path, folder);
            nrunes::trimLeft(folder, slash);
            folder = nrunes::findSelectUntil(folder, slash);
            return !folder.is_empty();
        }
    } // namespace npath
} // namespace ncore
