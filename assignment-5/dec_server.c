#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 1000

// Function to handle errors and exit
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to set up the server address structure
void setupServerAddress(struct sockaddr_in *serverAddress, int portNumber) {
    memset((char *)serverAddress, '\0', sizeof(*serverAddress));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(portNumber);
    serverAddress->sin_addr.s_addr = INADDR_ANY;
}

// Function to handle a client connection
void handleClientConnection(int connSocket) {
    int bytesRead, bytesWritten;
    char dataBuffer[MAX_BUFFER_SIZE];
    char socketBuffer[MAX_BUFFER_SIZE];
    int isConnectionActive = 1;
    int isPrefix = 1;

    // Read data from the client
    while (1) {
        memset(socketBuffer, '\0', MAX_BUFFER_SIZE);
        bytesRead = recv(connSocket, socketBuffer, MAX_BUFFER_SIZE - 1, 0);

        if (bytesRead < 0) {
            error("ERROR reading from socket");
        }

        if (isPrefix) {
            char prefix[2];
            strncpy(prefix, socketBuffer, 2);

            if (strcmp(prefix, "D!") != 0) {
                bytesWritten = send(connSocket, "0", 1, 0);

                if (bytesWritten < 0) {
                    error("ERROR writing to socket");
                }

                isConnectionActive = 0;
                break;
            } else {
                isPrefix = 0;
            }
        }

        strcat(dataBuffer, socketBuffer);

        if (strstr(socketBuffer, "@") != NULL) {
            break;
        }
    }

    // Process data and send a response back to the client
    if (isConnectionActive) {
        dataBuffer[strlen(dataBuffer) - 1] = '\0';
        char *temp = dataBuffer + 2;
        memmove(dataBuffer, temp, strlen(dataBuffer));

        long textLength = strlen(dataBuffer) / 2;
        char plaintext[textLength + 1];
        char key[textLength + 1];
        char ciphertext[textLength + 2];

        memset(plaintext, '\0', textLength + 1);
        memset(key, '\0', textLength + 1);
        memset(ciphertext, '\0', textLength + 2);

        strncpy(plaintext, dataBuffer, textLength);
        strcpy(key, &dataBuffer[textLength]);

        for (int i = 0; i < textLength; i++) {
            int textChar = plaintext[i] - 65;
            int keyChar = key[i] - 65;

            if (textChar == -33) {
                textChar = 26;
            }
            if (keyChar == -33) {
                keyChar = 26;
            }

            int cipherKey = textChar - keyChar;
            if (cipherKey < 0) {
                cipherKey += 27;
            }

            int result = cipherKey % 27;
            if (result == 26) {
                ciphertext[i] = 32;
            } else {
                ciphertext[i] = result + 65;
            }
        }

        strcat(ciphertext, "@");
        bytesWritten = send(connSocket, ciphertext, strlen(ciphertext), 0);

        if (bytesWritten < 0) {
            error("ERROR writing to socket");
        }

        if (bytesWritten < strlen(ciphertext)) {
            fprintf(stderr, "SERVER: WARNING: Not all data written to socket!\n");
        }
    }

    // Close the connection
    close(connSocket);
}

int main(int argc, char *argv[]) {
    int listenSocket, childStatus;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientInfoSize = sizeof(clientAddress);

    // Check for correct usage and arguments
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Configure the address structure for the server socket
    setupServerAddress(&serverAddress, atoi(argv[1]));

    // Bind the socket to the specified port
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    // Start listening for incoming connections with a queue of up to 5
    listen(listenSocket, 5);

    // Accept incoming connections and handle them in child processes
    while (1) {
        int connSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &clientInfoSize);

        if (connSocket < 0) {
            error("ERROR on accept");
        }

        pid_t childPid = fork(); // Fork a child process

        switch (childPid) {
            case -1: // Error
                error("fork() failed");
                break;

            case 0:
                // Child process
                handleClientConnection(connSocket);
                exit(0);
                break;

            default:
                // Parent process
                childPid = waitpid(childPid, &childStatus, WNOHANG);
                break;
        }
    }

    // Close the listening socket
    close(listenSocket);
    return 0;
}
