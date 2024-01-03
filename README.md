# CrypticPad-The-One-Time-Pad-Encryption-Suite

## Introduction

This README provides instructions for compiling and running a suite of five small programs that collectively implement a one-time pad-like encryption system. These programs utilize multi-processing and socket-based inter-process communication, and are accessible via standard Unix command line features.

## Learning Outcomes

Upon successful completion of this assignment, you should be able to:

- Understand and compare IPC facilities for communication (Module 7, MLO 2).
- Explain the Client-Server communication model (Module 8, MLO 1).
- Design network programs using the programmer's view of the internet (Module 8, MLO 3).
- Understand the concept and application of Unix sockets (Module 8, MLO 4).
- Implement client and server programs for IPC using sockets (Module 8, MLO 5).
- Evaluate different server designs (Module 8, MLO 6).

## One-Time Pads

Refer to the Wikipedia page on One-Time Pads as the primary reference.

## Program Specifications

### 1. `enc_server`

- Functions as an encryption server daemon.
- Listens on a specified port/socket.
- Accepts connections and spawns child processes for handling client requests.
- Communicates with `enc_client` for encryption tasks.
- Supports up to five concurrent socket connections.
- Syntax: `enc_server listening_port`

### 2. `enc_client`

- Connects to `enc_server` for encryption.
- Syntax: `enc_client plaintext key port`
- Outputs ciphertext to stdout.

### 3. `dec_server`

- Functions similarly to `enc_server` but for decryption.
- Communicates with `dec_client` to convert ciphertext back to plaintext.

### 4. `dec_client`

- Connects to `dec_server` for decryption.
- Works similarly to `enc_client`.
- Syntax: `dec_client ciphertext key port`

### 5. `keygen`

- Generates a key file of specified length.
- Characters include 26 capital letters and space.
- Syntax: `keygen keylength`
- Outputs key to stdout.

## Compilation and Execution

### Compilation Script

- You must write or use the provided compilation script to compile all five programs.
- This script should generate the appropriate executable files for each program.

### Running the Programs

1. Start the server programs (`enc_server` and `dec_server`) on your chosen ports.
2. Use `enc_client` and `dec_client` to encrypt and decrypt messages, respectively.
3. Generate keys using `keygen` as required.
4. Follow the specific syntax for each program as described above.

## Testing

- Utilize the provided plaintext files and grading script (`p5testscript`) to test your programs.
- Ensure to change port numbers between test runs to avoid port conflicts.

## Additional Notes

- Pay careful attention to error handling and output specifications for each program.
- Ensure strict adherence to the one-time pad encryption methodology.
- Remember to handle networking errors and bad inputs gracefully.
