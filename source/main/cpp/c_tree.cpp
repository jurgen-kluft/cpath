#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/private/c_tree.h"

namespace ncore
{
    namespace npath
    {
        void tree_t::init(u32 init_num_items, u32 max_num_items)
        {
            m_item_array.init(init_num_items, max_num_items + 1, sizeof(u32));
            m_node_array.init(init_num_items, max_num_items + 1, sizeof(ntree32::tree_t::nnode_t));
            m_color_array.init(init_num_items, (max_num_items + 1 + 31) >> 5, sizeof(u32));
            m_free_head  = 0;
            m_free_index = 0;

            ntree32::setup_tree(m_tree, max_num_items, m_node_array.m_ptr, m_color_array.m_ptr);
        }

        void tree_t::exit()
        {
            m_item_array.exit();
            m_node_array.exit();
            m_color_array.exit();
            m_free_head  = 0;
            m_free_index = 1;
        }

        void tree_t::reset()
        {
            m_free_head  = 0;
            m_free_index = 1;
        }

        node_t tree_t::find(node_t root, u32 const find, item_cmp cmp, void const* user_data) { return 0; }

        bool tree_t::insert(node_t& _root, u32 const _insert, item_cmp cmp, void const* user_data) { return false; }

        bool tree_t::remove(node_t& _root, u32 _remove, item_cmp cmp, void const* user_data) { return false; }

        u32 tree_t::get_item(node_t node) const { return 0; }

        // node_t tree_t::v_new_node(u32 _item)
        // {
        //     u32 index;
        //     if (m_free_head > 0)
        //     {
        //         index       = m_free_head;
        //         m_free_head = *((u32*)m_item_array.m_ptr + m_free_head);
        //     }
        //     else
        //     {
        //         if (m_free_index >= m_item_array.m_committed)
        //         {
        //             const u32 capacity_increase = 16384;
        //             m_item_array.add_capacity(capacity_increase, sizeof(u32));
        //             m_node_left_array.add_capacity(capacity_increase, sizeof(u32));
        //             m_node_right_array.add_capacity(capacity_increase, sizeof(u32));
        //             m_color_array.add_capacity(capacity_increase >> 5, sizeof(u32));
        //         }
        //         index = m_free_index++;
        //     }

        //     // initialize this new node
        //     v_set_red(index);
        //     v_set_left(index, 0);
        //     v_set_right(index, 0);
        //     v_set_item(index, _item);
        //     return index;
        // }

        // void tree_t::v_del_node(node_t node)
        // {
        //     *((u32*)m_item_array.m_ptr + node) = m_free_head;
        //     m_free_head                        = node;
        // }

    } // namespace npath
} // namespace ncore
