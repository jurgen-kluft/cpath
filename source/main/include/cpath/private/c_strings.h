#ifndef __C_PATH_STRINGS_H__
#define __C_PATH_STRINGS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_runes.h"
#include "cpath/private/c_freelist.h"
#include "cpath/private/c_tree.h"
#include "cpath/private/c_memory.h"

namespace ncore
{
    class alloc_t;

    namespace npath
    {
        typedef u32 string_t;

        struct strings_t
        {
            strings_t();

            void init(u32 max_items = 16 * 1024 * 1024);
            void exit();

            string_t attach(string_t node);
            string_t detach(string_t node);

            string_t find(crunes_t const& str);
            string_t insert(crunes_t const& str);

            u32  get_len(string_t index) const;
            s8   compare(string_t left, string_t right) const;
            void view_string(string_t str, utf8::pcrune& out_str, u32& out_len) const;

            DCORE_CLASS_PLACEMENT_NEW_DELETE

        private:
            struct object_t
            {
                u32          m_hash; // The hash of the string to speed-up comparison
                u32          m_len;  // Length of the string, excluding null terminator
                utf8::pcrune m_str;  // Points into 'm_data_buffer'
            };
            string_t  object_to_index(object_t const* item) const { return m_str_array.idx_of(item); }
            object_t* index_to_object(string_t index) const { return m_str_array.ptr_of(index); }

            s8        compare_str(object_t const* strA, object_t const* strB) const;
            static s8 compare_str(u32 const find_item, u32 const node_item, void const* user_data);

            memory_t             m_data_buffer; // Virtual memory for holding utf8 strings
            u8*                  m_data_ptr;    // Cursor in the data buffer
            freelist_t<object_t> m_str_array;   // Virtual memory array of string_t[]
            tree_t               m_str_tree;    // Red-black tree of string_t[]
            node_t               m_str_root;    // Tree root, all strings
        };

    } // namespace npath
} // namespace ncore

#endif
