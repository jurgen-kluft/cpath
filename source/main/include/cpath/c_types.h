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
        struct device_t;
        struct devices_t;
        struct strings_t;
        struct folders_t;
        struct folder_t;
        struct files_t;
        struct paths_t;

        struct devices_t;

        typedef u32             ifolder_t;
        typedef u32             string_t;
        typedef ntree32::node_t node_t;
        typedef s16             idevice_t;

        const u32       c_invalid_file   = 0xFFFFFFFF;
        const u32       c_invalid_folder = 0xFFFFFFFF;
        const u32       c_invalid_node   = 0xFFFFFFFF;
        const u32       c_invalid_string = 0xFFFFFFFF;
        const idevice_t c_invalid_device = -1;
        const u32       c_empty_file     = 0x0;
        const u32       c_empty_folder   = 0x0;
        const u32       c_empty_string   = 0x0;
        const u32       c_empty_node     = 0x0;
        const idevice_t c_default_device = 0x0;

    } // namespace npath

} // namespace ncore

#endif // __C_PATH_PATH_REG_H__
