#ifndef __C_PATH_STRINGS_H__
#define __C_PATH_STRINGS_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cpath/c_types.h"

namespace ncore
{
    class alloc_t;

    namespace npath
    {
        struct strings_t
        {
            strings_t();

            struct members_t;

            struct str_t
            {
                u32   m_hash; // The hash of the string to speed-up comparison
                u32   m_len;  // Length of the string, excluding null terminator
                char* m_str;  // Points into 'm_data_buffer'
            };

            string_t attach(string_t node);
            string_t detach(string_t node);

            string_t find(crunes_t const& str);
            string_t insert(crunes_t const& str);

            u32  get_len(string_t index) const;
            s8   compare(string_t left, string_t right) const;
            void view_string(string_t str, crunes_t& out_str) const;

            string_t object_to_index(str_t const* item) const; // { return m_data.m_str_buffer.idx_of((u8 const*)item, sizeof(str_t)); }
            str_t*   index_to_object(string_t index) const;    // { return (str_t*)m_data.m_str_buffer.ptr_of(index, sizeof(str_t)); }

            s8        compare_str(str_t const* strA, str_t const* strB) const;
            static s8 compare_str(u32 const find_item, u32 const node_item, void const* user_data);

            DCORE_CLASS_PLACEMENT_NEW_DELETE

            members_t* m_data;
        };

        strings_t* g_construct_strings(alloc_t* allocator, u64 max_items = 16 * 1024 * 1024);
        void       g_destruct_strings(alloc_t* allocator, strings_t*& strings);

    } // namespace npath
} // namespace ncore

#endif
