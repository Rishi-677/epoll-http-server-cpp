# High-Performance HTTP Server (C++)

A lightweight HTTP server built in C++ using epoll, non-blocking sockets, and a thread pool to demonstrate low-level networking, concurrency, and backend system design.

---

## Features

- Uses Linux `epoll` for scalable, event‑driven I/O handling.
- Supports non‑blocking sockets for efficient concurrent connections.
- Implements a thread pool to process requests in parallel.
- Provides a modular routing system for handling endpoints.
- Supports middleware execution before routing.
- Includes request logging middleware.
- Implements IP‑based rate limiting.
- Tracks runtime metrics such as total requests and latency.
- Exposes a `/metrics` endpoint for observability.
- Handles graceful shutdown using OS signals.
- Uses atomic counters for thread‑safe statistics.
- Designed with clean separation of concerns.

---

## How the Server Works

The server starts by creating a non‑blocking socket and registers it with `epoll`.  
The main thread continuously waits for socket events and accepts new client connections.

Incoming connections are pushed into a task queue, which is processed by a pool of worker threads.  
Each worker parses the HTTP request, runs middleware, and then forwards the request to the router.

Middleware executes logic such as logging and rate limiting before the route handler runs.  
The router generates the response, which is written back to the client socket.

During request handling, metrics such as request count, active connections, and latency are recorded using atomic counters. The server supports graceful shutdown by stopping new accepts and allowing in‑flight requests to finish cleanly.

---

## How to Run the Project

### Build the Server

Copy and run the following command:

```bash
g++ src/main.cpp \src/routes.cpp \src/middleware.cpp \src/middleware_setup.cpp \src/metrics.cpp \-o server -pthread
```

### Run the Server

```bash
./server
```

### Test the Server

```bash
curl http://localhost:2025
curl http://localhost:2025/hello
curl http://localhost:2025/metrics
curl http://localhost:2025/unknown
```

### Stop the Server

```bash
Ctrl + C
```

The server shuts down gracefully, completing active requests before exiting.