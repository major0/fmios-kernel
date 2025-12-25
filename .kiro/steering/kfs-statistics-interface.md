---
inclusion: always
---

# KFS Statistics Interface Standard

All kernel subsystems must provide statistics through the Kernel Filesystem (KFS) using a standardized interface format. This ensures consistent monitoring capabilities and forward-compatible tooling across all subsystems.

## Statistics Organization

### Hierarchical Structure
Statistics are organized hierarchically: `//kern/<subsystem>/stats/<category>` for specific statistic categories.

### Header/Data Format
All statistics files use a consistent header/data format:
- **First line**: Field names separated by spaces
- **Second line**: Corresponding values separated by spaces
- **Format**: `field1 field2 field3\nvalue1 value2 value3\n`

### Forward Compatibility
The statistics interface is forward-portable to field changes:
- New fields can be added without breaking existing parsers
- Parsers should handle additional fields gracefully
- Field order should remain stable when possible

## Standard Statistics Categories

### Required Categories for All Subsystems
1. **counts** - Object/entity count statistics
2. **operations** - Operation frequency and performance statistics
3. **errors** - Error rates and failure statistics
4. **resources** - Resource utilization and capacity statistics

### Example Statistics Structure
```
//kern/<subsystem>/stats/
├── counts          # Entity counts (total, active, inactive)
├── operations      # Operation statistics (rate, latency, throughput)
├── errors          # Error statistics (count, rate, types)
└── resources       # Resource usage (memory, handles, capacity)
```

## Implementation Requirements

### Atomic Operations
- Statistics updates must be atomic to prevent torn reads
- Statistics reads must provide consistent snapshots
- Use appropriate synchronization primitives for statistics access

### Performance Considerations
- Statistics collection should have minimal performance impact
- Use efficient data structures for statistics storage
- Consider using per-CPU statistics with aggregation for high-frequency updates

### Access Control
- Statistics files are readable through standard filesystem operations
- Integrate with capability system for access control
- Provide appropriate permissions for monitoring tools

## Subsystem-Specific Requirements

Each subsystem must implement statistics following this standard:

### Memory Subsystem Example
```
//kern/mem/stats/physical    # total free used cached
//kern/mem/stats/virtual     # total allocated reserved
//kern/mem/stats/system      # allocations faults swapping
//kern/mem/stats/pools       # pool_count utilization fragmentation
```

### Process Subsystem Example
```
//kern/proc/stats/counts     # total running sleeping zombie
//kern/proc/stats/creation   # forks exits failures
//kern/proc/stats/scheduling # context_switches preemptions
```

### Thread Subsystem Example
```
//kern/thread/stats/counts     # total running blocked
//kern/thread/stats/scheduling # quantum_usage priority_changes
```

### IPC Subsystem Example
```
//kern/ipc/stats/messages # sent received queued
//kern/ipc/stats/channels # active created destroyed
```

### Namespace Subsystem Example
```
//kern/ns/stats/entries    # total services directories
//kern/ns/stats/operations # lookups registrations bindings
```

## Tooling Integration

### Monitoring Tools
- Tools can parse statistics programmatically using the header/data format
- Tools decide display format and presentation based on field names
- Tools can handle new fields automatically for forward compatibility

### Historical Data
- Statistics interface supports historical data collection
- External tools can sample statistics over time
- Consider providing rate calculations and trend analysis

This standardized statistics interface ensures consistent monitoring capabilities across all FMI/OS kernel subsystems while maintaining forward compatibility and performance.
