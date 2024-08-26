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
        inode_t tree_t::rotate_single(inode_t node, s32 dir)
        {
            inode_t const save = v_get_child(node, 1 - dir);
            v_set_child(node, 1 - dir, v_get_child(save, dir));
            v_set_child(save, dir, node);
            v_set_red(node);
            v_set_black(save);
            return save;
        }

        inode_t tree_t::rotate_double(inode_t node, s32 dir)
        {
            v_set_child(node, 1 - dir, rotate_single(v_get_child(node, 1 - dir), 1 - dir));
            return rotate_single(node, dir);
        }

        inode_t tree_t::rotate_single_track_parent(inode_t node, s32 dir, inode_t fn, inode_t& fp)
        {
            inode_t const save = rotate_single(node, dir);

            if (fn == node)
                fp = save;
            else if (fn == v_get_child(node, 1 - dir)) // never triggered
                fp = node;

            return save;
        }

        inode_t tree_t::rotate_double_track_parent(inode_t node, s32 dir, inode_t fn, inode_t& fp)
        {
            inode_t const child = rotate_single_track_parent(v_get_child(node, 1 - dir), 1 - dir, fn, fp);
            v_set_child(node, 1 - dir, child);

            if (fn == child) // never triggered
                fp = node;

            inode_t const save = rotate_single_track_parent(node, dir, fn, fp);
            if (fn == node)
                fp = save;
            return save;
        }

        bool tree_t::v_clear(inode_t& _root, inode_t& removed_node)
        {
            removed_node = 0;

            inode_t node = _root;
            if (node == 0)
                return true;

            inode_t todelete = node;
            if (v_get_left(node) == 0)
            {
                // tree->v_set_root(node->get_right(tree));
                _root = v_get_right(node);
            }
            else if (v_get_right(node) == 0)
            {
                // tree->v_set_root(node->get_left(tree));
                _root = v_get_left(node);
            }
            else
            {
                // We have left and right branches
                // Take right branch and place it
                // somewhere down the left branch
                inode_t branch = v_get_right(node);
                v_set_child(node, RIGHT, 0);

                // Find a node in the left branch that does not
                // have both a left and right branch and place
                // our branch there.
                inode_t iter = v_get_left(node);
                while (v_get_left(iter) != 0 && v_get_right(iter) != 0)
                {
                    iter = v_get_left(iter);
                }
                if (v_get_left(iter) == 0)
                {
                    v_set_child(iter, LEFT, branch);
                }
                else if (v_get_right(iter) == 0)
                {
                    // iter->set_right(tree, branch);
                    v_set_child(iter, RIGHT, branch);
                }

                // tree->v_set_root(node->get_left(tree));
                _root = v_get_left(node);
            }

            removed_node = todelete;
            return false;
        }

        bool tree_t::v_clear(inode_t& _root)
        {
            inode_t node   = 0;
            bool    result = v_clear(_root, node);
            if (node != 0)
            {
                v_del_node(node);
            }
            return result;
        }

        // validate the tree (return violation description in 'result'), also returns black height
        s32 tree_t::v_validate(inode_t _root, const char*& result, item_cmp cmp, void const* user_data) const
        {
            if (_root == 0)
            {
                return 1;
            }
            else
            {
                inode_t const root = _root;
                inode_t const ln   = v_get_left(root);
                inode_t const rn   = v_get_right(root);

                // Consecutive red links
                if (v_is_red(root))
                {
                    if (v_is_red(ln) || v_is_red(rn))
                    {
                        result = "Red violation";
                        return 0;
                    }
                }

                s32 const lh = v_validate(ln, result, cmp, user_data);
                s32 const rh = v_validate(rn, result, cmp, user_data);

                // Invalid binary search tree
                if ((ln != 0 && cmp(ln, _root, user_data) >= 0) || (rn != 0 && cmp(rn, _root, user_data) <= 0))
                {
                    result = "Binary tree violation";
                    return 0;
                }

                if (lh != 0 && rh != 0 && lh != rh) // Black height mismatch
                {
                    result = "Black violation";
                    return 0;
                }

                if (lh != 0 && rh != 0) // Only count black links
                {
                    return v_is_red(root) ? lh : lh + 1;
                }
            }
            return 0;
        }

        void tree_t::init(alloc_t* allocator, u32 init_num_items, u32 max_num_items)
        {
            m_item_array.init(init_num_items, max_num_items, sizeof(u32));
            m_node_left_array.init(init_num_items, max_num_items, sizeof(u32));
            m_node_right_array.init(init_num_items, max_num_items, sizeof(u32));
            m_color_array.init(init_num_items, max_num_items >> 5, sizeof(u32));
            m_free_head  = 0;
            m_free_index = 1;
        }

        void tree_t::exit(alloc_t* allocator)
        {
            m_item_array.exit();
            m_node_left_array.exit();
            m_node_right_array.exit();
            m_color_array.exit();
            m_free_head  = 0;
            m_free_index = 1;
        }

        void tree_t::reset()
        {
            m_free_head  = 0;
            m_free_index = 1;
        }

        inode_t tree_t::find(inode_t root, u32 const find, item_cmp cmp, void const* user_data)
        {
            inode_t node = root;
            while (node != 0)
            {
                u32 const item = v_get_item(node);
                s8 const  c    = cmp(find, item, user_data);
                if (c == 0)
                    return node;
                node = v_get_child(node, (c + 1) >> 1);
            }
            return 0;
        }

        bool tree_t::insert(inode_t& _root, u32 const _insert, item_cmp cmp, void const* user_data)
        {
            bool    inserted = false;
            inode_t root     = _root;
            if (root == 0)
            {
                // We have an empty tree; attach the
                // new node directly to the root
                _root    = v_new_node(_insert);
                inserted = true;
            }
            else
            {
                inode_t head = v_get_temp(); // False tree root
                inode_t g, t;                // Grandparent & parent
                inode_t p, n;                // Iterator & parent
                s32     dir = 0, last = 0;

                // Set up our helpers
                t = head;
                v_set_black(t); // Color it black
                v_set_right(t, root);
                v_set_left(t, 0);

                g = p = 0;
                n     = root;

                // Search down the tree for a place to insert
                for (;;)
                {
                    if (n == 0)
                    {
                        // Insert a new node at the first null link
                        n = v_new_node(_insert);
                        v_set_child(p, dir, n);
                        inserted = true;
                    }
                    else if (v_is_red(v_get_left(n)) && v_is_red(v_get_right(n)))
                    {
                        // Simple red violation: color flip
                        v_set_red(n);
                        v_set_black(v_get_left(n));
                        v_set_black(v_get_right(n));
                    }

                    if (v_is_red(n) && v_is_red(p))
                    {
                        // Hard red violation: rotations necessary
                        const s32 dir2 = (v_get_right(t) == g) ? 1 : 0;

                        if (n == v_get_child(p, last))
                            v_set_child(t, dir2, rotate_single(g, 1 - last));
                        else
                            v_set_child(t, dir2, rotate_double(g, 1 - last));
                    }

                    // Stop working if we inserted a node. This
                    // check also disallows duplicates in the tree
                    last = dir;
                    dir  = cmp(_insert, v_get_item(n), user_data);
                    if (dir == 0)
                        break;
                    dir = ((dir + 1) >> 1);

                    // Move the helpers down
                    if (g != 0)
                        t = g;

                    g = p;
                    p = n;
                    n = v_get_child(n, (echild_t)dir);
                }

                // Update the root (it may be different)
                root  = v_get_right(head);
                _root = root;
            }

            // Make the root black for simplified logic
            root = _root;
            v_set_color(root, BLACK);

            return inserted;
        }

        bool tree_t::remove(inode_t& _root, u32 _remove, item_cmp cmp, void const* user_data)
        {
            inode_t root = _root;
            if (root == 0)
                return false;

            inode_t head = v_get_temp(); // False tree root
            inode_t fn   = 0;            // Found node
            inode_t fp   = 0;            // Found node parent
            s32     dir  = 1;

            // Set up our helpers
            inode_t g = 0;
            inode_t p = 0;

            inode_t n = head;
            v_set_black(n); // Color it black
            v_set_right(n, root);
            v_set_left(n, 0);

            // Search and push a red node down
            // to fix red violations as we go
            while (v_get_child(n, (echild_t)dir) != 0)
            {
                const s32 last = dir;

                // Move the helpers down
                g = p;
                p = n;
                n = v_get_child(n, (echild_t)dir);
                // dir = tree->v_compare_insert(key, n);
                dir = cmp(_remove, v_get_item(n), user_data);

                // Save the node with matching data and keep
                // going; we'll do removal tasks at the end
                if (dir == 0)
                {
                    fn = n;
                    fp = p;
                }
                dir = ((dir + 1) >> 1);

                /* Push the red node down with rotations and color flips */
                if (!v_is_red(n) && !v_is_red(v_get_child(n, (echild_t)dir)))
                {
                    if (v_is_red(v_get_child(n, (echild_t)(1 - dir))))
                    {
                        inode_t r = rotate_single_track_parent(n, dir, fn, fp);
                        v_set_child(p, (echild_t)last, r);
                        if (fn == r) // never triggered
                            fp = p;
                        p = r;
                    }
                    else if (!v_is_red(v_get_child(n, (echild_t)(1 - dir))))
                    {
                        inode_t s = v_get_child(p, (echild_t)(1 - last));
                        if (s != 0)
                        {
                            if (!v_is_red(v_get_child(s, (echild_t)(1 - last))) && !v_is_red(v_get_child(s, (echild_t)last)))
                            {
                                // Color flip
                                v_set_black(p);
                                v_set_red(s);
                                v_set_red(n);
                            }
                            else
                            {
                                const s32 dir2 = v_get_right(g) == p ? 1 : 0;
                                if (v_is_red(v_get_child(s, (echild_t)last)))
                                {
                                    inode_t r = rotate_double_track_parent(p, last, fn, fp);
                                    v_set_child(g, (echild_t)dir2, r);
                                    if (fn == r) // never triggered
                                        fp = g;
                                }
                                else if (v_is_red(v_get_child(s, (echild_t)(1 - last))))
                                {
                                    inode_t r = rotate_single_track_parent(p, last, fn, fp);
                                    v_set_child(g, (echild_t)dir2, r);
                                    if (fn == r) // never triggered
                                        fp = g;
                                }

                                // Ensure correct coloring
                                v_set_red(n);
                                v_set_red(v_get_child(g, (echild_t)dir2));

                                v_set_black(v_get_left(v_get_child(g, dir2)));
                                v_set_black(v_get_right(v_get_child(g, dir2)));
                            }
                        }
                    }
                }
            }

            // Update the root (it may be different)
            root = v_get_right(head);

            // Replace and remove the saved node
            if (fn != 0)
            {
                ASSERT(v_get_right(fp) == fn || v_get_left(fp) == fn);
                ASSERT(v_get_right(p) == n || v_get_left(p) == n);

                inode_t child1 = v_get_child(n, v_get_left(n) == 0 ? RIGHT : LEFT);
                v_set_child(p, v_get_right(p) == n ? RIGHT : LEFT, child1);

                if (fn != n)
                {
                    ASSERT(fp != p);

                    // swap 'n' and 'fn', we want to remove the node that was holding 'item'
                    v_set_child(fp, v_get_right(fp) == fn ? RIGHT : LEFT, n);
                    v_set_child(n, LEFT, v_get_left(fn));
                    v_set_child(n, RIGHT, v_get_right(fn));
                    v_set_color(n, v_get_color(fn));
                }

                if (root == fn)
                    root = n;

                v_del_node(fn);
            }

            // Make the root black for simplified logic
            _root = root;
            if (root != 0)
            {
                v_set_black(root);
            }
            return true;
        }

        u32 tree_t::get_item(inode_t node) const { return v_get_item(node); }

        u32  tree_t::v_get_item(inode_t node) const { return *(u32*)(m_item_array.m_ptr + node * sizeof(u32)); }
        void tree_t::v_set_item(inode_t node, u32 item) { *(u32*)(m_item_array.m_ptr + node * sizeof(u32)) = item; }

        void     tree_t::v_set_color(inode_t node, ecolor_t color) { m_color_array.m_ptr[node >> 3] = (m_color_array.m_ptr[node >> 3] & ~(1 << (node & 0x07))) | ((u8)color << (node & 0x07)); }
        ecolor_t tree_t::v_get_color(inode_t node) const { return v_is_red(node) ? RED : BLACK; }
        bool     tree_t::v_is_red(inode_t node) const { return node != 0 && (m_color_array.m_ptr[node >> 3] & (1 << (node & 0x07))) == 0; }
        bool     tree_t::v_is_black(inode_t node) const { return node != 0 && ((m_color_array.m_ptr[node >> 3] & (1 << (node & 0x07))) != 0); }
        void     tree_t::v_set_red(inode_t node) { m_color_array.m_ptr[node >> 3] &= ~(1 << (node & 0x07)); }
        void     tree_t::v_set_black(inode_t node) { m_color_array.m_ptr[node >> 3] |= (1 << (node & 0x07)); }

        inode_t tree_t::v_get_child(inode_t node, s8 ne) const
        {
            u32 const index = node;
            switch (ne)
            {
                case LEFT: return v_get_left(index);
                case RIGHT: return v_get_right(index);
            }
            return 0;
        }

        void tree_t::v_set_child(inode_t node, s8 ne, inode_t set)
        {
            u32 const index = node;
            switch (ne)
            {
                case LEFT: v_set_left(index, set); break;
                case RIGHT: v_set_right(index, set); break;
            }
        }

        inode_t tree_t::v_get_left(inode_t node) const
        {
            if (node == 0)
                return 0;
            return *(inode_t const*)(m_item_array.m_ptr + node * sizeof(u32));
        }

        void tree_t::v_set_left(inode_t node, inode_t child)
        {
            if (node == 0)
                return;
            *((u32*)m_node_left_array.m_ptr + node) = child;
        }

        inode_t tree_t::v_get_right(inode_t node) const
        {
            if (node == 0)
                return 0;
            return *(inode_t const*)(m_item_array.m_ptr + node * sizeof(u32));
        }

        void tree_t::v_set_right(inode_t node, inode_t child)
        {
            if (node == 0)
                return;
            *((u32*)m_node_right_array.m_ptr + node) = child;
        }

        // Optimization, we can also reserve index 1 for the temporary node
        inode_t tree_t::v_get_temp() { return v_new_node(0); }

        inode_t tree_t::v_new_node(u32 _item)
        {
            u32 index;
            if (m_free_head > 0)
            {
                index       = m_free_head;
                m_free_head = *((u32*)m_item_array.m_ptr + m_free_head);
            }
            else
            {
                if (m_free_index >= m_item_array.m_committed)
                {
                    const u32 capacity_increase = 16384;
                    m_item_array.add_capacity(capacity_increase, sizeof(u32));
                    m_node_left_array.add_capacity(capacity_increase, sizeof(u32));
                    m_node_right_array.add_capacity(capacity_increase, sizeof(u32));
                    m_color_array.add_capacity(capacity_increase >> 5, sizeof(u32));
                }
                index = m_free_index++;
            }

            // initialize this new node
            v_set_red(index);
            v_set_left(index, 0);
            v_set_right(index, 0);
            v_set_item(index, _item);
            return index;
        }

        void tree_t::v_del_node(inode_t node)
        {
            *((u32*)m_item_array.m_ptr + node) = m_free_head;
            m_free_head                        = node;
        }

    } // namespace npath
} // namespace ncore
