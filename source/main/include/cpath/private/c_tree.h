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

        namespace ntree2
        {
            struct node_t;
        }

        enum echild_t
        {
            LEFT  = 0,
            RIGHT = 1,
        };

        enum ecolor_t
        {
            RED   = 0,
            BLACK = 1,
        };

        struct tree_t
        {
            void init(alloc_t* allocator, u32 init_num_items = 16384, u32 max_num_items = 16 * 1024 * 1024);
            void exit(alloc_t* allocator);
            void reset();

            typedef s8 (*item_cmp)(u32 const find_item, u32 const node_item, void const* user_data);

            inode_t find(inode_t root, u32 const find, item_cmp cmp, void const* user_data);
            bool    insert(inode_t& root, u32 const insert, item_cmp cmp, void const* user_data);
            bool    remove(inode_t& root, u32 const remove, item_cmp cmp, void const* user_data);

            u32 get_item(inode_t node) const;

        protected:
            virtual_buffer_t m_item_array;       // Virtual memory array of u32[]
            virtual_buffer_t m_node_left_array;  // Virtual memory array of u32[]
            virtual_buffer_t m_node_right_array; // Virtual memory array of u32[]
            virtual_buffer_t m_color_array;      // Red-black tree color array
            u32              m_free_head;        // Head of the free list
            u32              m_free_index;       // Index of the free list

            inode_t rotate_single(inode_t node, s32 dir);
            inode_t rotate_double(inode_t node, s32 dir);
            inode_t rotate_single_track_parent(inode_t node, s32 dir, inode_t fn, inode_t& fp);
            inode_t rotate_double_track_parent(inode_t node, s32 dir, inode_t fn, inode_t& fp);

            bool     v_clear(inode_t& _root, inode_t& removed_node);
            bool     v_clear(inode_t& _root);
            s32      v_validate(inode_t root, const char*& result, item_cmp cmp, void const* user_data) const;
            u32      v_get_item(inode_t node) const;
            void     v_set_item(inode_t node, u32 item);
            inode_t  v_get_child(inode_t node, s8 ne) const;
            void     v_set_child(inode_t node, s8 ne, inode_t set);
            inode_t  v_get_left(inode_t node) const;
            void     v_set_left(inode_t node, inode_t child);
            inode_t  v_get_right(inode_t node) const;
            void     v_set_right(inode_t node, inode_t child);
            void     v_set_color(inode_t node, ecolor_t color);
            ecolor_t v_get_color(inode_t node) const;
            bool     v_is_red(inode_t node) const;
            bool     v_is_black(inode_t node) const;
            void     v_set_black(inode_t node);
            void     v_set_red(inode_t node);
            inode_t  v_get_temp();
            inode_t  v_new_node(u32 _item);
            void     v_del_node(inode_t node);
        };
    } // namespace npath
} // namespace ncore

#endif
