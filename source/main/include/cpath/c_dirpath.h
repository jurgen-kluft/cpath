#ifndef __C_PATH_DIRPATH_H__
#define __C_PATH_DIRPATH_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

namespace ncore
{
    class filepath_t;

    //==============================================================================
    // dirpath_t:
    //		- Relative:		"FolderA\FolderB\"
    //		- Absolute:		"Device:\FolderA\FolderB\"
    //
    // Root                     = "Device:\"
    // Parent                   = "Device:\FolderA\"
    // Dir                      = "\FolderA\FolderB\"
    //==============================================================================

    namespace npath
    {
        struct root_t;
        struct device_t;
        typedef u32 inode_t;
        typedef u32 istring_t;
    } // namespace npath

    class dirpath_t
    {
    protected:
        npath::device_t* m_device; // "E:\" (the file device)
        npath::inode_t   m_path;   // "documents\old\inventory\books\sci-fi\"

        friend class fileinfo_t;
        friend class dirinfo_t;
        friend class filepath_t;
        friend class filedevice_t;
        friend class npath::root_t;

    public:
        dirpath_t();
        dirpath_t(dirpath_t const& other);
        dirpath_t(npath::device_t* device);
        dirpath_t(npath::device_t* device, npath::inode_t path);
        ~dirpath_t();

        void clear();
        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        dirpath_t relative();

        npath::istring_t devname() const;  // "E:\documents\old\inventory\", -> "E:\"
        npath::istring_t rootname() const; // "E:\documents\old\inventory\", -> "documents"
        npath::istring_t basename() const; // "E:\documents\old\inventory\", -> "inventory"

        dirpath_t  device() const;                 // "E:\documents\old\inventory\books\sci-fi\", -> "E:\"
        dirpath_t  root() const;                   // "E:\documents\old\inventory\books\sci-fi\", -> "E:\documents\"
        dirpath_t  parent() const;                 // "E:\documents\old\inventory\books\sci-fi\", -> "E:\documents\old\inventory\books\"
        filepath_t file(crunes_t const& filepath); // "E:\documents\old\inventory\books\sci-fi\" + "perry-rhodan.pdf", -> "E:\documents\old\inventory\books\sci-fi\perry-rhodan.pdf"

        s32 getLevels() const;

        void down(crunes_t const& folder);
        void up();

        s32 compare(const dirpath_t& other) const;

        void to_string(runes_t& str) const;
        s32  to_strlen() const;

        dirpath_t& operator=(dirpath_t const& other);

        static void getSubDir(const dirpath_t& parentpath, const dirpath_t& path, dirpath_t& out_subpath);
    };

    inline bool operator==(const dirpath_t& left, const dirpath_t& right) { return left.compare(right) == 0; }
    inline bool operator!=(const dirpath_t& left, const dirpath_t& right) { return left.compare(right) != 0; }

    extern dirpath_t operator+(const dirpath_t& dirpath, const dirpath_t& append_dirpath);

}; // namespace ncore

#endif // __C_PATH_DIRPATH_H__
