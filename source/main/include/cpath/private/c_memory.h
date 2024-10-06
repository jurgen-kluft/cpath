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

            void ensure_capacity(u32 index, u32 item_size = 1, u32 add_capacity_when_needed = 64 * 1024);

            // Allocate = Reserve & Commit
            u8* allocate(u8*& ptr, u32 items, u32 item_size = 1);

            // Reserve -> Commit
            u8* reserve(u8* ptr, u32 items, u32 item_size = 1);
            u8* commit(u8*& ptr, u32 items, u32 item_size = 1);

            // Index to pointer and pointer to index
            inline u32 idx_of(u8 const* item, u32 item_size = 1) const { return (u32)(item - m_ptr) / item_size; }
            inline u8* ptr_of(u32 index, u32 item_size = 1) const { return m_ptr + index * item_size; }
        };

        template <typename T> struct objects_t
        {
            memory_t m_memory;

            void init(u32 initial_capacity, u32 max_capacity) { m_memory.init(initial_capacity, max_capacity, sizeof(T)); }
            void exit() { m_memory.exit(); }
            void reset(u32 initial_capacity) { m_memory.reset(initial_capacity, sizeof(T)); }
            void add_capacity(u32 add_capacity) { m_memory.add_capacity(add_capacity, sizeof(T)); }

            void ensure_capacity(u32 capacity, u32 add_capacity_when_needed = 1024 * 1024) { m_memory.ensure_capacity(capacity, add_capacity_when_needed); }

            T* allocate(u32 items) { return (T*)m_memory.allocate((u8*&)m_memory.m_ptr, items, sizeof(T)); }
            T* reserve(u32 items) { return (T*)m_memory.reserve(m_memory.m_ptr, items, sizeof(T)); }
            T* commit(u32 items) { return (T*)m_memory.commit((u8*&)m_memory.m_ptr, items, sizeof(T)); }

            inline u32 idx_of(T const* item) const { return (u32)(item - (T*)m_memory.m_ptr); }
            inline T*  ptr_of(u32 index) const { return (T*)m_memory.m_ptr + index; }
        };

    } // namespace npath
} // namespace ncore

#endif
