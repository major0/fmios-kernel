# Design Document

## Overview

The Console subsystem provides unified kernel output management supporting multiple output devices including serial ports, video displays, and debug interfaces. It ensures early availability of debug output and provides consistent logging capabilities across all architectures with proper device abstraction and multiplexing support.

## Architecture

### Console Architecture Overview

```
Kernel Code → kprintf()/klogf() → Console Multiplexer → Console Drivers → Hardware
    ↓              ↓                    ↓                    ↓              ↓
  Log Calls    Format/Buffer        Device Selection      Arch-Specific    Serial/Video
  Debug Msgs   Thread Safety        Priority Handling     Implementation   Debug Output
```

### Multi-Device Support

The console system supports simultaneous output to multiple devices through a priority-based multiplexer:

- **Primary Console**: Selected via boot parameters (console=ttyS0,115200)
- **Secondary Consoles**: Additional outputs for redundancy and debugging
- **Emergency Console**: Fallback output when primary console fails

### Architecture-Specific Drivers

```
arch/x86_64/drivers/serial_console.c    # PC serial ports (COM1, COM2)
arch/x86_64/drivers/video_console.c     # VGA text mode output
arch/arm64/drivers/serial_console.c     # ARM UART devices
arch/riscv64/drivers/serial_console.c   # RISC-V SBI console
```

## Components and Interfaces

### Console Driver Interface

```c
struct console_driver_s {
  const char *name;
  int (*init)(void);
  int (*putchar)(int c);
  int (*puts)(const char *s);
  int (*write)(const void *buf, size_t len);
  void (*flush)(void);
  bool (*ready)(void);
  int priority;
  bool enabled;
} console_driver_t;

// Console driver registration
int console_register_driver(console_driver_t *driver);
int console_unregister_driver(console_driver_t *driver);
```

### Console Multiplexer

```c
struct console_multiplexer_s {
  console_driver_t *drivers[MAX_CONSOLE_DRIVERS];
  size_t driver_count;
  console_driver_t *primary_console;
  console_driver_t *boot_console;
  spinlock_t mux_lock;
  bool early_init_complete;
} console_multiplexer_t;

// Console output functions
int console_putchar(int c);
int console_puts(const char *s);
int console_write(const void *buf, size_t len);
void console_flush(void);
```

### Kernel Logging Interface

```c
enum log_level {
  LOG_DEBUG = 0,
  LOG_INFO = 1,
  LOG_WARN = 2,
  LOG_ERROR = 3,
  LOG_CRITICAL = 4
};

// Kernel logging functions
int kprintf(const char *format, ...);
int klogf(enum log_level level, const char *format, ...);
int kvprintf(const char *format, va_list args);

// Log level management
void klog_set_level(enum log_level min_level);
enum log_level klog_get_level(void);
```

## Data Models

### Console State Management

```c
struct console_state_s {
  console_multiplexer_t multiplexer;
  enum log_level current_log_level;
  char *message_buffer;
  size_t buffer_size;
  size_t buffer_used;
  spinlock_t state_lock;
  bool emergency_mode;
  uint64_t message_count;
  uint64_t error_count;
} console_state_t;
```

### Boot Parameter Processing

```c
struct console_config_s {
  char device_name[32];      // e.g., "ttyS0"
  uint32_t baud_rate;        // e.g., 115200
  uint8_t data_bits;         // e.g., 8
  uint8_t stop_bits;         // e.g., 1
  char parity;               // 'N', 'E', 'O'
  bool flow_control;         // Hardware flow control
} console_config_t;

// Parse console boot parameters
int console_parse_boot_params(const char *cmdline, console_config_t *config);
```

### Message Formatting

```c
struct log_message_s {
  uint64_t timestamp;        // Jiffies since boot
  enum log_level level;
  char message[MAX_LOG_MESSAGE_SIZE];
  size_t message_len;
} log_message_t;

// Format log message with timestamp and level
int format_log_message(log_message_t *msg, enum log_level level,
            const char *format, va_list args);
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Console Output Atomicity
*For any* console output operation, the complete message should be written atomically without interleaving with other concurrent output operations.
**Validates: Requirements 6.4, 6.9**

### Property 2: Console Driver Registration Consistency
*For any* console driver registration, the driver should be properly integrated into the multiplexer and available for output operations.
**Validates: Requirements 5.1, 5.2**

### Property 3: Log Level Filtering Correctness
*For any* log message with a specific level, it should only be output if the current log level setting permits messages of that level or higher.
**Validates: Requirements 4.6, 4.2**

### Property 4: Early Console Availability
*For any* kernel subsystem initialization, console output should be available and functional before the subsystem begins producing debug messages.
**Validates: Requirements 2.1, 2.2**

### Property 5: Console Device Failover Reliability
*For any* primary console device failure, the system should automatically failover to available secondary console devices without losing messages.
**Validates: Requirements 1.8, 2.5**

### Property 6: Message Format Consistency
*For any* formatted log message, the output should contain the correct timestamp, log level, and message content in the specified format.
**Validates: Requirements 4.3, 4.4, 4.5**

### Property 7: Architecture-Specific Driver Compatibility
*For any* supported architecture, the console drivers should provide consistent functionality while utilizing architecture-specific hardware optimally.
**Validates: Requirements 3.7, 3.8**

## Error Handling

### Console Driver Errors
- Driver initialization failure → Attempt fallback drivers and log error
- Hardware communication failure → Switch to emergency mode and retry
- Driver registration conflicts → Resolve by priority and log warning

### Output Buffer Management
- Buffer overflow → Flush buffer and continue with new messages
- Memory allocation failure → Use static emergency buffer
- Format string errors → Output error message and continue

### Early Boot Errors
- No console drivers available → Use emergency output methods
- Console parameter parsing errors → Use default configuration
- Hardware detection failure → Attempt all available drivers

### Runtime Configuration Errors
- Invalid log level setting → Reject change and maintain current level
- Console device disable failure → Log error and maintain current state
- Configuration persistence failure → Use in-memory configuration only

## Testing Strategy

### Unit Testing
- Test console driver registration and deregistration
- Test message formatting with various log levels and formats
- Test console multiplexer device selection and failover
- Test boot parameter parsing for different console configurations

### Property-Based Testing
- Generate random console output and verify atomicity
- Test concurrent console operations for thread safety
- Validate log level filtering with random message streams
- Test console driver failover scenarios with random failures

### Integration Testing
- QEMU testing with different console configurations
- Multi-architecture testing (x86_64, ARM64, RISC-V)
- Boot parameter testing with various console settings
- Hardware compatibility testing on physical systems

### Architecture-Specific Testing
- x86_64: Test serial and VGA console drivers
- ARM64: Test UART console drivers with different platforms
- RISC-V: Test SBI console interface
- Cross-architecture: Verify consistent behavior and output formatting
