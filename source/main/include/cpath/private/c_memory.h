#ifndef __C_PATH_MEMORY_H__
#define __C_PATH_MEMORY_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    namespace npath
    {
        struct memory_t
        {
            u8* m_ptr;       // Virtual memory
            u32 m_reserved;  // Maximum reserved item count (unit=item size), memory size = m_reserved * item size
            u32 m_committed; // Current committed item count (unit=item size), memory size = m_committed * item size

            // Note: You need to stick to a particular item size when using this interface

            void init(u32 initial_capacity, u32 max_capacity, u32 item_size = 1);
            void exit();
            void reset(u32 initial_capacity, u32 item_size = 1);
            void add_capacity(u32 add_capacity, u32 item_size = 1);

            // Allocate = Reserve & Commit
            u8* allocate(u8*& ptr, u32 items, u32 item_size = 1, u32 add_capacity_when_needed = 1024 * 1024);

            // Reserve -> Commit
            u8* reserve(u8* ptr, u32 items, u32 item_size = 1, u32 add_capacity_when_needed = 1024 * 1024);
            u8* commit(u8*& ptr, u32 items, u32 item_size = 1);
        };
    } // namespace npath
} // namespace ncore

#endif
