# Simple Multi-threaded HTTP Server

A lightweight, multi-threaded HTTP server written in C that serves static files and handles basic routing.

## Features

- Multi-threaded request handling
- Static file serving with proper MIME type detection
- Basic routing system
- Support for different file types (HTML, TXT, JPG, PNG)
- Custom 404 error handling
- Configurable port number via command line

## Prerequisites

- GCC compiler
- POSIX-compliant operating system (Linux/Unix)
- pthread library

## Project Structure

```
.
├── main.c            # Main server implementation
├── socket_defs.h     # Socket initialization and connection handling
├── request_header.h  # Request parsing and route handling
└── public/          # Directory for static files
    ├── home/
    │   └── home.html
    └── not-found/
        └── 404.html
```

## Installation

1. Clone the repository
2. Create the public directory structure:
   ```bash
   mkdir -p public/home public/not-found
   ```
3. Add your static files to the public directory
4. Compile the server:
   ```bash
   gcc main.c -o server -pthread
   ```

## Usage

1. Start the server by providing a port number:
   ```bash
   ./server <port>
   ```
   Example:
   ```bash
   ./server 8080
   ```

2. Access the server through a web browser or HTTP client:
   ```
   http://localhost:8080/
   ```

## Adding Routes

Routes are defined in the `routes` array in `request_header.h`. To add a new route:

1. Create a handler function:
   ```c
   void newroute(int client_fd) {
       // Handle the request
   }
   ```

2. Add the route to the routes array:
   ```c
   struct route routes[] = {
       {"GET", "/home", homeroute},
       {"GET", "/newpath", newroute}
   };
   ```

## Supported MIME Types

- HTML files (`.html`, `.htm`): `text/html`
- Text files (`.txt`): `text/plain`
- JPEG images (`.jpg`, `.jpeg`): `image/jpeg`
- PNG images (`.png`): `image/png`
- Other files: `application/octet-stream`

## Error Handling

- The server provides custom 404 error pages
- Socket errors are logged to stderr
- Connection errors are handled gracefully with continued server operation

## Security Considerations

- The server is bound to `0.0.0.0`, allowing connections from any IP address
- File access is restricted to the `public` directory
- Basic error handling is implemented for file operations

## Limitations

- Supports only GET requests by default
- No built-in security features like SSL/TLS
- Limited to serving static files and basic routes
- No request body parsing
- No query parameter handling


## Learning Points - Simple HTTP Server Project

### Overview
This project demonstrates fundamental concepts in network programming and system design through a practical implementation of a multi-threaded HTTP server.

#### Core Programming Skills
- **C Programming**
  - Working with structs and pointers
  - Dynamic memory allocation and cleanup
  - Header file organization
  - Error handling patterns

#### Networking Fundamentals
- **Socket Programming**
  - BSD socket implementation
  - TCP/IP basics
  - Client-server communication
- **HTTP Protocol**
  - Request parsing
  - Response construction
  - Header handling

#### Multi-threading Concepts
- Thread creation and management with pthreads
- Concurrent request handling
- Resource sharing between threads
- Thread lifecycle management

#### File System Operations
- File reading and serving
- MIME type detection and handling
- Directory structure management
- Basic security through public directory isolation

#### Web Server Concepts
- URL routing implementation
- Static file serving patterns
- HTTP header construction
- Error page handling (404)

#### Key Takeaways
1. Practical understanding of web server internals
2. Real-world application of networking concepts
3. Importance of proper resource management
4. Fundamentals of concurrent programming

#### Target Audience
- Computer Science students
- Aspiring system programmers
- Web developers wanting to understand server internals
- Anyone interested in low-level networking

#### Project Value
This project provides hands-on experience with:
- Building production-like server applications
- Managing system resources effectively
- Handling concurrent network connections
- Implementing basic web server features

#### Next Steps
To extend learning, consider adding:
- HTTPS support
- Dynamic content handling
- Database integration
- Load balancing features
- Advanced security measures
