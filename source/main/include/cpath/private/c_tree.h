#ifndef __C_PATH_BST_TREE_H__
#define __C_PATH_BST_TREE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"

#include "cpath/private/c_virtual_buffer.h"

namespace ncore
{
    namespace npath
    {
        // Note: Index 0 is used as a null index
        typedef u32 inode_t;
        struct tree_t
        {
            virtual_buffer_t m_item_array;       // Virtual memory array of u32[]
            virtual_buffer_t m_node_left_array;  // Virtual memory array of u32[]
            virtual_buffer_t m_node_right_array; // Virtual memory array of u32[]
            virtual_buffer_t m_color_array;      // Red-black tree color array
            u32              m_free_head;        // Head of the free list
            u32              m_free_index;       // Index of the free list

            void init(alloc_t* allocator, u32 init_num_items = 8192, u32 max_num_items = 1024 * 1024);
            void exit(alloc_t* allocator);

            bool find(inode_t root, u32 const find, s8 (*cmp)(u32 const find_item, u32 const node_item, void const* user_data), void const* user_data);
            bool insert(inode_t& root, u32 const insert, s8 (*cmp)(u32 const find_item, u32 const node_item, void const* user_data), void const* user_data);
            bool remove(inode_t& root, u32 const remove, s8 (*cmp)(u32 const find_item, u32 const node_item, void const* user_data), void const* user_data);

            // item can be one of ifolder_t or istring_t
            inline void    set_item(inode_t index, u32 item) const { *((u32*)m_item_array.m_ptr + index) = item; }
            inline u32     get_item(inode_t index) const { return *((u32 const*)m_item_array.m_ptr + index); }
            inline bool    is_red(inode_t index) const { return (m_color_array.m_ptr[index >> 3] & (1 << (index & 0x07))) != 0; }
            inline bool    is_black(inode_t index) const { return (m_color_array.m_ptr[index >> 3] & (1 << (index & 0x07))) == 0; }
            inline void    set_red(inode_t index) { m_color_array.m_ptr[index >> 3] |= (1 << (index & 0x07)); }
            inline void    set_black(inode_t index) { m_color_array.m_ptr[index >> 3] &= ~(1 << (index & 0x07)); }
            inline inode_t get_left(inode_t index) const { return *((u32 const*)m_node_left_array.m_ptr + index); }
            inline inode_t get_right(inode_t index) const { return *((u32 const*)m_node_right_array.m_ptr + index); }
            inline void    set_left(inode_t index, inode_t child_index) { *((u32*)m_node_left_array.m_ptr + index) = child_index; }
            inline void    set_right(inode_t index, inode_t child_index) { *((u32*)m_node_right_array.m_ptr + index) = child_index; }
            inode_t        get_child(inode_t index, s8 child) const;
            void           set_child(inode_t index, s8 child, inode_t child_index);

            inode_t alloc();
            void    free(inode_t node);
            void    reset();
        };
    } // namespace npath
} // namespace ncore

#endif
