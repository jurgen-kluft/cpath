#ifndef __C_PATH_FILEPATH_H__
#define __C_PATH_FILEPATH_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

#include "cpath/c_dirpath.h"

namespace ncore
{
    class dirpath_t;
    class filesystem_t;

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
        dirpath_t        m_dirpath;
        npath::istring_t m_filename;
        npath::istring_t m_extension;

        friend class fileinfo_t;
        friend class filedevice_t;
        friend class npath::root_t;

    public:
        filepath_t();
        filepath_t(const filepath_t&);
        filepath_t(npath::istring_t filename, npath::istring_t extension);
        filepath_t(npath::device_t* device, npath::inode_t dirpath, npath::istring_t filename, npath::istring_t extension);
        filepath_t(dirpath_t const& dirpath, npath::istring_t filename, npath::istring_t extension);
        ~filepath_t();

        void clear();
        bool isRooted() const;
        bool isEmpty() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        void setDevice(crunes_t const& devicename);
        void setDirpath(dirpath_t const& dirpath);
        void setFilename(npath::istring_t* filename);
        void setFilename(crunes_t const& filename);
        void setExtension(npath::istring_t* extension);
        void setExtension(crunes_t const& extension);

        dirpath_t  root() const;
        dirpath_t  dirpath() const;
        filepath_t filename() const;
        filepath_t relative() const;

        npath::inode_t   dirnode() const;
        npath::istring_t filenamestr() const;
        npath::istring_t extensionstr() const;

        void split(s32 pivot, dirpath_t& left, filepath_t& right) const;
        void truncate(filepath_t& filepath, npath::inode_t*& folder) const;
        void truncate(npath::inode_t*& folder, filepath_t& filepath) const;
        void combine(npath::inode_t* folder, filepath_t const& filepath);
        void combine(filepath_t const& filepath, npath::inode_t* folder);

        void down(crunes_t const& folder);
        void up();

        s32  compare(const filepath_t& right) const;
        void to_string(runes_t& str) const;
        s32  to_strlen() const;

        filepath_t& operator=(const filepath_t& fp);
    };

    extern bool operator==(const filepath_t& left, const filepath_t& right);
    extern bool operator!=(const filepath_t& left, const filepath_t& right);

    extern filepath_t operator+(const dirpath_t& dirpath, const filepath_t& filepath);

}; // namespace ncore

#endif
