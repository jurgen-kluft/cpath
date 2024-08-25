#ifndef __C_PATH_STRINGS_H__
#define __C_PATH_STRINGS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "cpath/private/c_freelist.h"
#include "cpath/private/c_tree.h"

namespace ncore
{
    class alloc_t;

    namespace npath
    {
        typedef u32 istring_t;
        struct string_t
        {
            u32          m_hash;
            u32          m_len;
            utf8::pcrune m_str;
        };

        struct strings_t
        {
            strings_t();

            void init(alloc_t* allocator, u32 cap = 16 * 1024 * 1024);
            void exit(alloc_t* allocator);

            string_t* attach(string_t* node);
            string_t* detach(string_t* node);

            istring_t findOrInsert(crunes_t const& str);
            bool      remove(string_t* item);
            u32       get_len(istring_t index) const;
            istring_t to_idx(string_t const* item) const { return m_str_array.idx_of(item); }
            string_t* to_ptr(istring_t index) const { return m_str_array.ptr_of(index); }
            void      to_string(istring_t str, crunes_t& out_str) const;
            void      to_string(string_t const* str, crunes_t& out_str) const;
            s8        compare(istring_t left, istring_t right) const;
            s8        compare(string_t const* left, string_t const* right) const;

            void*                m_text_data;      // Virtual memory array (text_t[])
            u64                  m_text_data_size; // Current size of the text data
            u64                  m_text_data_cap;  // Current capacity of the text data
            freelist_t<string_t> m_str_array;      // Virtual memory array of string_t[]
            tree_t               m_str_tree;       // Red-black tree of string_t[]
            inode_t              m_str_root;       // Tree root, all strings
        };

    } // namespace npath
} // namespace ncore

#endif
