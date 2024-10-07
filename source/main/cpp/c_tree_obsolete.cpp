#include "cbase/c_allocator.h"
#include "cbase/c_binary_search.h"
#include "cbase/c_buffer.h"
#include "ccore/c_debug.h"
#include "cbase/c_hash.h"
#include "cbase/c_runes.h"
#include "ccore/c_target.h"

#include "cpath/private/c_tree_obsolete.h"

namespace ncore
{
    namespace npath
    {
        const u32 c_capacity_bias = 4;

        void tree_t::init(u32 init_num_items, u32 max_num_items)
        {
            const s32 c_extra_size = 2; // For 'find' and 'index_of_temporary_node' slots

            m_active_nodes = 0;
            m_free_nodes   = init_num_items;
            m_node_array.init(init_num_items, max_num_items + c_extra_size, sizeof(ntree32::nnode_t));
            ntree32::setup_tree(m_tree, (ntree32::nnode_t*)m_node_array.m_ptr);
        }

        void tree_t::exit()
        {
            m_node_array.exit();
            ntree32::teardown_tree(m_tree);
        }

        void tree_t::reset()
        {
            m_tree.m_free_head  = ntree32::c_invalid_node;
            m_tree.m_free_index = 0;
            m_active_nodes      = 0;
            m_free_nodes        = m_node_array.m_committed;
        }

        node_t tree_t::find(node_t root, u32 const _index_of_item_to_find, item_cmp cmp, void const* user_data) const
        {
            ntree32::node_t found;
            if (ntree32::find(m_tree, root, _index_of_item_to_find, cmp, user_data, found))
                return found;
            return ntree32::c_invalid_node;
        }

        bool tree_t::insert(node_t& _root, u32 const _index_of_item_to_insert, item_cmp cmp, void const* user_data, node_t& inserted)
        {
            // Check if we have enough free nodes
            if (m_tree.m_free_head == ntree32::c_invalid_node)
            {
                const u32 capacity_increase = 16384;
                m_node_array.ensure_capacity(m_tree.m_free_index, capacity_increase, sizeof(ntree32::nnode_t));
            }

            return ntree32::insert(m_tree, _root, temp_slot(), _index_of_item_to_insert, cmp, user_data, inserted);
        }

        bool tree_t::remove(node_t& _root, u32 _index_of_item_to_remove, item_cmp cmp, void const* user_data, node_t& removed)
        {
            if (ntree32::remove(m_tree, _root, temp_slot(), _index_of_item_to_remove, cmp, user_data, removed))
            {
                m_tree.del_node(removed);
                return true;
            }
            return false;
        }

    } // namespace npath
} // namespace ncore
