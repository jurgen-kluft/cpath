#ifndef __C_PATH_FILEPATH_H__
#define __C_PATH_FILEPATH_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cpath/c_dirpath.h"
#include "cpath/c_types.h"

namespace ncore
{
    //==============================================================================
    // filepath_t:
    //		- Relative:		"FolderA\FolderB\Filename.ext"
    //		- Absolute:		"Device:\FolderA\FolderB\Filename.ext"
    //
    // Root                     = "Device:\"
    // Parent                   = "Device:\FolderA\"
    // Dir                      = "\FolderA\FolderB\Filename.ext"
    // Filename                 = "Filename.ext"
    // FilenameWithoutExtension = "Filename"
    //==============================================================================

    class filepath_t
    {
        dirpath_t       m_dirpath;
        npath::string_t m_filename;
        npath::string_t m_extension;

        friend class fileinfo_t;
        friend class filedevice_t;
        friend struct npath::paths_t;

    public:
        filepath_t(npath::device_t* d);
        filepath_t(const filepath_t&);
        explicit filepath_t(npath::device_t* device, npath::string_t filename, npath::string_t extension);
        explicit filepath_t(npath::device_t* device, npath::node_t dirpath, npath::string_t filename, npath::string_t extension);
        explicit filepath_t(dirpath_t const& dirpath, npath::string_t filename, npath::string_t extension);
        ~filepath_t();

        void clear();
        bool isRooted() const;
        bool isEmpty() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        dirpath_t dirpath() const;

        void down(crunes_t const& folder);
        void up();

        s8 compare(const filepath_t& right) const;

        // void to_string(runes_t& str) const;
        // s32  to_strlen() const;
    };

    inline bool operator==(const filepath_t& left, const filepath_t& right) { return left.compare(right) == 0; }
    inline bool operator!=(const filepath_t& left, const filepath_t& right) { return left.compare(right) != 0; }

}; // namespace ncore

#endif
