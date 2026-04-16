# Other — semaphoreExamples

# **Other — semaphoreExamples**

This module demonstrates inter-process communication using POSIX semaphores. It contains two programs, `sema` and `semb`, that coordinate through a shared semaphore identified by a fixed key (`1492`). The coordination ensures that `semb` waits until `sema` performs a "V" (signal) operation, allowing sequential execution between processes.

The example uses standard System V IPC functions:
- `semget()` to access or create a semaphore.
- `semctl()` to initialize the semaphore's value.
- `semop()` to perform P/V operations on the semaphore.

These programs are intended as educational examples for understanding semaphore-based synchronization across multiple processes.

## Files

### `sema.c`

A program that performs a "V" (release) operation on a semaphore.

#### Purpose

To signal another process waiting on a semaphore, enabling it to proceed with its work.

#### Key Functions

- `main()`
  - Uses `semget()` to get the semaphore ID.
  - Exits if the semaphore doesn't exist.
  - Sets up a `sembuf` structure to perform a V operation (`sem_op = 1`) on semaphore index `0`.
  - Calls `semop()` to release the semaphore.
  - Reports success or failure.

#### Semaphore Usage

```c
struct sembuf operations[1];
operations[0].sem_num = 0;
operations[0].sem_op = 1;
operations[0].sem_flg = 0;
retval = semop(id, operations, 1);
```

### `semb.c`

A program that performs a "P" (wait) operation on a semaphore.

#### Purpose

To wait for a semaphore to be signaled before proceeding.

#### Key Functions

- `main()`
  - Uses `semget()` to get the semaphore ID.
  - Exits if the semaphore doesn't exist.
  - Sets up a `sembuf` structure to perform a P operation (`sem_op = -1`) on semaphore index `0`.
  - Calls `semop()` to wait on the semaphore.
  - Reports success or failure.

#### Semaphore Usage

```c
struct sembuf operations[1];
operations[0].sem_num = 0;
operations[0].sem_op = -1;
operations[0].sem_flg = 0;
retval = semop(id, operations, 1);
```

### `semabinit.c`

Initializes a semaphore used by both `sema` and `semb`.

#### Purpose

Creates and initializes a semaphore with an initial value of zero. This is typically run once before running either `sema` or `semb`, ensuring they can coordinate properly.

#### Key Functions

- `main()`
  - Uses `semget()` with `IPC_CREAT` to create a new semaphore array.
  - Initializes the semaphore’s value using `semctl()` with `SETVAL`.

#### Initialization Logic

```c
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
} argument;

argument.val = 0;
id = semget(KEY, 1, 0666 | IPC_CREAT);

if( semctl(id, 0, SETVAL, argument) < 0)
{
   fprintf(stderr, "Cannot set semaphore value.\n");
}
else
{
   fprintf(stderr, "Semaphore %d initialized.\n", KEY);
}
```

## Semaphore Behavior

The example uses a single semaphore in an array of size one (`semget(..., 1, ...)`). The semaphore's value starts at 0:

- A **V** (signal) operation increments the semaphore value.
- A **P** (wait) operation decrements it, blocking if the value would go below zero.

This allows `semb` to block until `sema` signals it, enabling synchronization between processes without shared memory or other mechanisms.

## Compilation & Execution

Compile all programs:
```bash
gcc -o sema sema.c
gcc -o semb semb.c
gcc -o semabinit semabinit.c
```

Run initialization first:
```bash
./semabinit
```

Then execute `semb` in background:
```bash
./semb &
```

Finally, run `sema`:
```bash
./sema
```

Expected output:
```
Program semb about to do a P-operation.
Process id is 12345
Successful P-operation by program semb.
Program sema about to do a V-operation.
Successful V-operation by program sema.
```

## Mermaid Diagram: Semaphore Coordination Flow

```mermaid
graph TD
    A[sema] --> B{semget()}
    C[semb] --> D{semget()}
    B -- exists? --> E[semop(V)]
    D -- exists? --> F[semop(P)]
    E --> G[exit]
    F --> H[exit]
    I[semabinit] --> J{semget(IPC_CREAT)}
    J --> K{semctl(SETVAL)}
    K --> L[exit]
```