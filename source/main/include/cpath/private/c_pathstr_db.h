#ifndef __C_PATH_STR_DB_H__
#define __C_PATH_STR_DB_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "cpath/private/c_virtual_array.h"
#include "cpath/private/c_rbtree.h"
#include "cpath/private/c_pathstr_db.h"

namespace ncore
{
    class alloc_t;

    typedef u32 pathstr_t;

    struct pathstring_t
    {
        u32          m_hash;
        u32          m_len;
        utf8::pcrune m_str;
    };

    struct pathstr_db_t
    {
        pathstr_db_t();

        void          init(alloc_t* allocator, u32 cap = 1024 * 1024);
        void          exit(alloc_t* allocator);
        pathstring_t* attach(pathstring_t* node);
        pathstring_t* detach(pathstring_t* node);

        pathstring_t* findOrInsert(crunes_t const& str);
        bool          remove(pathstring_t* item);
        u32           get_len(pathstr_t index) const;
        pathstr_t     to_idx(pathstring_t const* item) const { return m_str_array.idx_of(item); }
        pathstring_t* to_ptr(pathstr_t index) const { return m_str_array.ptr_of(index); }
        void          to_string(pathstr_t str, crunes_t& out_str) const;
        void          to_string(pathstring_t const* str, crunes_t& out_str) const;
        s32           compare(pathstr_t left, pathstr_t right);

        void*                            m_text_data;      // Virtual memory array (text_t[])
        u64                              m_text_data_size; // Current size of the text data
        u64                              m_text_data_cap;  // Current capacity of the text data
        virtual_freelist_t<pathstring_t> m_str_array;      // Virtual memory array of pathstring_t[]
        rbtree_t                         m_str_tree;       // Red-black tree of pathstring_t[]
        rbnode_t                         m_str_root;       // Tree root, all strings
    };

}; // namespace ncore

#endif
