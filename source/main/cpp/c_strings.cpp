#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/private/c_strings.h"
#include "cpath/c_path.h"

namespace ncore
{
    namespace npath
    {
        static u32 hash(utf8::pcrune str, utf8::pcrune end)
        {
            u32 hash = 0;
            while (str < end)
            {
                utf8::rune r = *str++;
                hash         = hash + r.r * 31;
            }
            return hash;
        }

        strings_t::data_t::data_t() : m_data_buffer(), m_data_ptr(nullptr), m_str_buffer(),  m_str_tree(), m_str_root(0) {}

        strings_t::strings_t() : m_data() {}

        void strings_t::init(u32 max_items)
        {
            m_data.m_data_buffer.init(max_items, max_items * 32, sizeof(u8));
            m_data.m_str_buffer.init(max_items, max_items * 32, sizeof(str_t));
            m_data.m_str_tree.init(max_items);

            m_data.m_data_ptr = m_data.m_data_buffer.m_ptr;
            m_data.m_str_root = 0;
        }

        void strings_t::exit()
        {
            m_data.m_str_tree.exit();
            m_data.m_str_buffer.exit();
            m_data.m_data_buffer.exit();
            m_data.m_data_ptr = nullptr;
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
            utf8::pcrune      str8 = _str.m_utf8 + _str.m_str;
            utf8::pcrune      src8 = str8;
            utf8::pcrune      end8 = _str.m_utf8 + _str.m_end;
            utf8::prune const dst  = (utf8::prune)m_data.m_data_buffer.reserve(m_data.m_data_ptr, (end8 - str8) + 1, sizeof(u8));
            utf8::prune       dst8 = dst;
            while (str8 < end8)
                *dst8++ = *src8++;
            dst->r = 0;

            str_t* const str = (str_t*)m_data.m_str_buffer.ptr_of(m_data.m_str_tree.find_slot(), sizeof(str_t));
            str->m_str       = dst;
            str->m_hash      = hash(str8, end8);
            str->m_len       = (end8 - str8);
            u32 const istr   = m_data.m_str_buffer.idx_of((u8 const*)str, sizeof(str_t));

            node_t node = m_data.m_str_tree.find(m_data.m_str_root, istr, s_compare_str_to_node, this);
            return node;
        }

        string_t strings_t::insert(crunes_t const& _str)
        {
            utf8::pcrune      str8 = _str.m_utf8 + _str.m_str;
            utf8::pcrune      src8 = str8;
            utf8::pcrune      end8 = _str.m_utf8 + _str.m_end;
            utf8::prune const dst  = (utf8::prune)m_data.m_data_buffer.reserve(m_data.m_data_ptr, (end8 - str8) + 1, sizeof(u8));
            utf8::prune       dst8 = dst;
            while (str8 < end8)
                *dst8++ = *src8++;
            dst->r = 0;

            str_t* const str = (str_t*)m_data.m_str_buffer.ptr_of(m_data.m_str_tree.find_slot(), sizeof(str_t));
            str->m_str       = dst;
            str->m_hash      = hash(str8, end8);
            str->m_len       = (end8 - str8);
            u32 const istr   = m_data.m_str_buffer.idx_of((u8 const*)str, sizeof(str_t));

            // TODO, check if we need to grow m_str_buffer

            node_t inserted_or_found;
            if (m_data.m_str_tree.insert(m_data.m_str_root, istr, s_compare_str_to_node, this, inserted_or_found))
            {
                m_data.m_data_buffer.commit(m_data.m_data_ptr, (end8 - str8) + 1, sizeof(u8));
                str_t* dst = index_to_object(inserted_or_found);
                *dst       = *str;
            }

            return inserted_or_found;
        }

        s8 strings_t::compare_str(str_t const* strA, str_t const* strB) const
        {
            if (strA->m_hash != strB->m_hash)
                return strA->m_hash < strB->m_hash ? -1 : 1;

            if (strA->m_len != strB->m_len)
                return strA->m_len < strB->m_len ? -1 : 1;

            utf8::pcrune strA8 = strA->m_str;
            utf8::pcrune strB8 = strB->m_str;
            utf8::pcrune endA8 = strA8 + strA->m_len;
            utf8::pcrune endB8 = strB8 + strB->m_len;
            while (strA8 < endA8)
            {
                utf8::rune rA = *strA8++;
                utf8::rune rB = *strB8++;
                if (rA.r != rB.r)
                    return rA.r < rB.r ? -1 : 1;
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

        void strings_t::view_string(string_t _str, utf8::pcrune& out_str, u32& out_len) const
        {
            str_t* str = index_to_object(_str);
            out_str    = str->m_str;
            out_len    = str->m_len;
        }

        s8 strings_t::compare(string_t left, string_t right) const
        {
            str_t const* left_str  = index_to_object(left);
            str_t const* right_str = index_to_object(right);
            return compare_str(left_str, right_str);
        }

    } // namespace npath
} // namespace ncore
