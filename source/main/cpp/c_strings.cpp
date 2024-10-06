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

        strings_t::strings_t() : m_data_buffer(), m_data_ptr(nullptr), m_str_array(), m_str_tree(), m_str_root(0) {}

        void strings_t::init(u32 max_items)
        {
            m_data_buffer.init(max_items, max_items * 32, sizeof(u8));
            m_str_array.init(max_items);
            m_str_tree.init(max_items);

            m_data_ptr = m_data_buffer.m_ptr;
            m_str_root = 0;
        }

        void strings_t::exit()
        {
            m_str_tree.exit();
            m_str_array.exit();
            m_data_buffer.exit();
            m_data_ptr = nullptr;
        }

        string_t strings_t::attach(string_t node) { return node; }
        string_t strings_t::detach(string_t node) { return 0; }

        string_t strings_t::find(crunes_t const& _str)
        {
            utf8::pcrune      str8 = _str.m_utf8 + _str.m_str;
            utf8::pcrune      src8 = str8;
            utf8::pcrune      end8 = _str.m_utf8 + _str.m_end;
            utf8::prune const dst  = (utf8::prune)m_data_buffer.reserve(m_data_ptr, (end8 - str8) + 1, sizeof(u8), 1024 * 1024);
            utf8::prune       dst8 = dst;
            while (str8 < end8)
                *dst8++ = *src8++;
            dst->r = 0;

            object_t* str       = m_str_array.alloc();
            u32 const str_index = object_to_index(str);
            str->m_str          = dst;
            str->m_hash         = hash(str8, end8);
            str->m_len          = (end8 - str8);

            node_t node = m_str_tree.find(m_str_root, str_index, compare_str, this);
            if (node != 0)
            {
                m_str_array.free(str);
                u32 const index = m_str_tree.get_item(node);
                return index;
            }

            m_str_array.free(str);
            return c_invalid_string;
        }

        string_t strings_t::insert(crunes_t const& _str)
        {
            utf8::pcrune      str8 = _str.m_utf8 + _str.m_str;
            utf8::pcrune      src8 = str8;
            utf8::pcrune      end8 = _str.m_utf8 + _str.m_end;
            utf8::prune const dst  = (utf8::prune)m_data_buffer.reserve(m_data_ptr, (end8 - str8) + 1, sizeof(u8), 1024 * 1024);
            utf8::prune       dst8 = dst;
            while (str8 < end8)
                *dst8++ = *src8++;
            dst->r = 0;

            object_t* str       = m_str_array.alloc();
            u32 const str_index = object_to_index(str);
            str->m_str          = dst;
            str->m_hash         = hash(str8, end8);
            str->m_len          = (end8 - str8);

            node_t node = m_str_tree.find(m_str_root, str_index, compare_str, this);
            if (node != 0)
            {
                m_str_array.free(str);
                u32 const index = m_str_tree.get_item(node);
                return index;
            }

            m_data_buffer.commit(m_data_ptr, (end8 - str8) + 1, sizeof(u8));
            m_str_tree.insert(m_str_root, str_index, compare_str, this);
            return str_index;
        }

        s8 strings_t::compare_str(object_t const* strA, object_t const* strB) const
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
            strings_t const*           strings  = (strings_t const*)user_data;
            strings_t::object_t const* find_str = strings->index_to_object(find_item);
            strings_t::object_t const* node_str = strings->index_to_object(node_item);
            return strings->compare_str(find_str, node_str);
        }

        u32 strings_t::get_len(string_t index) const { return index_to_object(index)->m_len; }

        void strings_t::view_string(string_t _str, utf8::pcrune& out_str, u32& out_len) const
        {
            object_t* str = m_str_array.ptr_of(_str);
            out_str       = str->m_str;
            out_len       = str->m_len;
        }

        s8 strings_t::compare(string_t left, string_t right) const
        {
            object_t const* left_str  = index_to_object(left);
            object_t const* right_str = index_to_object(right);
            return compare_str(left_str, right_str);
        }

    } // namespace npath
} // namespace ncore
