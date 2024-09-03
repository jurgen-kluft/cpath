#include "cpath/private/c_virtual_buffer.h"
#include "cvmem/c_virtual_memory.h"

namespace ncore
{
    void virtual_buffer_t::init(u32 initial_capacity, u32 max_capacity, u32 item_size)
    {
        u32 const page_size = nvmem::page_size();

        m_ptr                    = nullptr;
        u32 const reserved_size  = ((max_capacity * item_size + page_size - 1) / page_size) * page_size;
        u32 const committed_size = ((initial_capacity * item_size + page_size - 1) / page_size) * page_size;

        // u64 address_range, u32& page_size, u32 reserve_flags, void*& baseptr
        void* baseptr = nullptr;
        nvmem::reserve(reserved_size, nvmem::ReadWrite, baseptr);
        m_ptr = (u8*)baseptr;

        if (m_ptr && committed_size > 0)
        {
            nvmem::commit(m_ptr, committed_size);
            m_committed = committed_size / item_size;
        }
        m_reserved = reserved_size / item_size;
    }

    void virtual_buffer_t::exit()
    {
        nvmem::release(m_ptr, m_reserved);
        m_ptr       = nullptr;
        m_reserved  = 0;
        m_committed = 0;
    }

    void virtual_buffer_t::reset(u32 initial_capacity, u32 item_size)
    {
        // Figure out how much memory we need to uncommit
        u32 const page_size      = nvmem::page_size();
        u32 const committed_size = m_committed * item_size;
        u32 const initial_size   = ((initial_capacity * item_size + page_size - 1) / page_size) * page_size;
        u32 const uncommit_size  = committed_size - initial_size;

        if (uncommit_size > 0)
        {
            nvmem::decommit(m_ptr + initial_size, uncommit_size);
            m_committed = initial_capacity;
        }
    }

    void virtual_buffer_t::add_capacity(u32 capacity, u32 item_size)
    {
        // commit size = capacity * item size
        u32 const page_size   = nvmem::page_size();
        u32 const commit_size = ((capacity * item_size + page_size - 1) / page_size) * page_size;

        if (commit_size > 0)
        {
            nvmem::commit(m_ptr + m_committed * item_size, commit_size);
            m_committed += commit_size / item_size;
        }
    }

    u8* virtual_buffer_t::allocate(u8*& ptr, u32 items, u32 item_size, u32 add_capacity_when_needed)
    {
        if ((ptr + items*item_size) > (m_ptr + m_committed*item_size))
            add_capacity(add_capacity_when_needed, item_size);

        u8* const result = ptr;
        ptr += items * item_size;
        return result;
    }

}; // namespace ncore
