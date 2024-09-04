#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/private/c_strings.h"

namespace ncore
{
    namespace npath
    {
        strings_t::strings_t() : m_data_buffer(), m_data_ptr(nullptr), m_str_array(), m_str_tree(), m_str_root(0) {}

        void strings_t::init(u32 max_items)
        {
            m_data_buffer.init(1024 * 1024, max_items * 32, sizeof(u8));
            m_data_ptr = m_data_buffer.m_ptr;
            m_str_array.init(max_items);
            m_str_tree.init( max_items);

            m_str_root = 0;
        }

        void strings_t::exit()
        {
            m_str_tree.exit();
            m_str_array.exit();
            m_data_buffer.exit();
        }

        string_t strings_t::attach(string_t node) { return node; }
        string_t strings_t::detach(string_t node) { return 0; }

        static u32 hash(utf8::pcrune str, utf8::pcrune end)
        {
            u32 hash = 0;
            while (str < end)
            {
                utf8::rune r = *str++;
                hash         = hash + r.value * 31;
            }
            return hash;
        }

        s8 strings_t::compare_str(obj_t const* strA, obj_t const* strB) const
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
                if (rA.value != rB.value)
                    return rA.value < rB.value ? -1 : 1;
            }
            return 0;
        }

        s8 strings_t::compare_str(u32 const find_item, u32 const node_item, void const* user_data)
        {
            strings_t const*        strings  = (strings_t const*)user_data;
            strings_t::obj_t const* find_str = strings->m_str_array.ptr_of(find_item);
            strings_t::obj_t const* node_str = strings->m_str_array.ptr_of(node_item);
            return strings->compare_str(find_str, node_str);
        }

        string_t find_or_insert(utf8::pcrune str, u32 byte_len);

        string_t strings_t::find_or_insert(utf8::pcrune _str, u32 _byte_len)
        {
            utf8::pcrune      str8     = _str;
            utf8::pcrune      src8     = str8;
            utf8::pcrune      end8     = _str + _byte_len;
            u8*               data_ptr = m_data_ptr;
            utf8::prune const dst      = (utf8::prune)m_data_buffer.allocate(data_ptr, _byte_len + 1, sizeof(u8), 1024 * 1024);
            utf8::prune       dst8     = dst;
            while (str8 < end8)
                *dst8++ = *src8++;
            dst->value = 0;

            obj_t*    str       = m_str_array.alloc();
            u32 const str_index = m_str_array.idx_of(str);
            str->m_str          = dst;
            str->m_hash         = hash(str8, end8);
            str->m_len          = _byte_len;

            node_t node = m_str_tree.find(m_str_root, str_index, compare_str, this);
            if (node != 0)
            {
                m_str_array.free(str);
                u32 const index = m_str_tree.get_item(node);
                return index;
            }

            m_data_ptr = data_ptr;

            m_str_tree.insert(m_str_root, str_index, compare_str, this);
            return str_index;
        }

        string_t strings_t::find_or_insert(utf16::pcrune str, u32 byte_len)
        {
            // TODO implement
            // Allocate utf8 string data and convert the utf16 string to utf8
            return 0;
        }

        string_t strings_t::find_or_insert(utf32::pcrune str, u32 byte_len)
        {
            // TODO implement
            // Allocate utf8 string data and convert the utf16 string to utf8
            return 0;
        }

        string_t strings_t::find_or_insert(ucs2::pcrune str, u32 byte_len)
        {
            // TODO implement
            // Allocate utf8 string data and convert the ucs2 string to utf8
            return 0;
        }

        u32 strings_t::get_len(string_t index) const { return m_str_array.ptr_of(index)->m_len; }

        void strings_t::view_string(string_t _str, utf8::pcrune& out_str, u32& out_len) const
        {
            obj_t* str = m_str_array.ptr_of(_str);
            out_str    = str->m_str;
            out_len    = str->m_len;
        }

        s8 strings_t::compare(string_t left, string_t right) const
        {
            obj_t const* left_str  = m_str_array.ptr_of(left);
            obj_t const* right_str = m_str_array.ptr_of(right);
            return compare_str(left_str, right_str);
        }

    } // namespace npath
} // namespace ncore
