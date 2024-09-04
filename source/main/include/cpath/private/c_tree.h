#ifndef __C_PATH_BST_TREE_H__
#define __C_PATH_BST_TREE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"

#include "cpath/private/c_memory.h"

namespace ncore
{
    namespace npath
    {
        // Note: Index 0 is used as a null index
        typedef u32 node_t;

        struct tree_t
        {
            void init(u32 init_num_items = 16384, u32 max_num_items = 16 * 1024 * 1024);
            void exit();
            void reset();

            typedef s8 (*item_cmp)(u32 const find_item, u32 const node_item, void const* user_data);

            node_t find(node_t root, u32 const find, item_cmp cmp, void const* user_data);
            bool   insert(node_t& root, u32 const insert, item_cmp cmp, void const* user_data);
            bool   remove(node_t& root, u32 const remove, item_cmp cmp, void const* user_data);

            u32 get_item(node_t node) const;

        protected:
            memory_t m_item_array;       // Virtual memory array of u32[]
            memory_t m_node_left_array;  // Virtual memory array of u32[]
            memory_t m_node_right_array; // Virtual memory array of u32[]
            memory_t m_color_array;      // Red-black tree color array
            u32      m_free_head;        // Head of the free list
            u32      m_free_index;       // Index of the free list

            node_t rotate_single(node_t node, s32 dir);
            node_t rotate_double(node_t node, s32 dir);
            node_t rotate_single_track_parent(node_t node, s32 dir, node_t fn, node_t& fp);
            node_t rotate_double_track_parent(node_t node, s32 dir, node_t fn, node_t& fp);

            bool   v_clear(node_t& _root, node_t& removed_node);
            bool   v_clear(node_t& _root);
            s32    v_validate(node_t root, const char*& result, item_cmp cmp, void const* user_data) const;
            u32    v_get_item(node_t node) const;
            void   v_set_item(node_t node, u32 item);
            node_t v_get_child(node_t node, s8 ne) const;
            void   v_set_child(node_t node, s8 ne, node_t set);
            node_t v_get_left(node_t node) const;
            void   v_set_left(node_t node, node_t child);
            node_t v_get_right(node_t node) const;
            void   v_set_right(node_t node, node_t child);
            void   v_set_color(node_t node, s8 color);
            s8     v_get_color(node_t node) const;
            bool   v_is_red(node_t node) const;
            bool   v_is_black(node_t node) const;
            void   v_set_black(node_t node);
            void   v_set_red(node_t node);
            node_t v_get_temp();
            node_t v_new_node(u32 _item);
            void   v_del_node(node_t node);
        };
    } // namespace npath
} // namespace ncore

#endif
