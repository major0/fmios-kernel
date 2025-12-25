# Design Document

## Overview

The KFS (Kernel Filesystem) Namespace subsystem provides a complete filesystem implementation accessible via `//` paths, enabling service registration, discovery, and management through standard filesystem operations. It integrates seamlessly with the IPC system to provide transparent routing between kernel subsystems and userland services.

## Architecture

### KFS Architecture Overview

```
User/Kernel → VFS Operations → KFS Router → Service Handlers → Kernel/Userland
    ↓              ↓              ↓              ↓              ↓
  File Commands   open/read/     Path Resolution  Handler       Service
  ls, cat, etc.   write/stat     Service Lookup   Selection     Implementation
```

### Namespace Organization

```
//                          # Root namespace
├── kern/                   # Kernel subsystem information
│   ├── mem/               # Memory management statistics
│   ├── proc/              # Process management
│   └── sched/             # Scheduler information
├── proc/                  # Process information (like /proc)
├── services/              # Registered userland services
└── dev/                   # Device interfaces
```

## Components and Interfaces

### VFS Operations Interface

```c
struct vfs_operations_s {
  int (*open)(const char *path, int flags);
  int (*close)(int fd);
  ssize_t (*read)(int fd, void *buf, size_t count);
  ssize_t (*write)(int fd, const void *buf, size_t count);
  int (*readdir)(const char *path, struct dirent *entries, size_t max_entries);
  int (*stat)(const char *path, struct stat *statbuf);
} vfs_operations_t;
```

### Service Registration Interface

```c
struct service_entry_s {
  char name[64];
  char path[256];
  vfs_operations_t *operations;
  void *service_data;
  uint32_t permissions;
  struct list_head service_list;
  kobj_t service_kobj;
} service_entry_t;

int kfs_register_service(const char *path, vfs_operations_t *ops, void *data);
int kfs_unregister_service(const char *path);
```

## Correctness Properties

### Property 1: Service Registration Consistency
*For any* service registration operation, the service should be accessible through filesystem operations immediately after successful registration.
**Validates: Requirements 2.1, 2.2**

### Property 2: Filesystem Operation Atomicity
*For any* filesystem operation, it should complete atomically without interference from concurrent operations.
**Validates: Requirements 1.6, 1.7**

### Property 3: Path Resolution Correctness
*For any* valid KFS path, the path resolution should correctly identify the appropriate service handler.
**Validates: Requirements 1.1, 1.4**

## Error Handling

### Service Registration Errors
- Duplicate service paths → Return error and maintain existing registration
- Invalid service operations → Validate operations before registration
- Memory allocation failure → Return appropriate error code

### Filesystem Operation Errors
- Invalid paths → Return ENOENT error
- Permission denied → Return EACCES error
- Service unavailable → Return ECONNREFUSED error

## Testing Strategy

### Unit Testing
- Test service registration and deregistration
- Test filesystem operations (open, read, write, stat)
- Test path resolution and service lookup

### Property-Based Testing
- Test concurrent filesystem operations for atomicity
- Test service registration consistency
- Test path resolution correctness with random paths
