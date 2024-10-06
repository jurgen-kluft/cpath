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
    // 'appear' relative to another path, but the full path itself is always absolute.
    // When no root is provided, it is assumed to be the 'application' root.
    //==============================================================================

    namespace npath
    {
        struct instance_t;
        struct device_t;
        typedef u32 node_t;
        typedef u32 string_t;
    } // namespace npath

    class dirpath_t
    {
    protected:
        npath::device_t* m_device; // "E" (the device)
        npath::node_t    m_base;   //                                      "", "documents\old\"          or "documents\old\inventory\books\"
        npath::node_t    m_path;   // "documents\old\inventory\books\sci-fi\", "inventory\books\sci-fi\" or "sci-fi\"

        friend class fileinfo_t;
        friend class dirinfo_t;
        friend class filepath_t;
        friend class filedevice_t;
        friend struct npath::instance_t;

    public:
        dirpath_t();
        dirpath_t(dirpath_t const& other);
        explicit dirpath_t(npath::device_t* device);
        explicit dirpath_t(npath::device_t* device, npath::node_t path);
        ~dirpath_t();

        void clear();
        bool isEmpty() const;
        bool isRoot() const;
        bool isRooted() const;

        // this = "E:\documents\old\inventory\"
        // dirpath = "E:\documents\old\inventory\books\sci-fi\"
        // returns "books\sci-fi\" (device = "E", base = "documents\old\inventory\", path = "books\sci-fi\")
        dirpath_t makeRelative(const dirpath_t& dirpath) const;
        dirpath_t makeAbsolute(const dirpath_t& dirpath) const;

        npath::string_t devname() const;  // "E:\documents\old\inventory\", -> "E"
        npath::string_t rootname() const; // "E:\documents\old\inventory\", -> "documents"
        npath::string_t basename() const; // "E:\documents\old\inventory\", -> "inventory"
        dirpath_t       device() const;   // "E:\documents\old\inventory\books\sci-fi\", -> "E"
        dirpath_t       root() const;     // "E:\documents\old\inventory\books\sci-fi\", -> "documents"
        dirpath_t       parent() const;   // "E:\documents\old\inventory\books\sci-fi\", -> "documents\old\inventory\books\"

        s32        depth() const;                  // "E:\documents\old\inventory\books\sci-fi\", -> 5
        dirpath_t  up() const;                     // "E:\documents\old\inventory\books\sci-fi\", -> "E:\documents\old\inventory\books\"
        dirpath_t  down() const;                   // return the first child folder of this dirpath
        void       down(crunes_t const& folder);   // return the child folder of this dirpath that matches the folder name
        dirpath_t  next() const;                   // return the next sibling
        filepath_t file(crunes_t const& filename); // "E:\documents\old\inventory\books\sci-fi\" + "perry-rhodan.pdf", -> "E:\documents\old\inventory\books\sci-fi\perry-rhodan.pdf"

        dirpath_t& operator=(dirpath_t const& other);
        s32        compare(const dirpath_t& other) const;
        bool       operator==(dirpath_t const& other) const { return compare(other) == 0; }
        bool       operator!=(dirpath_t const& other) const { return compare(other) != 0; }

        // (device = "E", base = "documents\old\inventory\", path = "books\sci-fi\") -> "books\sci-fi\"
        void relative_path_to_string(runes_t& str) const;
        s32  relative_path_to_strlen() const;

        // (device = "E", base = "documents\old\inventory\", path = "books\sci-fi\") -> "E:\documents\"
        void root_path_to_string(runes_t& str) const;
        s32  root_path_to_strlen() const;

        // (device = "E", base = "documents\old\inventory\", path = "books\sci-fi\") -> "E:\documents\old\inventory\"
        void base_path_to_string(runes_t& str) const;
        s32  base_path_to_strlen() const;

        // (device = "E", base = "documents\old\inventory\", path = "books\sci-fi\") -> "E:\documents\old\inventory\books\sci-fi\"
        void full_path_to_string(runes_t& str) const;
        s32  full_path_to_strlen() const;
    };

    inline bool operator==(const dirpath_t& left, const dirpath_t& right) { return left.compare(right) == 0; }
    inline bool operator!=(const dirpath_t& left, const dirpath_t& right) { return left.compare(right) != 0; }

}; // namespace ncore

#endif // __C_PATH_DIRPATH_H__
