# Project Documentation: ATM Simulator - "The Limitless Bank"

Author: Szymon Król

## 1. Introduction

The project is a simulation of a banking system based on a client-server architecture. The application allows users to remotely manage their bank accounts (logging in, checking balances, depositing, withdrawing, and transferring funds). The system was designed to securely handle multiple clients simultaneously while ensuring the consistency of the database stored in binary files.

## 2. Architecture and Employed Mechanisms

### Network Communication (Sockets)

The foundation of communication between the client and the server is network sockets.

- **In the `server.h`/`server.c` file**: The server creates a socket, binds it to a port, and enters the listening state.
- **In the `client.c` file**: The client establishes a connection with the server using the `connect` function.
- **Protocol**: The implementation in `communication.c` defines a custom method of data transmission. Each message is preceded by its size (`ssize_t`), which allows for sending dynamic data structures and variable-length strings.

### Multithreading (Pthreads)

The server is multithreaded, enabling parallel handling of multiple clients.

- **Thread creation**: Each new connection results in the creation of a new thread, which delegates session handling to the `connection_handler` function.

### Synchronization and Data Security

Due to concurrent access to database files, locking mechanisms have been implemented:

- **Mutexes**: The global active connections counter (`active_threads`) is protected using a `pthread_mutex_t`, which prevents race conditions when monitoring the server state.
- **Record locking**: The `fcntl` function is utilized in the `db_manager.c` module. The system locks specific byte ranges (records), allowing multiple users to operate on the database simultaneously, provided they are not modifying the same account.
- **Deadlock avoidance**: During transfer operations, where locking two accounts simultaneously is required, the system always acquires locks in the order of ascending account identifiers.

### File Management and Data Persistence

The system stores data in binary files (`.db`).

- **I/O Operations**: Low-level system functions such as `open`, `close`, `pread`, and `pwrite` are used.
- **Structure management**: Files feature a header storing the number of records, allowing for rapid calculation of offsets for specific user IDs.

### Signal Handling

The application has been equipped with a Graceful Shutdown mechanism.

- **SIGINT**: The server intercepts the interrupt signal (Ctrl+C) using `sigaction`.
- Worker threads have `SIGINT` signal handling blocked (`pthread_sigmask`), so that only the main thread controls the shutdown process, waiting for all active sessions to terminate before finally exiting the program.

### Dynamic Memory Management

Dynamic memory allocation (`malloc`, `free`) is used throughout the project (especially in the communication and database access logic). This allows for flexible loading of user records and receiving network messages of arbitrary size without the risk of buffer overflows.

## 3. How to Use

### Building the Project

```bash
make
```

### Running the Server

```bash
./TheLimitlessBankSimulator server
```

The server starts on port `8080`. To stop it, press `Ctrl+C` — the server will wait for all active sessions to close before shutting down.

### Running the Client

In a separate terminal:

```bash
./TheLimitlessBankSimulator client
```

### Available Operations

The following options are available after logging in:

| Option | Description       |
|--------|-------------------|
| `1`    | Check balance     |
| `2`    | Deposit funds     |
| `3`    | Withdraw funds    |
| `4`    | Transfer funds    |
| `0`    | Exit              |
