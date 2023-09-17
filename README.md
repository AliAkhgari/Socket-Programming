# Socket-Programming

## Introduction
The goal of this project is to implement a question and answer system for different courses, similar to Stack Overflow, using socket programming and C language system calls. This system allows clients to connect to a server, join specific chat rooms based on their majors, ask and answer questions, and select the best answers.


## Features
- Chat Functionality:
  - Clients can connect to the server over a network.
  - Clients are grouped into chat rooms based on their majors: C, B, O, or M.
  - Clients can ask questions and provide answers to questions asked by others.
  - Clients can select the best answer from provided responses.
- Question and Answer Format:
  - The format of asking a question: client_id#Q:message.
  - The format of answering a question: client_id#client_id_of_questioner?A:message.
  - The format of the best response: client_id#B:best_ans.

## Prerequisites

C Compiler (e.g., Clang or GCC)

## Installation

1. Clone the repository or download the ZIP file.
2. Navigate to the project directory in the terminal.

```bash
cd /path/to/project-directory
```

3. Compile the client and server C programs using a C compiler.
```bash
# Compile the client
gcc -o client client.c
# Compile the server
gcc -o server server.c
```

## Usage

1. Run the server executable.
```bash
./server
```
2. Run multiple client executables in separate terminals.
```bash
./client
```
3. Follow the prompts in the client's terminal to interact with the chat application.