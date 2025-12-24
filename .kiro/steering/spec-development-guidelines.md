# Spec Development Guidelines

This document establishes the guidelines and requirements for creating and maintaining specification documents in the FMI/OS project.

## Core Principles

1. **Focus on Requirements and Design**: Specs should focus on what needs to be built and how, not when
2. **Avoid Time Estimates**: Time estimation is inherently unreliable and creates false expectations
3. **Prioritize by Dependencies**: Task ordering should be based on technical dependencies, not arbitrary time estimates
4. **Iterative Development**: Specs should support iterative development without time pressure

## Task List Requirements

### Prohibited Elements

**NEVER include the following in task lists:**

1. **Estimated Effort Information**: 
   - No "Estimated Effort: X days/hours/weeks"
   - No time-based estimates of any kind
   - No duration predictions or scheduling information

2. **Time-Based Priorities**:
   - No priorities based on estimated completion time
   - No "quick wins" or "low-hanging fruit" classifications based on time

3. **Deadline References**:
   - No target completion dates
   - No milestone timing information
   - No schedule-driven task ordering

### Required Elements

**ALWAYS include the following in task lists:**

1. **Clear Task Descriptions**:
   - Specific, actionable task descriptions
   - Clear objectives and deliverables
   - Technical requirements and constraints

2. **Dependency-Based Ordering**:
   - Tasks ordered by technical dependencies
   - Clear prerequisite relationships
   - Logical progression from foundation to implementation

3. **Priority Classifications**:
   - Priority based on technical importance (CRITICAL, HIGH, MEDIUM, LOW)
   - Priority based on blocking relationships
   - Priority based on system stability and safety requirements

4. **Requirement Traceability**:
   - Each task references specific requirements
   - Clear mapping between tasks and acceptance criteria
   - Validation that all requirements are covered

## Task Format Standards

### Correct Task Format

```markdown
- [ ] 1. Task Name
**Priority: [CRITICAL/HIGH/MEDIUM/LOW] - [Technical justification]**
**Dependencies: [List of prerequisite tasks or "None"]**

[Detailed task description with specific deliverables]

- [Specific sub-task or deliverable]
- [Another specific sub-task or deliverable]
- [Technical requirements and constraints]
- _Requirements: [List of requirement IDs]_

- [ ]* 1.1 Optional sub-task
  - [Optional task details]
  - _Requirements: [Requirement IDs]_
```

### Incorrect Task Format

```markdown
❌ NEVER DO THIS:
- [ ] 1. Task Name
**Priority: HIGH**
**Dependencies: Task 2**
**Estimated Effort: 2 days**  ← PROHIBITED

[Task description]
```

## Priority Classification Guidelines

### Priority Levels

1. **CRITICAL**: 
   - System safety and security requirements
   - Blocking dependencies for multiple other tasks
   - Core functionality without which the system cannot operate

2. **HIGH**:
   - Important functionality required for system operation
   - Dependencies for other high-priority tasks
   - Significant impact on system reliability or performance

3. **MEDIUM**:
   - Important features that enhance system functionality
   - Dependencies for lower-priority tasks
   - Moderate impact on system usability

4. **LOW**:
   - Nice-to-have features
   - No blocking dependencies
   - Minimal impact on core system operation

### Priority Justification

Each priority level MUST include a technical justification:

- **CRITICAL SAFETY**: Prevents memory corruption, security vulnerabilities
- **CRITICAL BLOCKER**: Required for all subsequent development
- **HIGH DEPENDENCY**: Required by multiple other high-priority tasks
- **HIGH FUNCTIONALITY**: Core system functionality
- **MEDIUM ENHANCEMENT**: Improves system capabilities
- **LOW OPTIONAL**: Optional improvements

## Dependency Management

### Dependency Types

1. **Technical Dependencies**: Task A must be completed before Task B can begin
2. **Logical Dependencies**: Task A provides foundation knowledge for Task B
3. **Testing Dependencies**: Task A provides testing infrastructure for Task B
4. **Resource Dependencies**: Task A and Task B cannot be done simultaneously

### Dependency Documentation

Dependencies MUST be clearly documented:

```markdown
**Dependencies: Task 1 (foundation), Task 3 (testing infrastructure)**
**Dependencies: None**
**Dependencies: Tasks 1, 2 (both required for implementation)**
```

## Testing Integration

### Test-Driven Development

All specs MUST follow test-driven development principles:

1. **Tests First**: Testing tasks should be defined alongside implementation tasks
2. **Property-Based Testing**: Include property-based tests for universal correctness
3. **Unit Testing**: Include unit tests for specific examples and edge cases
4. **Integration Testing**: Include integration tests for cross-component functionality

### Testing Task Format

```markdown
- [ ]* 1.1 Write property test for [specific property]
  - **Property N: [Property Name]**
  - **Validates: Requirements X.Y, X.Z**

- [ ]* 1.2 Write unit tests for [specific functionality]
  - Test [specific behavior]
  - Test [edge cases]
  - _Requirements: X.Y_
```

## Spec Review Process

### Review Checklist

Before finalizing any spec, verify:

- [ ] No estimated effort information anywhere in the document
- [ ] All tasks have clear technical priorities with justifications
- [ ] Dependencies are clearly documented and logical
- [ ] All requirements are covered by implementation tasks
- [ ] Testing tasks are included for all functionality
- [ ] Task ordering follows dependency relationships, not time estimates

### Compliance Enforcement

All spec documents MUST be reviewed for compliance with these guidelines:

1. **Automated Scanning**: Build system should scan for prohibited time estimates
2. **Manual Review**: All specs require manual review for compliance
3. **Rejection Criteria**: Specs with time estimates must be rejected and revised

## Rationale

### Why No Time Estimates?

1. **Inherent Unreliability**: Software time estimates are notoriously inaccurate
2. **False Expectations**: Time estimates create unrealistic expectations and pressure
3. **Focus Distraction**: Time pressure detracts from quality and correctness
4. **Dependency Clarity**: Technical dependencies are more important than time estimates
5. **Iterative Development**: Agile development works better without rigid time constraints

### Benefits of This Approach

1. **Quality Focus**: Emphasis on correctness and completeness over speed
2. **Realistic Planning**: Dependencies provide realistic task ordering
3. **Reduced Pressure**: No artificial time pressure on developers
4. **Better Estimates**: Actual completion data provides better future planning
5. **Flexibility**: Easy to adjust priorities based on changing requirements

## Migration Guidelines

### Updating Existing Specs

When updating existing specs that contain time estimates:

1. **Remove All Time Information**: Delete estimated effort, duration, and timing data
2. **Strengthen Dependencies**: Clarify and strengthen dependency relationships
3. **Improve Priorities**: Add technical justifications for all priority levels
4. **Add Missing Tests**: Ensure comprehensive testing coverage
5. **Verify Requirements**: Confirm all requirements are properly covered

### Example Migration

```markdown
❌ Before:
- [ ] 1. Implement Feature X
**Priority: HIGH**
**Dependencies: Task 2**
**Estimated Effort: 3 days**

✅ After:
- [ ] 1. Implement Feature X
**Priority: HIGH FUNCTIONALITY - Core system operation requirement**
**Dependencies: Task 2 (provides required foundation interface)**
```

This approach ensures that FMI/OS specs focus on technical excellence and logical development progression rather than arbitrary time constraints, leading to higher quality software and more realistic development processes.