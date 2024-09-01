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

        void strings_t::init(alloc_t* allocator, u32 max_items)
        {
            m_data_buffer.init(1024 * 1024, max_items * 32, sizeof(u8));
            m_data_ptr = m_data_buffer.m_ptr;
            m_str_array.init(allocator, max_items);
            m_str_tree.init(allocator, max_items);

            m_str_root = 0;
        }

        void strings_t::exit(alloc_t* allocator)
        {
            m_str_tree.exit(allocator);
            m_str_array.exit(allocator);
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
                hash = hash + r.value * 31;
            }
            return hash;
        }

        s8 strings_t::compare_str(u32 const find_item, u32 const node_item, void const* user_data)
        {
            strings_t const* strings  = (strings_t const*)user_data;
            strings_t::obj_t const*  find_str = strings->m_str_array.ptr_of(find_item);
            strings_t::obj_t const*  node_str = strings->m_str_array.ptr_of(node_item);
            return strings->compare(find_str, node_str);
        }

        string_t findOrInsert(utf8::pcrune str, u32 byte_len);

        string_t strings_t::findOrInsert(utf8::pcrune _str, u32 _byte_len)
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

            obj_t* str       = m_str_array.alloc();
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

        string_t strings_t::findOrInsert(utf16::pcrune str, u32 byte_len)
        {
            // TODO implement
            // Allocate utf8 string data and convert the utf16 string to utf8
            return 0;
        }
        string_t strings_t::findOrInsert(utf32::pcrune str, u32 byte_len)
        {
            // TODO implement
            // Allocate utf8 string data and convert the utf16 string to utf8
            return 0;
        }
        string_t strings_t::findOrInsert(ucs2::pcrune str, u32 byte_len)
        {
            // TODO implement
            // Allocate utf8 string data and convert the ucs2 string to utf8
            return 0;
        }

        u32 strings_t::get_len(string_t index) const { return m_str_array.ptr_of(index)->m_len; }

        void strings_t::to_string(string_t _str, crunes_t& out_str) const
        {
            obj_t* str = m_str_array.ptr_of(_str);
            utf8::pcrune str8    = str->m_str;
            utf8::pcrune end8    = str->m_str + str->m_len;
            out_str.m_utf8 = str8;
            out_str.m_eos = str->m_len;
            out_str.m_end = str->m_len;
            out_str.m_str = 0;
        }

        s8 strings_t::compare(string_t left, string_t right) const
        {
            obj_t const* left_str  = m_str_array.ptr_of(left);
            obj_t const* right_str = m_str_array.ptr_of(right);
            return compare(left_str, right_str);
        }

    } // namespace npath
} // namespace ncore
