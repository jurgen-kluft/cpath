#include "cbase/c_allocator.h"
#include "ccore/c_debug.h"
#include "ccore/c_target.h"

#include "cpath/private/c_freelist.h"

namespace ncore
{
    namespace npath
    {
        // Note: Index 0xFFFFFFFF is reserved a 'null'
        void* freelist_alloc(u8* memory, u32 _item_size, u32& _free_head, u32& _free_index)
        {
            if (_free_head != 0xFFFFFFFF)
            {
                u32 const index = _free_head;
                _free_head      = *((u32*)(memory + index * _item_size));
                return memory + index * _item_size;
            }
            u32 const index = _free_index++;
            return memory + index * _item_size;
        }

        void freelist_free(void* item, u8* memory, u32 _item_size, u32& _free_head)
        {
            u32 const index = freelist_idx_of(item, memory, _item_size);
            *((u32*)item)   = _free_head;
            _free_head      = index;
        }

        void freelist_reset(u32& _free_head, u32& _free_index)
        {
            _free_head  = 0xFFFFFFFF;
            _free_index = 0;
        }
    } // namespace npath
} // namespace ncore
