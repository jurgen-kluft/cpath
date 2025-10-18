#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/private/c_strings.h"
#include "cpath/private/c_memory.h"
#include "cpath/c_path.h"

namespace ncore
{
    namespace npath
    {
        struct strings_t::members_t
        {
            varena_t        m_data_buffer; // Virtual memory for holding utf8 strings
            u8*             m_data_ptr;    // Cursor in the data buffer
            varena_t        m_str_buffer;  // Virtual memory for holding string_t[]
            u32             m_size;        // Number of strings allocated
            ntree32::node_t m_str_root;    // Tree root, all strings
            ntree32::tree_t m_str_tree;    // Red-black tree for strings
            varena_t        m_node_array;  // Virtual memory array of nnode_t[]

            DCORE_CLASS_PLACEMENT_NEW_DELETE
        };

        static void s_init_members(strings_t::members_t* m)
        {
            g_init_arena(m->m_data_buffer);
            g_init_arena(m->m_str_buffer);
            m->m_data_ptr = nullptr;
            m->m_str_root = c_invalid_node;
            ntree32::g_init(m->m_str_tree);
        }

        static inline u32 s_find_slot(strings_t::members_t const* data) { return data->m_size; }
        static inline u32 s_temp_slot(strings_t::members_t const* data) { return data->m_size + 1; }

        strings_t::strings_t() : m_data() {}

        strings_t* g_construct_strings(alloc_t* allocator, u64 max_items)
        {
            strings_t* strings = g_construct<strings_t>(allocator);
            strings->m_data    = g_construct<strings_t::members_t>(allocator);
            s_init_members(strings->m_data);

            g_init_arena(strings->m_data->m_data_buffer, 8192, max_items, sizeof(u8));
            g_init_arena(strings->m_data->m_str_buffer, 8192, max_items, sizeof(strings_t::str_t));

            const s32 c_extra_size = 2;
            g_init_arena(strings->m_data->m_node_array, 8192, max_items + c_extra_size, sizeof(ntree32::nnode_t));
            ntree32::setup_tree(strings->m_data->m_str_tree, (ntree32::nnode_t*)strings->m_data->m_node_array.m_ptr);

            strings->m_data->m_data_ptr = strings->m_data->m_data_buffer.m_ptr;
            strings->m_data->m_str_root = c_invalid_node;

            return strings;
        }

        void g_destruct_strings(alloc_t* allocator, strings_t*& strings)
        {
            g_teardown_arena(strings->m_data->m_str_buffer);
            g_teardown_arena(strings->m_data->m_data_buffer);
            g_teardown_arena(strings->m_data->m_node_array);
            ntree32::teardown_tree(strings->m_data->m_str_tree);
            strings->m_data->m_data_ptr = nullptr;
            g_destruct(allocator, strings->m_data);
            g_destruct(allocator, strings);
        }

        string_t strings_t::attach(string_t node) { return node; }
        string_t strings_t::detach(string_t node) { return 0; }

        static s8 s_compare_str_to_node(u32 const _str, u32 const _node, void const* user_data)
        {
            strings_t const*        strings  = (strings_t const*)user_data;
            strings_t::str_t const* str      = strings->index_to_object(_str);
            strings_t::str_t const* node_str = strings->index_to_object(_node);
            return strings->compare_str(str, node_str);
        }

        string_t strings_t::find(crunes_t const& _str)
        {
            ASSERT(_str.m_type == utf8::TYPE || _str.m_type == ascii::TYPE);
            const char* str8 = _str.m_ascii + _str.m_str;
            const char* src8 = str8;
            const char* end8 = _str.m_ascii + _str.m_end;
            char* const dst  = (char*)m_data->m_data_buffer.reserve(m_data->m_data_ptr, (end8 - str8) + 1, sizeof(u8));
            char*       dst8 = dst;
            while (src8 < end8)
                *dst8++ = *src8++;
            *dst = 0;

            str_t* const str = (str_t*)m_data->m_str_buffer.ptr_of(s_find_slot(m_data), sizeof(str_t));
            str->m_str       = dst;
            str->m_hash      = nhash::strhash((const char*)str8, (const char*)end8);
            str->m_len       = (end8 - str8);
            u32 const istr   = m_data->m_str_buffer.idx_of((u8 const*)str, sizeof(str_t));

            ntree32::node_t found_node;
            ntree32::find(m_data->m_str_tree, m_data->m_str_root, istr, s_compare_str_to_node, this, found_node);
            return found_node;
        }

        string_t strings_t::insert(crunes_t const& _str)
        {
            ASSERT(_str.m_type == utf8::TYPE || _str.m_type == ascii::TYPE);
            const char* str8 = _str.m_ascii + _str.m_str;
            const char* src8 = str8;
            const char* end8 = _str.m_ascii + _str.m_end;
            char* const dst  = (char*)m_data->m_data_buffer.reserve(m_data->m_data_ptr, (end8 - str8) + 1, sizeof(u8));
            char*       dst8 = dst;
            while (src8 < end8)
                *dst8++ = *src8++;
            *dst = 0;

            str_t* const str = (str_t*)m_data->m_str_buffer.ptr_of(s_find_slot(m_data), sizeof(str_t));
            str->m_str       = dst;
            str->m_hash      = nhash::strhash((const char*)str8, (const char*)end8);
            str->m_len       = (end8 - str8);
            u32 const istr   = m_data->m_str_buffer.idx_of((u8 const*)str, sizeof(str_t));

            // TODO, check if we need to grow m_str_buffer

            node_t inserted_or_found;
            if (ntree32::insert(m_data->m_str_tree, m_data->m_str_root, s_temp_slot(m_data), istr, s_compare_str_to_node, this, inserted_or_found))
            {
                m_data->m_str_buffer.ensure_capacity(inserted_or_found);
                str_t* dstr = index_to_object(inserted_or_found);
                *dstr       = *str;
            }

            return inserted_or_found;
        }

        string_t          strings_t::object_to_index(str_t const* item) const { return m_data->m_str_buffer.idx_of((u8 const*)item, sizeof(str_t)); }
        strings_t::str_t* strings_t::index_to_object(string_t index) const { return (str_t*)m_data->m_str_buffer.ptr_of(index, sizeof(str_t)); }

        s8 strings_t::compare_str(str_t const* strA, str_t const* strB) const
        {
            if (strA->m_hash != strB->m_hash)
                return strA->m_hash < strB->m_hash ? -1 : 1;

            if (strA->m_len != strB->m_len)
                return strA->m_len < strB->m_len ? -1 : 1;

            const char* strA8 = strA->m_str;
            const char* strB8 = strB->m_str;
            const char* endA8 = strA8 + strA->m_len;
            // const char* endB8 = strB8 + strB->m_len;
            while (strA8 < endA8)
            {
                char rA = *strA8++;
                char rB = *strB8++;
                if (rA != rB)
                    return rA < rB ? -1 : 1;
            }
            return 0;
        }

        s8 strings_t::compare_str(u32 const find_item, u32 const node_item, void const* user_data)
        {
            strings_t const*        strings  = (strings_t const*)user_data;
            strings_t::str_t const* find_str = strings->index_to_object(find_item);
            strings_t::str_t const* node_str = strings->index_to_object(node_item);
            return strings->compare_str(find_str, node_str);
        }

        u32 strings_t::get_len(string_t index) const { return index_to_object(index)->m_len; }

        void strings_t::view_string(string_t _str, crunes_t& out_str) const
        {
            str_t* str = index_to_object(_str);
            out_str    = make_crunes(str->m_str, 0, str->m_len, str->m_len);
        }

        s8 strings_t::compare(string_t left, string_t right) const
        {
            str_t const* left_str  = index_to_object(left);
            str_t const* right_str = index_to_object(right);
            return compare_str(left_str, right_str);
        }

    } // namespace npath
} // namespace ncore
