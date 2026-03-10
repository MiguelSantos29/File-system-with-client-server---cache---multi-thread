# Document Indexing and Search Service

Implementation of a **document indexing and search service** in **C**, based on a **client-server architecture** using **named pipes (FIFOs)**, with support for **persistent metadata storage**, **in-memory caching**, and **concurrent document search**.

Developed for the **Operating Systems** course at the **University of Minho**.

## Overview

This system allows users to index and search text documents stored locally on a Linux system.  
A central server (`dserver`) manages document metadata and processes all requests, while a client (`dclient`) sends commands and displays results.

Main capabilities:
- document metadata indexing
- metadata consultation
- index removal
- keyword-based line counting
- keyword search across indexed documents
- concurrent search execution
- persistent metadata storage
- configurable metadata cache

## Project Structure

The repository is organized as follows:

- **src/** – main source code for the server and client
- **include/** – header files used by the project
- **obj/** – compiled object files
- **bin/** – compiled executables
- **cache/** – cache implementation and related logic
- **indice/** – document indexing structures
- **dataset/** – documents used for testing and indexing
- **scripts/** – helper scripts used for running experiments or tests
- **Makefile** – build configuration for compiling the project

This structure separates compilation artifacts, source code, datasets and support tools to keep the project organized.
## Documentation

The repository includes a detailed project report that analyzes the system design and implementation.  
The report also evaluates the performance of the system under different usage scenarios, comparing various cache configurations and execution strategies. These experiments measure how different workloads and cache sizes affect performance, allowing a better understanding of the trade-offs between caching, concurrency, and overall system efficiency.

## System Architecture

The system is composed of two main programs.

### dserver
Responsible for:
- storing and managing document metadata
- processing client requests
- performing document searches
- maintaining persistent indexed data
- managing the in-memory metadata cache

### dclient
A non-interactive command-line client that sends a single request to the server and prints the response.

## Communication

Communication between the client and server is implemented using **named pipes (FIFOs)**.  
The client sends a request through the pipe, the server processes it, and returns the result.

## Build

Compile the project with:

`make`

Clean compiled files with:

`make clean`

## Running the Server

Start the server with:

`./dserver document_folder cache_size`

Example:

`./dserver dataset 10`

Where:
- `document_folder` is the directory containing the documents
- `cache_size` defines the maximum number of metadata entries kept in memory

## Client Commands

Add a document to the index:

`./dclient -a "title" "authors" "year" "path"`

Consult document metadata:

`./dclient -c "key"`

Delete indexed metadata:

`./dclient -d "key"`

Count lines containing a keyword in a document:

`./dclient -l "key" "keyword"`

Search indexed documents containing a keyword:

`./dclient -s "keyword"`

Search indexed documents using multiple processes:

`./dclient -s "keyword" "nr_processes"`

Shutdown the server:

`./dclient -f`

## Technologies

- C
- Linux
- system calls
- Named pipes (FIFOs)
- Process management
- Low-level file I/O
- Multi threading
- Makefile


## Author

Miguel Santos

## Academic Context

Operating Systems  
Software Engineering / Computer Engineering  
University of Minho
