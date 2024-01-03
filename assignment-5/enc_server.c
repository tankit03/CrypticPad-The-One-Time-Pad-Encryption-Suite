#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define BUFFER_SIZE 30000
#define MAX_TEXT_LENGTH 1000

// Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to set up the server socket address structure
void setupSocketAddress(struct sockaddr_in *address, int portNumber) {
    memset((char *)address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]) {
    int connectionSocket, charsRead, charsWritten, childStatus;
    char inputBuffer[BUFFER_SIZE];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientInfoSize = sizeof(clientAddress);

    int isConnectionGood = 1;

    if (argc < 2) {
        error("USAGE: ./program_name port");
    }

    // Create a socket to listen for incoming connections
    int listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket < 0) {
        error("ERROR opening socket");
    }

    // Configure the server address structure
    setupSocketAddress(&serverAddress, atoi(argv[1]));

    // Bind the socket to the specified port
    if (bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    // Start listening for incoming connections with a queue of size 5
    listen(listeningSocket, 5);

    while (1) {
        // Accept incoming connection
        connectionSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientInfoSize);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }

        // Fork a child process to handle the client connection
        pid_t childPid;
        childPid = fork();

        switch (childPid) {
            case -1:
                error("fork() failed");
                break;

            case 0: // Child process
                memset(inputBuffer, 0, BUFFER_SIZE);
                char readBuffer[MAX_TEXT_LENGTH];
                int isPrefix = 1;

                // Read data from the client until the "@" character is received
                while (strstr(readBuffer, "@") == NULL) {
                    memset(inputBuffer, 0, BUFFER_SIZE);
                    charsRead = recv(connectionSocket, readBuffer, MAX_TEXT_LENGTH - 1, 0);

                    if (charsRead < 0) {
                        error("ERROR reading from socket");
                    }

                    if (isPrefix) {
                        char pre[2];
                        strncpy(pre, readBuffer, 2);

                        if (strcmp(pre, "E!") != 0) {
                            charsWritten = send(connectionSocket, "0", 1, 0);
                            if (charsWritten < 0) {
                                error("ERROR writing to socket");
                            }
                            isConnectionGood = 0;
                            break;
                        } else {
                            isPrefix = 0;
                        }
                    }

                    strcat(inputBuffer, readBuffer);
                }

                if (isConnectionGood) {
                    // Process the received data
                    inputBuffer[strlen(inputBuffer) - 1] = '\0';
                    char *temp = inputBuffer + 2;
                    memmove(inputBuffer, temp, strlen(inputBuffer));

                    long textLength = strlen(inputBuffer) / 2;
                    char plainText[textLength + 1];
                    char key[textLength + 1];
                    char cipher[textLength + 2];

                    memset(plainText, 0, textLength + 1);
                    memset(key, 0, textLength + 1);
                    memset(cipher, 0, textLength + 2);

                    strncpy(plainText, inputBuffer, textLength);
                    strcpy(key, &inputBuffer[textLength]);

                    for (int i = 0; i < textLength; i++) {
                        int textChar = plainText[i] - 65;
                        int keyChar = key[i] - 65;

                        if (textChar == -33) {
                            textChar = 26;
                        }

                        if (keyChar == -33) {
                            keyChar = 26;
                        }

                        int result = (textChar + keyChar) % 27;

                        if (result == 26) {
                            cipher[i] = 32;
                        } else {
                            cipher[i] = result + 65;
                        }
                    }

                    strcat(cipher, "@");
                    charsWritten = send(connectionSocket, cipher, strlen(cipher), 0);

                    if (charsWritten < 0) {
                        error("ERROR writing to socket");
                    }

                    if (charsWritten < strlen(cipher)) {
                        fprintf(stderr, "SERVER: WARNING: Not all data written to socket!\n");
                    }
                }

                close(connectionSocket);
                exit(0);
                break;

            default: // Parent process
                childPid = waitpid(childPid, &childStatus, WNOHANG);
        }
    }

    // Close the listening socket
    close(listeningSocket);
    return 0;
}
