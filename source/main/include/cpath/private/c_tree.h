#ifndef __C_PATH_TREE_H__
#define __C_PATH_TREE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"
#include "cbase/c_tree32.h"

#include "cpath/private/c_memory.h"

namespace ncore
{
    namespace npath
    {
        typedef u32 node_t;

        struct tree_t
        {
            void init(u32 init_num_items = 16384, u32 max_num_items = 256 * 1024 * 1024);
            void exit();
            void reset();

            typedef s8 (*item_cmp)(u32 find_item, u32 node_item, void const* user_data);

            node_t find(node_t root, u32 const find, item_cmp cmp, void const* user_data) const;
            bool   insert(node_t& root, u32 const insert, item_cmp cmp, void const* user_data);
            bool   remove(node_t& root, u32 const remove, item_cmp cmp, void const* user_data);

            u32 get_item(node_t node) const;

            DCORE_CLASS_PLACEMENT_NEW_DELETE

        protected:
            s64             m_size;
            ntree32::tree_t m_tree;
            memory_t        m_item_array;  // Virtual memory array of u32[]
            memory_t        m_node_array;  // Virtual memory array of nnode_t[]
            memory_t        m_color_array; // Red-black tree color array (bit array)
        };
    } // namespace npath
} // namespace ncore

#endif
