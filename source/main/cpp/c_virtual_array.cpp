#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/private/c_virtual_array.h"

namespace ncore
{
    void virtual_buffer_t::init(u32 initial_capacity, u32 max_capacity, u32 item_size)
    {
        m_ptr = nullptr;
        m_reserved = 0;
        m_committed = 0;
        // TODO
    }

    void virtual_buffer_t::exit()
    {
        // TODO
    }

    void virtual_buffer_t::add_capacity(u32 capacity, u32 item_size)
    {
        // TODO
    }

    void virtual_freelist_base_t::init(alloc_t* allocator, u32 size_of_item, u32 max_items)
    {
        m_free_head  = 0;
        m_free_index = 0;
        m_item_size  = size_of_item;
        m_memory.init(1024, max_items, size_of_item);
    }

    void virtual_freelist_base_t::release(alloc_t* allocator)
    {
        m_memory.exit();
        m_item_size  = 0;
        m_free_head  = 0;
        m_free_index = 0;
    }

    void* virtual_freelist_base_t::alloc()
    {
        if (m_free_head > 0)
        {
            u32 const index = m_free_head;
            m_free_head     = *((u32*)(m_memory.m_ptr + index * m_item_size));
            return m_memory.m_ptr + index * m_item_size;
        }
        else if (m_free_index < m_memory.m_committed)
        {
            u32 const index = m_free_index++;
            return &m_memory.m_ptr[index];
        }

        // TODO Grow the (virtual) array and adjust m_max_count
        m_memory.add_capacity(8192, m_item_size);

        u32 const index = m_free_index++;
        return (m_memory.m_ptr + index * m_item_size);
    }

    void virtual_freelist_base_t::free(void* item)
    {
        u32 const index = idx_of(item);
        *((u32*)item)   = m_free_head;
        m_free_head     = index;
    }

    void virtual_freelist_base_t::reset()
    {
        m_free_head  = 0xffffffff;
        m_free_index = 1;
    }

}; // namespace ncore
