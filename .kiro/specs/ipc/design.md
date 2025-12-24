# Design Document

## Overview

The IPC (Inter-Process Communication) subsystem provides Plan 9-style filesystem-based communication through the KFS namespace. Userland processes traverse the KFS namespace to discover services, bind service paths into their local namespace, attach to services, and perform authenticated file operations. This maintains a strict client/server model with capability-based security.

## Architecture

### IPC Interaction Flow

```
Userland Process → KFS Traversal → Service Discovery → Bind/Mount → IPC Library → IPC Messages → Auth → File Operations
       ↓               ↓              ↓               ↓         ↓            ↓             ↓       ↓           ↓
   open("//")    readdir/stat    find service    bind //svc /svc  kattach()  ATTACH msg  auth msgs  AUTH_READ/WRITE
```

### Service Discovery and Binding Model

```
1. Discovery:
   fd = open("//", O_RDONLY);
   readdir(fd, entries, count);  // Discover available services
   close(fd);

2. Binding (userland command):
   bind //proc /proc             // Command-line tool mounts service into local namespace

3. Attachment (userland library function → IPC message):
   fd = kattach("//proc");       // Library function sends ATTACH message to IPC subsystem
                                 // Returns special fd for IPC operations

4. Authentication (if required):
   auth_fd = get_auth_interface(fd);      // Library function sends GET_AUTH message
   perform_sasl_challenge(auth_fd);       // Library function sends AUTH_CHALLENGE messages
   capability = complete_auth(auth_fd);   // Library function sends COMPLETE_AUTH message

5. File Operations:
   data_fd = auth_open("/proc/1/status", O_RDONLY, capability);  // Sends AUTH_OPEN message
   read(data_fd, buffer, size);                                  // Sends AUTH_READ message
   close(data_fd);                                               // Sends AUTH_CLOSE message
```

### Namespace Protection

```
Service Registration Rules:
✓ //service1/          (allowed - top-level service)
✓ //service2/          (allowed - top-level service)
✗ //service1/hijack/   (prohibited - child of existing service)
✗ //service2/fake/     (prohibited - namespace hijacking attempt)
```

## Components and Interfaces

### KFS Namespace Operations

```c
/* KFS traversal and discovery */
int kfs_open(const char *path, int flags);
int kfs_readdir(int fd, struct dirent *entries, size_t count);
int kfs_stat(const char *path, struct stat *statbuf);
int kfs_close(int fd);
```

### High-Level IPC Library (Userland)

```c
/* High-level IPC library functions (implemented in userland) */
/* These translate to IPC message types sent to the kernel IPC subsystem */

/* Service attachment */
int kattach(const char *service_path);    /* Sends ATTACH message type */
int kdetach(int service_fd);              /* Sends DETACH message type */

/* Authentication interface */
int get_auth_interface(int service_fd);   /* Sends GET_AUTH message type */
int auth_challenge(int auth_fd, const void *challenge, size_t challenge_len,
                   void *response, size_t *response_len);  /* Sends AUTH_CHALLENGE message type */
capability_t complete_auth(int auth_fd);  /* Sends COMPLETE_AUTH message type */

/* Authenticated file operations */
int auth_open(const char *path, int flags, capability_t cap);     /* Sends AUTH_OPEN message type */
ssize_t auth_read(int fd, void *buf, size_t count, capability_t cap);   /* Sends AUTH_READ message type */
ssize_t auth_write(int fd, const void *buf, size_t count, capability_t cap);  /* Sends AUTH_WRITE message type */
```

### IPC Message Types

```c
/* IPC message types handled by kernel IPC subsystem */
enum ipc_message_type {
    IPC_ATTACH = 1,           /* Service attachment request */
    IPC_DETACH,               /* Service detachment request */
    IPC_GET_AUTH,             /* Get authentication interface */
    IPC_AUTH_CHALLENGE,       /* Authentication challenge/response */
    IPC_COMPLETE_AUTH,        /* Complete authentication */
    IPC_AUTH_OPEN,            /* Authenticated file open */
    IPC_AUTH_READ,            /* Authenticated file read */
    IPC_AUTH_WRITE,           /* Authenticated file write */
    IPC_AUTH_CLOSE,           /* Authenticated file close */
    IPC_AUTH_STAT,            /* Authenticated file stat */
};

/* IPC message structure */
struct ipc_message_s {
    enum ipc_message_type type;
    size_t payload_size;
    capability_t capability;    /* For authenticated operations */
    char payload[];            /* Variable-length payload */
} ipc_message_t;
```

### Service Registration Structure

```c
struct service_registration_s {
    kobj_t kobj;                    /* Kernel object header */
    char service_path[256];         /* Full KFS path (e.g., "//proc") */
    char service_name[64];          /* Service name */
    
    /* Registration validation */
    bool is_top_level;              /* Must be true for valid registration */
    struct service_registration_s *parent; /* Must be NULL for top-level */
    
    /* Service capabilities */
    bool requires_auth;             /* Service requires authentication */
    auth_method_t auth_method;      /* SASL, capability-based, etc. */
    
    /* Service operations */
    struct service_ops *ops;        /* Service-specific operations */
    void *service_data;             /* Service private data */
} service_registration_t;
```

### Client Attachment State

```c
struct client_attachment_s {
    kobj_t kobj;                    /* Kernel object header */
    process_t *client_process;      /* Attached client process */
    service_registration_t *service; /* Target service */
    
    /* Attachment state */
    enum attachment_state state;    /* DETACHED, ATTACHING, ATTACHED, AUTHENTICATED */
    int auth_fd;                    /* Authentication file descriptor */
    capability_t capability;        /* Authentication capability */
    
    /* Bound paths */
    char local_path[256];           /* Local mount point */
    char service_path[256];         /* Service KFS path */
} client_attachment_t;
```

### Capability Restriction Chain Example

```c
/* Multi-level capability restriction example */

// 1. Init has root capability
capability_t *init_cap = get_root_capability();  /* Full "///" access */

// 2. Init creates restricted capability for container manager
uint32_t container_mgr_perms = PERM_READ | PERM_TRAVERSE | PERM_BIND | PERM_DELEGATE;
capability_t *container_mgr_cap = restrict_capability(init_cap, container_mgr_perms);

// 3. Container manager creates further restricted capability for container
const char *container_paths[] = { "//proc/", "//kern/version", NULL };
capability_t *container_base_cap = restrict_capability_paths(container_mgr_cap, container_paths);

// 4. Container creates read-only capability for application
uint32_t app_perms = PERM_READ | PERM_TRAVERSE;  /* No write, bind, or delegate */
capability_t *app_cap = restrict_capability(container_base_cap, app_perms);

// 5. Application receives most restricted capability
pass_capability_fd(capability_to_fd(app_cap), app_process);
```

### Capability Validation Rules

```c
/* Capability restriction validation */
bool can_restrict_capability(capability_t *original_cap, capability_restrictions_t *restrictions) {
    // Can only remove permissions, not add them
    if ((restrictions->permissions & ~original_cap->permissions) != 0) {
        return false;
    }
    
    // Can only set earlier expiry time
    if (restrictions->expiry_time > original_cap->expiry_time) {
        return false;
    }
    
    // Can only restrict to subset of allowed paths
    if (!is_path_subset(restrictions->allowed_paths, original_cap->allowed_paths)) {
        return false;
    }
    
    return true;
}
```

```
Kernel Boot Sequence:
1. Kernel initializes IPC subsystem
2. Kernel creates root capability for "///" namespace with full permissions
3. Kernel starts init process
4. Kernel grants init process the root capability
5. Init process can now:
   - Traverse entire "///" namespace
   - Bind any service into its namespace
   - Attach to any service
   - Delegate reduced capabilities to child processes

Child Process Capability Flow:
init (root "///" cap) → child1 (reduced cap) → grandchild (further reduced cap)
                    → child2 (different reduced cap)
                    → child3 (minimal cap)
```

### Capability Delegation Example

```c
/* Init process delegates capability to child */
capability_t *init_root_cap = get_root_capability();  /* Full "///" access */

/* Create reduced capability for child process */
uint32_t child_permissions = PERM_READ | PERM_TRAVERSE;  /* No write access */
capability_t *child_cap = delegate_capability(init_root_cap, child_process, child_permissions);

/* Pass capability as file descriptor to child */
int cap_fd = capability_to_fd(child_cap);
pass_fd_to_child(child_process, cap_fd);

/* Child process receives and uses the capability */
// In child process:
capability_t *inherited_cap = receive_capability_fd(cap_fd);
int service_fd = auth_open("//proc", O_RDONLY, inherited_cap);
```

```c
struct ipc_subsystem_s {
    kobj_t kobj;                    /* Kernel object header */
    
    /* Service registry */
    hash_table_t *services;         /* Registered services by path */
    spinlock_t registry_lock;       /* Protects service registry */
    
    /* Client attachments */
    hash_table_t *attachments;      /* Active client attachments */
    spinlock_t attachment_lock;     /* Protects attachment table */
    
    /* Namespace bindings */
    hash_table_t *bindings;         /* Process namespace bindings */
    spinlock_t binding_lock;        /* Protects binding table */
    
    /* Statistics */
    atomic_t services_registered;
    atomic_t services_unregistered;
    atomic_t attachments_created;
    atomic_t attachments_destroyed;
    atomic_t auth_challenges;
    atomic_t auth_successes;
    atomic_t auth_failures;
} ipc_subsystem_t;

/* Service registration validation */
int validate_service_path(const char *path);
bool is_namespace_hijacking(const char *path);
service_registration_t *find_parent_service(const char *path);
```

## Data Models

### Attachment States

```c
enum attachment_state {
    ATTACHMENT_DETACHED = 0,        /* No attachment */
    ATTACHMENT_ATTACHING,           /* Attachment in progress */
    ATTACHMENT_ATTACHED,            /* Attached but not authenticated */
    ATTACHMENT_AUTHENTICATING,      /* Authentication in progress */
    ATTACHMENT_AUTHENTICATED,       /* Fully authenticated */
    ATTACHMENT_ERROR                /* Error state */
};
```

### Authentication Methods

```c
enum auth_method {
    AUTH_NONE = 0,                  /* No authentication required */
    AUTH_SASL,                      /* SASL challenge/response */
    AUTH_CAPABILITY,                /* Capability-based */
    AUTH_CUSTOM                     /* Service-specific method */
};
```

### Capability Structure

```c
struct capability_s {
    uint64_t capability_id;         /* Unique capability identifier */
    process_t *original_owner;      /* Process that originally obtained this capability */
    process_t *current_owner;       /* Current process holding this capability */
    service_registration_t *service; /* Service this capability grants access to */
    uint32_t permissions;           /* Access permissions granted */
    uint32_t delegatable_permissions; /* Permissions that can be delegated to children */
    uint64_t expiry_time;           /* Capability expiration time */
    uint32_t magic;                 /* Validation magic number */
    
    /* Inheritance chain */
    struct capability_s *parent_cap; /* Parent capability (if inherited) */
    list_t child_caps;              /* List of capabilities delegated from this one */
    atomic_t ref_count;             /* Reference count for sharing */
} capability_t;
```

### Capability Restriction and Modification

```c
/* Capability restriction operations */
capability_t *restrict_capability(capability_t *original_cap, uint32_t new_permissions);
capability_t *restrict_capability_paths(capability_t *original_cap, const char **allowed_paths);
capability_t *restrict_capability_time(capability_t *original_cap, uint64_t new_expiry);
capability_t *restrict_capability_operations(capability_t *original_cap, uint32_t allowed_ops);

/* Combined restriction */
struct capability_restrictions_s {
    uint32_t permissions;           /* Reduced permission set */
    char **allowed_paths;           /* Restricted path list */
    size_t path_count;              /* Number of allowed paths */
    uint64_t expiry_time;           /* Earlier expiry time */
    uint32_t allowed_operations;    /* Restricted operation set */
} capability_restrictions_t;

capability_t *apply_restrictions(capability_t *original_cap, 
                                capability_restrictions_t *restrictions);
```

### Permission Restriction Model

```c
/* Original capability permissions */
#define PERM_READ           0x0001
#define PERM_WRITE          0x0002
#define PERM_EXECUTE        0x0004
#define PERM_TRAVERSE       0x0008
#define PERM_BIND           0x0010
#define PERM_DELEGATE       0x0020
#define PERM_ADMIN          0x0040
#define PERM_ALL            0xFFFF

/* Capability restriction examples */
uint32_t original_perms = PERM_READ | PERM_WRITE | PERM_TRAVERSE | PERM_BIND;
uint32_t restricted_perms = PERM_READ | PERM_TRAVERSE;  /* Remove write and bind */

capability_t *restricted_cap = restrict_capability(original_cap, restricted_perms);
```

### Path-Based Restrictions

```c
/* Restrict capability to specific paths */
const char *allowed_paths[] = {
    "//proc/self/",     /* Only access to own process info */
    "//kern/version",   /* Only kernel version info */
    NULL
};

capability_t *path_restricted_cap = restrict_capability_paths(original_cap, allowed_paths);
```

### Container-Style Capability Restrictions

```c
/* Container capability restriction example */
struct container_capability_s {
    capability_t base_cap;          /* Base capability structure */
    
    /* Container-specific restrictions */
    char *chroot_path;              /* Restricted root path */
    uint32_t namespace_mask;        /* Available namespace types */
    uint64_t resource_limits;       /* Resource usage limits */
    bool network_isolated;          /* Network isolation flag */
    bool filesystem_readonly;       /* Read-only filesystem access */
} container_capability_t;

/* Create container capability from parent */
container_capability_t *create_container_capability(capability_t *parent_cap,
                                                   const char *container_root,
                                                   uint32_t restrictions);
```

```c
/* Capability inheritance operations */
int inherit_capability(capability_t *parent_cap, process_t *child_process, 
                      uint32_t reduced_permissions);
int delegate_capability(capability_t *cap, process_t *target_process,
                       uint32_t delegated_permissions);
int revoke_capability(capability_t *cap);

/* File descriptor passing for capabilities */
int pass_capability_fd(int cap_fd, process_t *target_process);
capability_t *receive_capability_fd(int inherited_fd);

/* Init process bootstrap */
capability_t *create_root_capability(void);  /* Kernel creates root "///" capability for init */
int grant_init_capability(process_t *init_process, capability_t *root_cap);
```

### Process Capability Management

```c
struct process_capabilities_s {
    kobj_t kobj;                    /* Kernel object header */
    process_t *owner;               /* Owning process */
    
    /* Capability storage */
    hash_table_t *capabilities;     /* Capabilities owned by this process */
    spinlock_t cap_lock;            /* Protects capability table */
    
    /* Inheritance tracking */
    list_t inherited_caps;          /* Capabilities inherited from parent */
    list_t delegated_caps;          /* Capabilities delegated to children */
    
    /* Root namespace access */
    capability_t *root_capability;  /* Capability for "///" namespace access */
} process_capabilities_t;
```

## Data Models

### IPC States

```c
enum ipc_state {
    IPC_DISCONNECTED = 0,           /* Channel not connected */
    IPC_CONNECTING,                 /* Connection in progress */
    IPC_CONNECTED,                  /* Active connection */
    IPC_DISCONNECTING,              /* Disconnection in progress */
    IPC_ERROR                       /* Error state */
};
```

### Memory Mapping

```c
struct ipc_mapping_s {
    void *kernel_addr;              /* Kernel virtual address */
    void *user_addr;                /* User virtual address */
    size_t size;                    /* Mapping size */
    int permissions;                /* Access permissions */
    atomic_t ref_count;             /* Reference count */
} ipc_mapping_t;
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Namespace Hijacking Prevention
*For any* service registration attempt, if the path is a child of an existing service path, the registration should be rejected to prevent namespace hijacking.
**Validates: Requirements 1.3, 7.6**

### Property 2: Attachment State Consistency
*For any* client attachment, file operations should only be permitted after successful attachment and authentication (if required).
**Validates: Requirements 1.2, 7.1**

### Property 3: Service Discovery Completeness
*For any* KFS namespace traversal, all registered top-level services should be discoverable through standard directory operations.
**Validates: Requirements 2.3, 6.2**

### Property 4: Authentication Challenge Integrity
*For any* authentication sequence, the challenge/response protocol should complete successfully before granting access capabilities.
**Validates: Requirements 7.2, 7.4**

### Property 5: Capability-Based Access Control
*For any* authenticated file operation, the operation should only succeed if the provided capability grants the required permissions.
**Validates: Requirements 7.1, 7.11**

### Property 6: Binding Isolation
*For any* process namespace binding, the binding should only affect the requesting process and not interfere with other processes' namespaces.
**Validates: Requirements 6.1, 6.7**

### Property 7: Capability Restriction Monotonicity
*For any* capability restriction operation, the resulting capability should have equal or fewer permissions than the original capability, ensuring security cannot be escalated.
**Validates: Requirements 7.11, 8.5**

## Error Handling

### Service Registration Errors
- Namespace hijacking attempt → Return -EPERM and reject registration
- Duplicate service path → Return -EEXIST
- Invalid service path → Return -EINVAL
- Registration failure → Return -ENOMEM and clean up resources

### Attachment Errors
- Service not found → Return -ENOENT
- Attachment without binding → Return -ENOTCONN
- Authentication required → Return auth interface file descriptor
- Authentication failure → Return -EACCES and close attachment
- Invalid attachment state → Return -EINVAL

### File Operation Errors
- Operation without attachment → Return -ENOTCONN
- Operation without authentication → Return -EACCES
- Invalid capability → Return -EPERM
- Expired capability → Return -ETIME
- Permission denied → Return -EACCES

### Binding Errors
- Source path not found → Return -ENOENT
- Target path already bound → Return -EEXIST
- Invalid binding operation → Return -EINVAL
- Binding failure → Return -ENOMEM

## Testing Strategy

### Unit Testing
- Test service registration and validation (including hijacking prevention)
- Test KFS namespace traversal and service discovery
- Test binding and unbinding operations
- Test attachment and detachment sequences
- Test authentication challenge/response protocols
- Test capability-based access control

### Property-Based Testing
- Test namespace hijacking prevention with random path combinations
- Test attachment state consistency with random operation sequences
- Test service discovery completeness with random service registrations
- Test authentication integrity with random challenge/response patterns
- Test capability-based access control with random permission combinations
- Test binding isolation with concurrent process operations