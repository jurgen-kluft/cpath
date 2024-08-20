#ifndef __C_PATH_VIRTUAL_ARRAY_H__
#define __C_PATH_VIRTUAL_ARRAY_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"
#include "cbase/c_runes.h"

namespace ncore
{
    class alloc_t;

    struct virtual_buffer_t
    {
        u8*  m_ptr;       // Virtual memory
        u32  m_reserved;  // Maximum reserved item count (unit=item size), memory size = m_reserved * item size
        u32  m_committed; // Current committed item count (unit=item size), memory size = m_committed * item size
        void init(u32 initial_capacity, u32 max_capacity, u32 item_size = 1);
        void exit();
        void add_capacity(u32 add_capacity, u32 item_size = 1);
    };

    // Note: Index 0 is used as a null index
    class virtual_freelist_base_t
    {
        virtual_buffer_t m_memory;     // Virtual memory
        u32              m_item_size;  // Size of item
        u32              m_free_head;  // Head of the free list of str_t
        u32              m_free_index; // Index of the free list of str_t

    public:
        void  init(alloc_t* allocator, u32 size_of_item, u32 max_items = 1024 * 1024);
        void  release(alloc_t* allocator);
        void* alloc();
        void  free(void* item);
        void  reset();

        inline u32   idx_of(void const* item) const { return (u32)((u8*)item - m_memory.m_ptr) / m_item_size; }
        inline void* ptr_of(u32 index) const { return m_memory.m_ptr + index * m_item_size; }
    };

    template <typename T> class virtual_freelist_t
    {
        virtual_freelist_base_t m_base;

    public:
        inline void init(alloc_t* allocator, u32 max_items = 1024 * 1024) { m_base.init(allocator, sizeof(T), max_items); }
        inline void release(alloc_t* allocator) { m_base.release(allocator); }
        inline T*   alloc() { return (T*)m_base.alloc(); }
        inline void free(T* item) { m_base.free(item); }
        inline void reset() { m_base.reset(); }
        inline u32  idx_of(T const* item) const { return m_base.idx_of(item); }
        inline T*   ptr_of(u32 index) const { return (T*)m_base.ptr_of(index); }
    };

}; // namespace ncore

#endif
