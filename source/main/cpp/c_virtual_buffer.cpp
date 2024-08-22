#include "cpath/private/c_virtual_buffer.h"

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

}; // namespace ncore
