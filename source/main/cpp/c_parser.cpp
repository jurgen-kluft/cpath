#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/private/c_pathparser.h"

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

        void parser_t::parse(const crunes_t& fullpath)
        {
            utf32::rune slash_chars[] = {'\\', '/', '\0'};
            crunes_t    slash(slash_chars);
            utf32::rune devicesep_chars[] = {':', '\\', '\0'};
            crunes_t    devicesep(devicesep_chars);

            m_device          = nrunes::findSelectUntilIncludedAbortAtOneOf(fullpath, devicesep, slash_chars);
            crunes_t filepath = nrunes::selectAfterExclude(fullpath, m_device);
            m_path            = nrunes::findLastSelectUntilIncluded(filepath, slash);
            m_filename        = nrunes::selectAfterExclude(fullpath, m_path);
            m_filename        = nrunes::findLastSelectUntil(m_filename, '.');
            m_extension       = nrunes::selectAfterExclude(fullpath, m_filename);
            m_first_folder    = nrunes::findSelectUntilIncluded(m_path, slash);
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

        bool parser_t::next_folder(crunes_t& folder) const
        {
            // example: projects\binary_reader\bin\, "projects\" -> "binary_reader\" -> "bin\"
            folder = nrunes::selectAfterExclude(m_path, folder);
            nrunes::trimLeft(folder, '\\');
            folder = nrunes::findSelectUntil(folder, '\\');
            return !folder.is_empty();
        }
    } // namespace npath
} // namespace ncore
