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
        struct varena_t
        {
            u8* m_ptr;       // Virtual memory
            u64 m_reserved;  // Maximum reserved item count (unit=item size), memory size = m_reserved * item size
            u64 m_committed; // Current committed item count (unit=item size), memory size = m_committed * item size

            // Note: You need to stick to a particular item size when using this interface
            void reset(u64 initial_capacity, u32 item_size = 1);
            void add_capacity(u64 add_capacity, u32 item_size = 1);

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

        void g_init_arena(varena_t& m);
        void g_init_arena(varena_t& m, u64 initial_capacity, u64 max_capacity, u32 item_size = 1);
        void g_teardown_arena(varena_t& m);

        template <typename T> struct vpool_t
        {
            varena_t m_arena;

            void reset(u64 initial_capacity) { m_arena.reset(initial_capacity, sizeof(T)); }
            void add_capacity(u64 add_capacity) { m_arena.add_capacity(add_capacity, sizeof(T)); }

            inline T* ptr() const { return (T*)m_arena.m_ptr; }
            void      ensure_capacity(u64 capacity, u32 add_capacity_when_needed = 64 * 1024) { m_arena.ensure_capacity(capacity, add_capacity_when_needed); }

            T* allocate(u32 items) { return (T*)m_arena.allocate((u8*&)m_arena.m_ptr, items, sizeof(T)); }
            T* reserve(u32 items) { return (T*)m_arena.reserve(m_arena.m_ptr, items, sizeof(T)); }
            T* commit(u32 items) { return (T*)m_arena.commit((u8*&)m_arena.m_ptr, items, sizeof(T)); }

            inline u32 idx_of(T const* item) const { return (u32)(item - (T*)m_arena.m_ptr); }
            inline T*  ptr_of(u32 index) const { return (T*)m_arena.m_ptr + index; }
        };

        template<typename T>
        inline void g_setup_vpool(vpool_t<T>& o) { g_init_arena(o.m); }

        template<typename T>
        inline void g_setup_vpool(vpool_t<T>& o, u64 initial_capacity, u64 max_capacity) { g_init_arena(o.m_arena, initial_capacity, max_capacity, sizeof(T)); }

        template<typename T>
        inline void g_teardown_vpool(vpool_t<T>& o) { g_teardown_arena(o.m_arena); }

    } // namespace npath
} // namespace ncore

#endif
