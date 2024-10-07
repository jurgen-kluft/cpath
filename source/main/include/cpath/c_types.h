#ifndef __C_PATH_TYPES_H__
#define __C_PATH_TYPES_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    class alloc_t;

    class filepath_t;
    class dirpath_t;

    namespace ntree32
    {
        typedef u32 node_t;
    }

    namespace npath
    {
        const u32 c_invalid_file   = 0xFFFFFFFF;
        const u32 c_invalid_folder = 0xFFFFFFFF;
        const u32 c_invalid_node   = 0xFFFFFFFF;
        const u32 c_invalid_string = 0xFFFFFFFF;
        const u32 c_empty_file     = 0x0;
        const u32 c_empty_folder   = 0x0;
        const u32 c_empty_string   = 0x0;
        const u32 c_empty_node     = 0x0;

        struct device_t;
        struct devices_t;
        struct strings_t;
        struct folders_t;
        struct folder_t;
        struct files_t;
        struct instance_t;

        typedef u32             ifolder_t;
        typedef u32             string_t;
        typedef ntree32::node_t node_t;

    } // namespace npath

} // namespace ncore

#endif // __C_PATH_PATH_REG_H__
