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
        struct search_data_t
        {
            u32 const*       m_items;
            void const*      m_user_data;
            tree_t::item_cmp m_cmp;
        };

        static s8 s_intermediate_compare(u32 find_item, u32 node_item, void const* user_data)
        {
            search_data_t const* data = (search_data_t const*)user_data;
            return data->m_cmp(find_item, data->m_items[node_item], data->m_user_data);
        }

        void tree_t::init(u32 init_num_items, u32 max_num_items)
        {
            m_item_array.init(init_num_items, max_num_items + 1, sizeof(u32));
            m_node_array.init(init_num_items, max_num_items + 1, sizeof(ntree32::tree_t::nnode_t));
            m_color_array.init(init_num_items, (max_num_items + 1 + 31) >> 5, sizeof(u32));
            ntree32::setup_tree(m_tree, max_num_items, m_node_array.m_ptr, m_color_array.m_ptr);
        }

        void tree_t::exit()
        {
            m_item_array.exit();
            m_node_array.exit();
            m_color_array.exit();
            ntree32::teardown_tree(m_tree);
        }

        void tree_t::reset()
        {
            m_tree.m_free_head  = ntree32::c_invalid_node;
            m_tree.m_free_index = 0;
            m_tree.m_count      = 0;
        }

        node_t tree_t::find(node_t root, u32 const find, item_cmp cmp, void const* user_data) const
        {
            search_data_t search_data = {(u32 const*)m_item_array.m_ptr, user_data, cmp};

            ntree32::node_t found;
            if (ntree32::find(m_tree, root, find, s_intermediate_compare, &search_data, found))
                return found;
            return ntree32::c_invalid_node;
        }

        bool tree_t::insert(node_t& _root, u32 const _insert, item_cmp cmp, void const* user_data)
        {
            // Check if we have enough free nodes
            if ((m_tree.m_count + 4) >= m_item_array.m_committed)
            {
                const u32 capacity_increase = 16384;
                m_item_array.add_capacity(capacity_increase, sizeof(u32));
                m_node_array.add_capacity(capacity_increase, sizeof(u32));
                m_color_array.add_capacity(capacity_increase >> 5, sizeof(u32));
            }

            search_data_t search_data = {(u32 const*)m_item_array.m_ptr, user_data, cmp};

            ntree32::node_t inserted;
            if (ntree32::insert(m_tree, _root, _insert, s_intermediate_compare, &search_data, inserted))
                return true;
            return false;
        }

        bool tree_t::remove(node_t& _root, u32 _remove, item_cmp cmp, void const* user_data)
        {
            search_data_t search_data = {(u32 const*)m_item_array.m_ptr, user_data, cmp};

            ntree32::node_t removed;
            if (ntree32::remove(m_tree, _root, _remove, s_intermediate_compare, &search_data, removed))
            {
                m_tree.v_del_node(removed);
                return true;
            }
            return false;
        }

        u32 tree_t::get_item(node_t node) const
        {
            u32 const* items = (u32 const*)m_item_array.m_ptr;
            return items[node];
        }

    } // namespace npath
} // namespace ncore
