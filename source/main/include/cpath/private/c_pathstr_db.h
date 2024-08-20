#ifndef __C_PATH_STR_DB_H__
#define __C_PATH_STR_DB_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "cpath/private/c_virtual_array.h"
#include "cpath/private/c_bst.h"
#include "cpath/private/c_pathstr_db.h"

namespace ncore
{
    class alloc_t;

    typedef u32 pathstr_t;

    struct pathstr_db_t
    {
        pathstr_db_t();

        struct str_t
        {
            u32          m_hash;
            u32          m_len;
            utf8::pcrune m_str;
        };

        void   init(alloc_t* allocator, u32 cap = 1024 * 1024);
        void   exit(alloc_t* allocator);
        str_t* attach(str_t* node);
        str_t* detach(str_t* node);

        str_t*    findOrInsert(crunes_t const& str);
        bool      remove(str_t* item);
        u32       get_len(pathstr_t index) const;
        pathstr_t to_index(str_t const* item) const { return m_str_array.index_of(item); }
        str_t*    to_ptr(pathstr_t index) const { return m_str_array.ptr_of(index); }
        void      to_string(pathstr_t str, crunes_t& out_str) const;
        void      to_string(str_t const* str, crunes_t& out_str) const;
        s32       compare(pathstr_t left, pathstr_t right);

        void*                  m_text_data;      // Virtual memory array (text_t[])
        u64                    m_text_data_size; // Current size of the text data
        u64                    m_text_data_cap;  // Current capacity of the text data
        virtual_array_t<str_t> m_str_array;      // Virtual memory array of str_t[]
        rbtree_t               m_str_tree;       // Red-black tree of str_t[]
        rbnode_t               m_str_root;       // Tree root, all strings
    };

}; // namespace ncore

#endif
