#ifndef __C_PATH_VIRTUAL_FREELIST_H__
#define __C_PATH_VIRTUAL_FREELIST_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cpath/private/c_virtual_buffer.h"

namespace ncore
{
    class alloc_t;

    void*        freelist_alloc(u8* memory, u32 _item_size, u32& _free_head, u32& _free_index);
    void         freelist_free(void* item, u8* memory, u32 _item_size, u32& _free_head);
    inline bool  freelist_isfull(u32 _free_head, u32 _free_index, u32 _max_index) { return _free_head == 0 && _free_index >= _max_index; }
    void         freelist_reset(u32& _free_head, u32& _free_index);
    inline u32   freelist_idx_of(void const* item, u8* memory, u32 _item_size) { return (u32)((u8*)item - memory) / _item_size; }
    inline void* freelist_ptr_of(u32 index, u8* memory, u32 _item_size) { return memory + index * _item_size; }

    template <typename T> class freelist_t
    {
        u32              m_free_head;  // Head of the free list of str_t
        u32              m_free_index; // Index of the free list of str_t
        virtual_buffer_t m_memory;     // Virtual memory

    public:
        inline void init(alloc_t* allocator, u32 initial_items = 8192, u32 max_items = 1024 * 1024)
        {
            // Note: Index 0 is used as a null index
            m_free_head  = 0;
            m_free_index = 1;
            m_memory.init(initial_items, max_items, sizeof(T));
        }
        inline void exit(alloc_t* allocator)
        {
            m_memory.exit();
            m_free_head  = 0;
            m_free_index = 1;
        }
        inline T* alloc()
        {
            if (freelist_isfull(m_free_head, m_free_index, m_memory.m_committed))
                m_memory.add_capacity(8192, sizeof(T));
            return (T*)freelist_alloc(m_memory.m_ptr, sizeof(T), m_free_head, m_free_index);
        }
        inline void free(T* item) { freelist_free(item, m_memory.m_ptr, sizeof(T), m_free_head); }
        inline void reset(u32 initial_items = 8192, u32 max_items = 1024 * 1024)
        {
            m_memory.reset(initial_items, max_items, sizeof(T));
            freelist_reset(m_free_head, m_free_index);
        }
        inline u32 idx_of(T const* item) const { return freelist_idx_of(item, m_memory.m_ptr, sizeof(T)); }
        inline T*  ptr_of(u32 index) const { return (T*)freelist_ptr_of(index, m_memory.m_ptr, sizeof(T)); }
    };

}; // namespace ncore

#endif
