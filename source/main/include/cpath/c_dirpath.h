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
    // dirpath_t
    // A path is never relative, it is always absolute. There is a way to make a path
    // appear relative to another path, but the path itself is always absolute.
    //==============================================================================

    namespace npath
    {
        struct root_t;
        struct device_t;
        typedef u32 node_t;
        typedef u32 string_t;
    } // namespace npath

    class dirpath_t
    {
    protected:
        npath::device_t* m_device; // "E:\" (the file device)
        npath::node_t    m_base;   // "" or "documents\old\inventory\books\"
        npath::node_t    m_path;   // "documents\old\inventory\books\sci-fi\"

        friend class fileinfo_t;
        friend class dirinfo_t;
        friend class filepath_t;
        friend class filedevice_t;
        friend struct npath::root_t;

    public:
        dirpath_t();
        dirpath_t(dirpath_t const& other);
        dirpath_t(npath::device_t* device);
        dirpath_t(npath::device_t* device, npath::node_t path);
        ~dirpath_t();

        dirpath_t& operator=(dirpath_t const& other);

        void clear();
        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;

        void makeRelativeTo(const dirpath_t& dirpath);
        void makeAbsoluteTo(const dirpath_t& dirpath);

        npath::string_t devname() const;  // "E:\documents\old\inventory\", -> "E:\"
        npath::string_t rootname() const; // "E:\documents\old\inventory\", -> "documents"
        npath::string_t basename() const; // "E:\documents\old\inventory\", -> "inventory"

        dirpath_t  device() const;                 // "E:\documents\old\inventory\books\sci-fi\", -> "E:\"
        dirpath_t  root() const;                   // "E:\documents\old\inventory\books\sci-fi\", -> "E:\documents\"
        dirpath_t  parent() const;                 // "E:\documents\old\inventory\books\sci-fi\", -> "E:\documents\old\inventory\books\"
        filepath_t file(crunes_t const& filename); // "E:\documents\old\inventory\books\sci-fi\" + "perry-rhodan.pdf", -> "E:\documents\old\inventory\books\sci-fi\perry-rhodan.pdf"
        s32        depth() const;
        void       down(crunes_t const& folder);
        void       up();

        s32 compare(const dirpath_t& other) const;

        void to_string(runes_t& str) const;
        s32  to_strlen() const;
    };

    inline bool operator==(const dirpath_t& left, const dirpath_t& right) { return left.compare(right) == 0; }
    inline bool operator!=(const dirpath_t& left, const dirpath_t& right) { return left.compare(right) != 0; }

}; // namespace ncore

#endif // __C_PATH_DIRPATH_H__
