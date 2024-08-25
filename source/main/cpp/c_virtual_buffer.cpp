#include "cpath/private/c_virtual_buffer.h"
#include "cvmem/c_virtual_memory.h"

namespace ncore
{
    void virtual_buffer_t::init(u32 initial_capacity, u32 max_capacity, u32 item_size)
    {
        u32 const page_size = vmem_t::page_size();

        m_ptr       = nullptr;
        u32 const reserved_size  = ((max_capacity * item_size + page_size - 1) / page_size) * page_size;
        u32 const committed_size = ((initial_capacity * item_size + page_size - 1) / page_size) * page_size;

        // u64 address_range, u32& page_size, u32 reserve_flags, void*& baseptr
        void* baseptr = nullptr;
        vmem_t::reserve(reserved_size, {vmem_protect_t::ReadWrite}, baseptr);
        m_ptr = (u8*)baseptr;

        if (m_ptr && committed_size > 0)
        {
            vmem_t::commit(m_ptr, committed_size);
            m_committed = committed_size / item_size;
        }
        m_reserved = reserved_size / item_size;
    }

    void virtual_buffer_t::exit()
    {
        vmem_t::release(m_ptr, m_reserved);
        m_ptr       = nullptr;
        m_reserved  = 0;
        m_committed = 0;
    }

    void virtual_buffer_t::add_capacity(u32 capacity, u32 item_size)
    {
        // TODO
    }

}; // namespace ncore
