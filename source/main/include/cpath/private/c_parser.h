#ifndef __C_PATH_PATH_PARSER_H__
#define __C_PATH_PATH_PARSER_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_allocator.h"
#include "cbase/c_runes.h"

namespace ncore
{
    namespace npath
    {
        class parser_t
        {
        public:
            crunes_t m_device;
            crunes_t m_path;
            crunes_t m_filename;
            crunes_t m_extension;
            crunes_t m_first_folder;

            void parse(const crunes_t& fullpath);

            bool has_device() const;
            bool has_path() const;
            bool has_filename() const;
            bool has_extension() const;

            crunes_t deviceAndPath() const;
            crunes_t path() const;

            crunes_t iterate_folder() const;
            bool     next_folder(crunes_t& folder) const;
        };
    } // namespace npath
} // namespace ncore

#endif
