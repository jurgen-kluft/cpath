# cpath: Technical Design Document

## Overview

**cpath** is a C++ library that provides an efficient, hierarchical path management system for directory and file paths. It abstracts filesystem paths with a unified API supporting multiple logical devices, relative/absolute path conversions, and memory-efficient string pooling.

## Purpose & Goals

### Primary Goals

1. **Unified Path Abstraction**: Provide consistent `dirpath_t` and `filepath_t` classes that abstract filesystem path operations
2. **Device Management**: Support multiple logical devices (drives, volumes, mounted paths) with transparent device handling
3. **Memory Efficiency**: Use string pooling and compact node representation to minimize memory footprint for large path hierarchies
4. **Hierarchical Path Organization**: Maintain paths as trees where each directory can have child files and subdirectories
5. **Cross-Platform Compatibility**: Support both Windows (`C:\`) and Unix-like (`/volume/`) path formats

### Design Principles

- **Absolute Paths Internally**: All paths are stored internally as absolute paths; relative paths are computed on-demand
- **Lazy Materialization**: Paths don't need to exist on disk to be registered—they're "inserted" logically
- **Type Safety**: Distinct classes (`dirpath_t`, `filepath_t`) prevent mixing directory and file operations
- **Efficient Lookup**: Tree-based structures enable O(log n) path lookups and comparisons

## Architecture

### Core Components

#### 1. **paths_t** - Central Registry
The main entry point and singleton-like registry managing all path infrastructure.

```cpp
struct paths_t {
    alloc_t*   m_allocator;  // Memory allocator
    strings_t* m_strings;    // String pooling system
    devices_t* m_devices;    // Device registry
    folders_t* m_folders;    // Folder hierarchy
};
```

**Responsibilities:**
- Create and manage all global path infrastructure
- Register devices, folders, and files
- Manage string pooling to reduce memory duplication
- Provide conversion utilities between path components

#### 2. **device_t** - Logical Device
Represents a logical device or mount point (e.g., `C:\`, `E:\`, `/Volumes/...`).

```cpp
struct device_t {
    string_t  m_name;       // Device name (e.g., "C")
    node_t    m_path;       // Root directory node
    idevice_t m_index;      // Index in device registry
    idevice_t m_redirector; // Reference to another device (for aliasing)
};
```

**Responsibilities:**
- Store device identity and root directory reference
- Support device redirection/aliasing
- Register absolute paths on this device
- Convert paths to/from strings

#### 3. **dirpath_t** - Directory Path Class
Public-facing class representing a directory path.

```cpp
class dirpath_t {
    device_t*  m_device;  // Logical device (C:, E:, /Volumes/...)
    node_t     m_base;    // Base directory (anchor point)
    node_t     m_path;    // Relative path from base
};
```

**Key Properties:**
- Never fully relative—always anchored to a device
- Three-part structure: `device + base + path`
- Example: `C:\documents\old\ + inventory\ + books\sci-fi\`
  - Device: `C:\documents\old\`
  - Base: `documents\old\`
  - Path: `books\sci-fi\`

**Public Methods:**
- `isEmpty()`, `isRoot()`, `isRooted()` - State queries
- `parent()`, `up()` - Navigate up hierarchy
- `down()`, `down(name)` - Navigate down hierarchy
- `filename(name)` - Create child file path
- `depth()` - Get nesting level
- `makeRelative()`, `makeAbsolute()` - Path conversion
- String conversion: `full_path_to_string()`, `relative_path_to_string()`, etc.

#### 4. **filepath_t** - File Path Class
Represents a specific file within a directory path.

```cpp
class filepath_t {
    dirpath_t      m_dirpath;   // Parent directory
    string_t       m_filename;  // Filename (no extension)
    string_t       m_extension; // File extension
};
```

**Public Methods:**
- Path navigation: `up()`, `down()`
- State queries: `isEmpty()`, `isRooted()`
- Conversion: `makeRelativeTo()`, `makeAbsoluteTo()`
- Comparison: `compare()`

#### 5. **strings_t** - String Pool
Memory-efficient string storage using interning/pooling.

```cpp
struct strings_t {
    // Stores unique strings with reference counting or pooling
    string_t find(crunes_t const& str) const;
    string_t insert(crunes_t const& str);
    string_t unregister(string_t str);
    crunes_t view_string(string_t str) const;
};
```

**Benefits:**
- Eliminates string duplication when multiple paths share components
- Reduces memory overhead for large path hierarchies
- Fast string lookups via hash tables

#### 6. **folders_t** - Folder Hierarchy
Manages the tree structure of directories and files.

```cpp
struct folders_t {
    folder_t*             m_array;        // Folder storage
    ntree32::tree_t       m_tree;         // Tree structure (red-black)
    ntree32::nnode_t*     m_nodes;        // Tree nodes
};

struct folder_t {
    string_t  m_name;         // Folder name (index into strings pool)
    node_t    m_parent;       // Parent folder node
    node_t    m_first_file;   // Root of file tree
    node_t    m_first_folder; // Root of subdirectory tree
};
```

**Structure:**
- Each folder contains two red-black trees: one for files, one for subdirectories
- Recursive structure allows arbitrary nesting depth
- Each node is an index into the compact array

#### 7. **devices_t** - Device Registry
Manages all registered devices.

```cpp
struct devices_t {
    device_t**        m_arr_devices;    // Device array
    ntree32::tree_t   m_device_tree;    // Tree for device lookup
    ntree32::node_t   m_device_tree_root;
};
```

**Responsibilities:**
- Register new devices
- Lookup devices by name
- Maintain default device
- Support device redirection

### Data Model: Hierarchical Tree Structure

The entire path registry is organized as a **hierarchical red-black tree**:

```
Root Device (e.g., C:)
├── Folder: documents
│   ├── Subfolder: old
│   │   ├── Subfolder: inventory
│   │   │   ├── Subfolder: books
│   │   │   │   └── Subfolder: sci-fi
│   │   │   └── File: data.txt
│   │   └── File: readme.md
│   └── File: report.docx
└── Folder: bin
```

**Key Characteristics:**
- Each folder node has two children trees:
  - **File tree**: Contains all files in this directory
  - **Subfolder tree**: Contains all subdirectories
- Tree nodes use red-black balancing for O(log n) operations
- Compact representation: nodes are indices, not pointers

### Index-Based Node System

Instead of pointers, the system uses indices (u32 values):

```cpp
typedef u32 node_t;           // Index into folder array
typedef u32 string_t;         // Index into string pool
typedef u32 ifolder_t;        // Folder index
typedef s16 idevice_t;        // Device index
```

**Advantages:**
- Compact memory layout (4-byte indices vs. 8-byte pointers)
- Stable references (resizing doesn't invalidate references)
- Cache-friendly array access
- Easy serialization

### Path Structure: Three-Part Model

Paths use a three-component model for efficient relative path handling:

```
┌─────────┬──────────┬──────────┐
│ Device  │ Base     │ Path     │
├─────────┼──────────┼──────────┤
│   C:    │ docum   │ inventory│
│         │ old     │ books    │
│         │         │ sci-fi   │
└─────────┴──────────┴──────────┘
         ↓          ↓          ↓
      Device    Base Anchor  Relative
    Identifier                Component
```

**Example Operations:**

1. **Full Path**: Concatenate device + base + path
   - Result: `C:\documents\old\inventory\books\sci-fi\`

2. **Relative Path**: Use only the path component
   - Result: `inventory\books\sci-fi\`

3. **Root Path**: Device + base
   - Result: `C:\documents\old\`

4. **Make Relative**: Compute relative path from one dirpath to another
   - Useful for creating reference-based paths

## Memory Layout

### Allocation Strategy

1. **Static Allocator**: Used via injected `alloc_t` interface
2. **String Pool**: Dynamic growth as new unique strings are registered
3. **Folder Array**: Pre-allocated with configurable capacity (default: 1GB in indices)
4. **Tree Nodes**: Allocated along with folders

### Memory Efficiency

- **String Deduplication**: Common path components (e.g., "documents", "bin") stored once
- **Index-Based References**: 4-byte indices instead of 8-byte pointers
- **Tree32 Implementation**: Compact red-black tree using array-based nodes
- **Zero Heap Fragmentation**: All structures use linear allocators

## Public API

### Initialization

```cpp
// Create path registry with specified capacity
paths_t* paths = g_construct_paths(allocator, 1024 * 1024 * 1024);

// Create path registry with default capacity (1GB)
paths_t* paths = g_construct_paths(allocator);

// Destroy path registry
g_destruct_paths(allocator, paths);
```

### Device Registration

```cpp
// Register a new device (e.g., "C:", "appdir", "/Volumes/MyDrive")
device_t* device = paths->register_device(make_crunes("C:"));
```

### Path Registration

```cpp
// Register absolute directory path (creates all intermediate folders)
dirpath_t dirpath = paths->register_fulldirpath(
    make_crunes("C:\\documents\\old\\inventory\\")
);

// Register absolute file path
filepath_t filepath = paths->register_fullfilepath(
    make_crunes("C:\\documents\\old\\data.txt")
);
```

### Navigation

```cpp
// Navigate up the directory tree
dirpath_t parent = dirpath.up();

// Navigate down (first child directory)
dirpath_t child = dirpath.down();

// Navigate down to specific named subdirectory
dirpath_t specific = dirpath.down(make_crunes("subfolder"));

// Create child file path
filepath_t file = dirpath.filename(make_crunes("data.txt"));

// Get parent directory of a file
dirpath_t file_parent = filepath.dirpath();
```

### Path Information

```cpp
// Query path components
string_t device_name = dirpath.devname();   // "C"
string_t root_name = dirpath.rootname();    // "documents"
string_t base_name = dirpath.basename();    // "inventory"
s32 depth = dirpath.depth();                // Number of levels

// Check path state
bool is_root = dirpath.isRoot();
bool is_empty = dirpath.isEmpty();
bool is_rooted = dirpath.isRooted();
```

### Path Conversion

```cpp
// Get different string representations
runes_t buffer = ...;

// Full absolute path
dirpath.full_path_to_string(buffer);   // "C:\documents\old\inventory\"

// Relative path from base
dirpath.relative_path_to_string(buffer); // "inventory\"

// Root path
dirpath.root_path_to_string(buffer);   // "C:\documents\"

// Base path (device + base)
dirpath.base_path_to_string(buffer);   // "C:\documents\old\"

// Query string lengths before allocating
s32 full_len = dirpath.full_path_to_strlen();
s32 rel_len = dirpath.relative_path_to_strlen();
```

### Relative Path Calculations

```cpp
// Make one path relative to another (compute relative path)
dirpath_t abs_path = ...; // "C:\documents\old\inventory\books\"
dirpath_t base_path = ...; // "C:\documents\old\"
dirpath_t relative = abs_path.makeRelative(base_path);

// Make a relative path absolute given a base
dirpath_t base = ...; // "C:\documents\"
dirpath_t rel = ...; // "old\inventory\"
dirpath_t absolute = base.makeAbsolute(rel);
```

### Comparison

```cpp
// Compare two paths
s8 cmp = dirpath1.compare(dirpath2);
// Returns: < 0 (less), 0 (equal), > 0 (greater)

if (dirpath1 == dirpath2) { /* ... */ }
if (dirpath1 != dirpath2) { /* ... */ }
```

## Usage Examples

### Example 1: Basic Directory Navigation

```cpp
// Setup
alloc_t* allocator = g_create_allocator();
paths_t* paths = g_construct_paths(allocator);

// Create device
device_t* device = paths->register_device(make_crunes("appdir"));

// Register full path (creates entire hierarchy)
dirpath_t data_dir = paths->register_fulldirpath(
    make_crunes("appdir:\\data\\cache\\temp\\")
);

// Navigate up
dirpath_t cache_dir = data_dir.up();     // appdir:\data\cache\
dirpath_t data_parent = cache_dir.up();  // appdir:\data\

// Navigate down
dirpath_t first_subdir = data_dir.down();
dirpath_t specific_dir = data_dir.down(make_crunes("logs"));

// Create file path
filepath_t config_file = data_dir.filename(make_crunes("config.cfg"));
```

### Example 2: Cross-Device Path Operations

```cpp
// Register multiple devices
device_t* dev_c = paths->register_device(make_crunes("C:"));
device_t* dev_e = paths->register_device(make_crunes("E:"));

// Create paths on different devices
dirpath_t source = paths->register_fulldirpath(
    make_crunes("C:\\projects\\src\\")
);
dirpath_t backup = paths->register_fulldirpath(
    make_crunes("E:\\backups\\projects\\")
);

// Compare devices
s8 cmp = source.compare(backup);  // Different devices
```

### Example 3: Relative Path Conversion

```cpp
// Create two paths
dirpath_t project_root = paths->register_fulldirpath(
    make_crunes("C:\\projects\\myapp\\")
);
dirpath_t source_dir = paths->register_fulldirpath(
    make_crunes("C:\\projects\\myapp\\src\\graphics\\")
);

// Get relative path
dirpath_t relative = source_dir.makeRelative(project_root);
// Result: base = myapp\, path = src\graphics\

// Convert to relative string
runes_t rel_str = ...;
relative.relative_path_to_string(rel_str);  // "src\graphics\"
```

## Internal Implementation Details

### Folder Allocation

Folders are allocated on-demand when registering paths:

```cpp
node_t path_node = allocate_folder(name_string);
```

Each folder tracks:
- Name (string index)
- Parent node reference
- File tree root
- Subfolder tree root

### Tree32 Integration

Uses the `tree32` module (from cbase) for red-black tree operations:
- **File Tree**: Organizes files within a directory
- **Subfolder Tree**: Organizes subdirectories within a directory
- Both use string comparison for ordering

### String Registration

When registering paths like `"C:\documents\old\inventory\"`:

1. Split by path separator: `["C:", "documents", "old", "inventory"]`
2. Register each component in string pool
3. Create or retrieve folder nodes for each component
4. Link nodes via parent-child relationships

### Device Registry

Devices are stored in their own red-black tree, keyed by device name (string comparison). This allows:
- O(log n) device lookup by name
- Support for device aliases/redirectors
- Atomic device registration

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Register device | O(log d) | d = number of devices |
| Register full path | O(k log n) | k = path depth, n = folders at each level |
| Lookup folder by name | O(log n) | Binary search within level |
| Navigate up | O(1) | Direct parent reference |
| Navigate down | O(log n) | Tree search for named child |
| Compare paths | O(k) | k = common depth |
| Path to string | O(k) | k = path depth |

## Design Decisions & Rationale

### 1. Index-Based Architecture
**Decision**: Use u32 indices instead of pointers for tree nodes and strings.

**Rationale**:
- Stable references across allocator operations
- Cache-friendly compact representation
- Easy serialization/deserialization
- Supports memory-mapped files

### 2. Three-Part Path Model
**Decision**: Separate device, base, and relative components.

**Rationale**:
- Efficient relative path computation without string manipulation
- Multiple representations without duplication
- Clean anchoring system for cross-path operations
- Memory efficient storage

### 3. Lazy Path Materialization
**Decision**: Allow registering paths that don't exist on disk.

**Rationale**:
- Supports virtual/abstract filesystems
- Decouples path management from disk I/O
- Enables testing without actual files
- Path registry is independent of OS filesystem

### 4. Hierarchical Tree Structure
**Decision**: Use nested trees (files + folders at each level).

**Rationale**:
- Natural representation of directory hierarchy
- Fast lookup within each level
- Supports directory iteration
- Maintains sorted ordering for consistency

### 5. String Pooling
**Decision**: Deduplicate strings via interning.

**Rationale**:
- Common path components stored once
- Massive memory savings in large hierarchies
- Fast string comparison (index comparison)
- Enables efficient string-based lookups

## Thread Safety & Concurrency

**Current Status**: The design assumes **single-threaded access**. No locks or synchronization primitives are used.

**Future Considerations**:
- Potential for read-write locks on device registry
- String pool could use atomic operations
- Path navigation could be thread-safe with immutable references

## Extensibility & Future Work

### Potential Enhancements

1. **Persistence**: Serialize/deserialize path registry to/from disk
2. **Path Compression**: Compress node storage further
3. **Lazy Device Loading**: Load devices on-demand from configuration
4. **Path Aliasing**: Support Windows junction points or Unix symlinks
5. **Wildcards**: Pattern matching for path enumeration
6. **File Metadata**: Associate attributes with files (size, modification time, etc.)
7. **Concurrent Access**: Thread-safe API with read-write locks

### Known Limitations

- Relative path operations (`makeRelative`, `makeAbsolute`) incomplete in current implementation
- No persistence/serialization support
- Single-threaded design
- No filesystem metadata association

## Dependencies

| Dependency | Purpose |
|-----------|---------|
| **ccore** | Memory allocation, debugging, target platform definitions |
| **cbase** | String utilities (runes), tree32 data structure, binary search, hash |
| **ctime** | Timestamp/datetime utilities (used in tests) |
| **cunittest** | Unit testing framework (used in test suite) |

## Testing Strategy

The test suite includes:

### Test Fixtures

1. **constructor1**: Basic dirpath construction
2. **register_device**: Device registration
3. **constructor2**: Full path registration
4. **to_string**: Path string conversion

### Test Coverage Areas

- Object lifecycle (construction, destruction, clearing)
- Device registration and lookup
- Path registration from full paths
- String conversion and representations
- Path navigation (parent, child)
- Path comparison

## Conclusion

**cpath** provides a robust, memory-efficient abstraction for hierarchical path management in C++. Its index-based architecture, string pooling, and tree-structured organization make it suitable for applications requiring large-scale path hierarchies or custom filesystem abstractions.

The design balances simplicity with efficiency, using well-established data structures (red-black trees, string interning) while maintaining a clean, type-safe API through distinct `dirpath_t` and `filepath_t` classes.
