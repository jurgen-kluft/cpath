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
        strings_t::strings_t()
            : m_text_data(nullptr)
            , m_text_data_size(0)
            , m_text_data_cap(0)
            , m_str_array()
            , m_str_tree()
            , m_str_root(0)
        {
		}

        void strings_t::init(alloc_t* allocator, u32 cap)
        {
            m_text_data      = allocator->allocate(cap);
            m_text_data_size = 0;
            m_text_data_cap  = cap;

            m_str_array.init(allocator, cap);
            m_str_tree.init(allocator, cap);

            m_str_root = 0;
        }

        void strings_t::exit(alloc_t* allocator)
        {
            m_str_tree.exit(allocator);
            m_str_array.exit(allocator);
            allocator->deallocate(m_text_data);
        }

        string_t* strings_t::attach(string_t* node) { return node; }
        string_t* strings_t::detach(string_t* node) { return nullptr; }

        static u32 hash(utf8::pcrune str, utf8::pcrune end)
        {
            u32 hash = 0;
            while (str < end)
            {
                hash = hash + *str++ * 31;
            }
            return hash;
        }

        static s8 compare_str(u32 const find_item, u32 const node_item, void const* user_data)
        {
            strings_t const* strings  = (strings_t const*)user_data;
            string_t const*  find_str = strings->m_str_array.ptr_of(find_item);
            string_t const*  node_str = strings->m_str_array.ptr_of(node_item);
            return strings->compare(find_str, node_str);
        }

        istring_t strings_t::findOrInsert(crunes_t const& _str)
        {
            utf8::pcrune str8 = _str.m_utf8.m_bos;
            utf8::pcrune src8 = str8;
            utf8::pcrune end8 = _str.m_utf8.m_bos + _str.m_utf8.m_eos;
            utf8::prune dst  = (utf8::prune)m_text_data + m_text_data_size;
            while (str8 < end8)
                *dst++ = *src8++;
            *dst = 0;

            string_t* str       = m_str_array.alloc();
            u32 const str_index = m_str_array.idx_of(str);
            str->m_str          = (utf8::prune)m_text_data + m_text_data_size;
            str->m_hash         = hash(str8, end8);
            str->m_len          = _str.m_utf8.m_eos;

            inode_t node = m_str_tree.find(m_str_root, str_index, compare_str, this);
            if (node != 0)
            {
                m_str_array.free(str);
                u32 const index = m_str_tree.get_item(node);
                return index;
            }

            m_text_data_size += str->m_len + 1;

            m_str_tree.insert(m_str_root, str_index, compare_str, this);
            return str_index;
        }

        bool strings_t::remove(string_t* item) { return true; }

        u32 strings_t::get_len(istring_t index) const { return m_str_array.ptr_of(index)->m_len; }

        void strings_t::to_string(istring_t str, crunes_t& out_str) const
        {
            string_t* str_entry = m_str_array.ptr_of(str);
            to_string(str_entry, out_str);
        }

        void strings_t::to_string(string_t const* str, crunes_t& out_str) const
        {
            utf8::pcrune str8    = str->m_str;
            utf8::pcrune end8    = str->m_str + str->m_len;
            out_str.m_utf8.m_bos = str8;
            out_str.m_utf8.m_eos = str->m_len;
            out_str.m_utf8.m_end = str->m_len;
            out_str.m_utf8.m_str = 0;
        }

        s8 strings_t::compare(istring_t left, istring_t right) const
        {
            string_t const* left_str  = m_str_array.ptr_of(left);
            string_t const* right_str = m_str_array.ptr_of(right);
            return compare(left_str, right_str);
        }

        s8 strings_t::compare(string_t const* left_str, string_t const* right_str) const
        {
            if (left_str->m_hash < right_str->m_hash)
                return -1;
            if (left_str->m_hash > right_str->m_hash)
                return 1;
            return utf8::compare(left_str->m_str, left_str->m_len, right_str->m_str, right_str->m_len);
        }

    } // namespace npath
} // namespace ncore
