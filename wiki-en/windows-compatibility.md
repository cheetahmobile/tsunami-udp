# Windows Compatibility

# Windows Compatibility Module

## Purpose

The Windows Compatibility module provides POSIX thread (pthreads) implementation details specific to Windows systems. It includes internal utilities for managing thread cleanup handlers, attribute handling, condition variables, barriers, and other threading primitives that bridge the gap between POSIX thread semantics and Windows threading APIs.

This module is part of the `pthreads-win32` library which implements POSIX threads on top of Windows Win32 API.

## Key Components

### Thread Cleanup Handlers

#### Functions:
```c
ptw32_cleanup_t *ptw32_pop_cleanup(int execute);
void ptw32_push_cleanup(ptw32_cleanup_t *cleanup, ptw32_cleanup_callback_t routine, void *arg);
```

These functions manage a stack of cleanup handlers associated with each thread. They are used when threads exit or are cancelled to ensure proper resource cleanup.

Cleanup handlers are stored in a linked list using `prev` pointers. The stack is managed through `pthread_getspecific()` and `pthread_setspecific()` using the global cleanup key (`ptw32_cleanupKey`). 

When a cleanup handler is popped, if `execute` is true and the handler exists, it's invoked with the argument passed during registration.

### Attribute Management

#### Structures:
```c
struct pthread_attr_t_
struct pthread_barrierattr_t_
struct pthread_condattr_t_
```

These structures define attributes for various threading objects like threads, barriers, conditions, etc. The attributes are initialized via `pthread_attr_init`, validated by `ptw32_is_attr`, and destroyed by `pthread_attr_destroy`.

### Threading Primitives

#### Condition Variables:
```c
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
```

Condition variable implementation uses two semaphores (`semBlockLock`, `semBlockQueue`) and mutexes (`mtxUnblockLock`) to synchronize waiting threads.

#### Barriers:
```c
int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count);
int pthread_barrier_wait(pthread_barrier_t *barrier);
int pthread_barrier_destroy(pthread_barrier_t *attr);
```

Barriers use alternating semaphores (`semBarrierBreeched[0]`, `semBarrierBreeched[1]`) to coordinate multiple threads at a synchronization point.

#### Mutexes:
```c
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

Mutex implementations include both recursive and normal variants, with support for asynchronous cancellation.

## Internal APIs

### Cleanup Stack Management:

The cleanup mechanism relies on `pthread_getspecific()`/`pthread_setspecific()` with a dedicated key (`ptw32_cleanupKey`) to store per-thread cleanup stacks.

- **`ptw32_pop_cleanup(int execute)`**: Pops the most recently pushed cleanup handler from thread-specific storage.
  - If `execute` is non-zero and handler exists, calls handler routine with stored argument.
  - Returns pointer to previous cleanup handler in stack.

- **`ptw32_push_cleanup(ptw32_cleanup_t *cleanup, ptw32_cleanup_callback_t routine, void *arg)`**: Pushes new cleanup handler onto thread-specific stack.
  - Stores callback function and argument.
  - Links into existing cleanup stack using `prev` field.

### Attribute Validation:

- **`ptw32_is_attr(const pthread_attr_t *attr)`: Validates that an attribute object is valid before use. Returns 0 if valid, otherwise EINVAL.

### Thread Management:

- **`ptw32_threadStart(void *vthreadParms)`**: Entry point for all created threads; manages thread lifecycle including cleanup execution upon exit or cancellation.

- **`ptw32_threadDestroy(pthread_t tid)`**: Cleans up resources associated with a thread when it exits or is destroyed.

### Synchronization Primitives:

- **`ptw32_semwait(sem_t *sem)`**: Non-cancelable wait operation on semaphore (used by condition variables).
- **`ptw32_relmillisecs(const struct timespec *abstime)`**: Calculates relative milliseconds until absolute time expires.

## Execution Flow Diagram

```mermaid
graph TD
    A[pthread_create] --> B[ptw32_threadStart]
    B --> C[ptw32_push_cleanup]
    C --> D[Cleanup Handler Registration]
    B --> E[Thread Body]
    E --> F[ptw32_pop_cleanup]
    F --> G[Execute Handlers]
    G --> H[Resource Cleanup]
    
    subgraph Threading_Layer
        A
        B
        C
        D
        E
        F
        G
        H
    end
    
    I[ptw32_throw] --> J[ptw32_pop_cleanup_all]
    J --> K[ptw32_pop_cleanup]
    K --> L[Cleanup Routine Call]
end
```

## Integration Points

This module integrates with:
1. Core threading functions via `pthread.h`
2. Internal implementation details through `implement.h`
3. Windows API for thread management (`CreateThread`, `SuspendThread`)
4. POSIX-compatible synchronization primitives like mutexes, conditions, semaphores
5. Thread-local storage mechanisms (`pthread_getspecific`, `pthread_setspecific`)
6. Exception handling system (`ptw32_throw`)