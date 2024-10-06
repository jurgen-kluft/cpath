#ifndef __C_PATH_PATH_PARSER_H__
#define __C_PATH_PATH_PARSER_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cbase/c_runes.h"

namespace ncore
{
    namespace npath
    {
        class parser_t
        {
        public:
            parser_t();

            enum { WINDOWS=0, MACOS=1, LINUX=2 };

            crunes_t m_device;
            crunes_t m_path;
            crunes_t m_filename;
            crunes_t m_extension;
            crunes_t m_first_folder;

            bool parse(const crunes_t& fullpath);
            bool parse(const crunes_t& fullpath, s8 os);

            bool has_device() const;
            bool has_path() const;
            bool has_filename() const;
            bool has_extension() const;

            crunes_t deviceAndPath() const;
            crunes_t path() const;

            crunes_t iterate_folder() const;
            bool     next_folder(crunes_t& folder, char slash = '\\') const;
        };
    } // namespace npath
} // namespace ncore

#endif
