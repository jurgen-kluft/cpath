#include "cpath/private/c_memory.h"
#include "cvmem/c_virtual_memory.h"

namespace ncore
{
    namespace npath
    {
        const u32 c_capacity_bias = 4;

        void g_init_memory(memory_t& m)
        {
            m.m_ptr = nullptr;
            m.m_reserved = 0;
            m.m_committed = 0;
        }

        void g_init_memory(memory_t& m, u64 initial_capacity, u64 max_capacity, u32 item_size)
        {
            u32 const page_size = nvmem::page_size();

            m.m_ptr                    = nullptr;
            u64 const reserved_size  = ((max_capacity * item_size + page_size - 1) / page_size) * page_size;
            u64 const committed_size = ((initial_capacity * item_size + page_size - 1) / page_size) * page_size;

            // u64 address_range, u32& page_size, u32 reserve_flags, void*& baseptr
            void* baseptr = nullptr;
            nvmem::reserve(reserved_size, nvmem::ReadWrite, baseptr);
            m.m_ptr = (u8*)baseptr;

            if (m.m_ptr && committed_size > 0)
            {
                nvmem::commit(m.m_ptr, committed_size);
                m.m_committed = committed_size / item_size;
            }
            m.m_reserved = reserved_size / item_size;
        }

        void g_exit_memory(memory_t& m)
        {
            nvmem::release(m.m_ptr, m.m_reserved);
            m.m_ptr       = nullptr;
            m.m_reserved  = 0;
            m.m_committed = 0;
        }

        void memory_t::reset(u64 initial_capacity, u32 item_size)
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

        void memory_t::add_capacity(u64 capacity, u32 item_size)
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

        void memory_t::ensure_capacity(u32 index, u32 item_size, u32 add_capacity_when_needed)
        {
            if ((index + c_capacity_bias) >= m_committed)
                add_capacity((index + c_capacity_bias) - m_committed, item_size);
        }

        u8* memory_t::allocate(u8*& ptr, u32 items, u32 item_size)
        {
            reserve(ptr, items, item_size);
            return commit(ptr, items, item_size);
        }

        u8* memory_t::reserve(u8* ptr, u32 items, u32 item_size)
        {
            if ((ptr + items * item_size) > (m_ptr + m_committed * item_size))
                add_capacity(items + c_capacity_bias, item_size);
            return ptr;
        }

        u8* memory_t::commit(u8*& ptr, u32 items, u32 item_size)
        {
            u8* p = ptr;
            ptr += (items * item_size);
            return p;
        }

    } // namespace npath
} // namespace ncore
