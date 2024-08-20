#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/c_dirpath.h"
#include "cpath/c_filepath.h"
#include "cpath/private/c_rbtree.h"

namespace ncore
{
    void rbtree_t::init(alloc_t* allocator, u32 init_num_items, u32 max_num_items)
    {
        m_item_array.init(init_num_items, max_num_items, sizeof(u32));
        m_node_left_array.init(init_num_items, max_num_items, sizeof(u32));
        m_node_right_array.init(init_num_items, max_num_items, sizeof(u32));
        m_color_array.init(init_num_items, max_num_items >> 5, sizeof(u32));
        m_free_head  = 0;
        m_free_index = 1;
    }

    void rbtree_t::exit(alloc_t* allocator)
    {
        m_item_array.exit();
        m_node_left_array.exit();
        m_node_right_array.exit();
        m_color_array.exit();
        m_free_head  = 0;
        m_free_index = 1;
    }

    bool rbtree_t::find(rbnode_t root, u32 const find, s8 (*cmp)(u32 const find_item, u32 const node_item, void const* user_data), void const* user_data)
    {
        rbnode_t node = root;
        while (node != 0)
        {
            u32 const item = get_item(node);
            s8 const c = cmp(find, item, user_data);
            if (c == 0)
                return true;
            node = get_child(node, (c + 1) >> 1);
        }
        return false;
    }

    rbnode_t rbtree_t::get_child(rbnode_t node, s8 child) const
    {
        u32 const* child_ptr = child == 0 ? (u32 const*)m_node_left_array.m_ptr : (u32 const*)m_node_right_array.m_ptr;
        return *(child_ptr + node);
    }

    void rbtree_t::set_child(rbnode_t node, s8 child, rbnode_t child_index)
    {
        u32* child_ptr      = child == 0 ? (u32*)m_node_left_array.m_ptr : (u32*)m_node_right_array.m_ptr;
        *(child_ptr + node) = child_index;
    }

    rbnode_t rbtree_t::alloc()
    {
        if (m_free_head != 0)
        {
            const u32 index = m_free_head;
            m_free_head     = *((u32*)m_item_array.m_ptr + m_free_head);
            return index;
        }
        if (m_free_index >= m_item_array.m_committed)
        {
            const u32 capacity_increase = 8192;
            m_item_array.add_capacity(capacity_increase, sizeof(u32));
            m_node_left_array.add_capacity(capacity_increase, sizeof(u32));
            m_node_right_array.add_capacity(capacity_increase, sizeof(u32));
            m_color_array.add_capacity(capacity_increase >> 5, sizeof(u32));
        }
        const u32 index = m_free_index++;

        // initialize this new node
        set_red(index);
        set_left(index, 0);
        set_right(index, 0);
        set_item(index, 0);

        return index;
    }

    void rbtree_t::free(rbnode_t node)
    {
        *((u32*)m_item_array.m_ptr + (node * sizeof(u32))) = m_free_head;
        m_free_head                                        = node;
    }

    void rbtree_t::reset()
    {
        m_free_head  = 0;
        m_free_index = 1;
    }

}; // namespace ncore
