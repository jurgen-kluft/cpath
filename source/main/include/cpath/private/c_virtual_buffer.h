#ifndef __C_PATH_VIRTUAL_BUFFER_H__
#define __C_PATH_VIRTUAL_BUFFER_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    struct virtual_buffer_t
    {
        u8*  m_ptr;       // Virtual memory
        u32  m_reserved;  // Maximum reserved item count (unit=item size), memory size = m_reserved * item size
        u32  m_committed; // Current committed item count (unit=item size), memory size = m_committed * item size
        void init(u32 initial_capacity, u32 max_capacity, u32 item_size = 1);
        void exit();
        void reset(u32 initial_capacity, u32 max_capacity, u32 item_size = 1);
        void add_capacity(u32 add_capacity, u32 item_size = 1);
    };
}; // namespace ncore

#endif
