#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define MAX 1000

// Function to display error messages and exit
void showError(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to configure the server address structure
void configureAddress(struct sockaddr_in *address, int portNumber, char *hostname) {
    memset((char *)address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(portNumber);
    struct hostent *hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    memcpy((char *)&address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

int main(int argc, char *argv[]) {
    FILE *filePtr = fopen(argv[1], "r");
    char *inputText;
    long inputLength;

    if (filePtr == NULL) {
        showError("CLIENT: ERROR opening text file");
        exit(1);
    } else {
        // Get the length of the input text file
        fseek(filePtr, 0, SEEK_END);
        inputLength = ftell(filePtr);
        fseek(filePtr, 0, SEEK_SET);

        if (inputLength) {
            // Allocate memory for the input text buffer
            inputText = malloc(sizeof *inputText * (inputLength + 1));
            // Read the contents of the input file into inputText
            fgets(inputText, sizeof *inputText * (inputLength + 1), filePtr);
        }
        fclose(filePtr);
    }

    if (inputText) {
        // Remove the newline character at the end, if present
        inputText[inputLength - 1] = '\0';
        inputLength -= 1;
    }

    for (int i = 0; i < inputLength; i++) {
        int charRep = inputText[i];
        // Check if input contains valid characters (A-Z or space)
        if ((charRep < 65 && charRep != 32) || charRep > 90) {
            showError("CLIENT: ERROR: input contains invalid characters");
            exit(1);
        }
    }

    FILE *keyFilePtr = fopen(argv[2], "r");
    char *encryptionKey;
    long keyLength;

    if (keyFilePtr == NULL) {
        showError("CLIENT: ERROR opening key file");
        exit(1);
    } else {
        // Get the length of the key file
        fseek(keyFilePtr, 0, SEEK_END);
        keyLength = ftell(keyFilePtr);
        fseek(keyFilePtr, 0, SEEK_SET);

        if (keyLength) {
            // Allocate memory for the encryption key buffer
            encryptionKey = malloc(sizeof *encryptionKey * (keyLength + 1));
            // Read the contents of the key file into encryptionKey
            fgets(encryptionKey, sizeof *encryptionKey * (keyLength + 1), keyFilePtr);
        }
        fclose(keyFilePtr);
    }

    if (encryptionKey) {
        // Remove the newline character at the end, if present
        encryptionKey[keyLength - 1] = '\0';
        keyLength -= 1;
    }

    if (keyLength < inputLength) {
        showError("CLIENT: ERROR: encryption key is too short");
        exit(1);
    }

    // Calculate the output buffer size
    long outputBufferSize = (inputLength * 2) + 4;
    char *outputBuffer = malloc(outputBufferSize);

    // Prepare the output buffer with encryption instructions and data
    strcpy(outputBuffer, "E!");
    strcat(outputBuffer, inputText);
    strncat(outputBuffer, encryptionKey, inputLength);
    strcat(outputBuffer, "@");

    int socketFD, charsWritten, charsRead;
    struct sockaddr_in serverAddress;

    if (argc < 4) {
        fprintf(stderr, "USAGE: %s plaintext key enc_port\n", argv[0]);
        exit(0);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        showError("CLIENT: ERROR opening socket");
    }

    // Configure the server address
    configureAddress(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to the server
    if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        showError("CLIENT: ERROR connecting socket");
    }

    // Send the message to the server
    charsWritten = send(socketFD, outputBuffer, strlen(outputBuffer), 0);
    if (charsWritten < 0) {
        showError("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(outputBuffer)) {
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    memset(outputBuffer, 0, outputBufferSize);
    char readBuffer[MAX];

    while (strstr(readBuffer, "@") == NULL) {
        memset(readBuffer, 0, MAX);
        charsRead = recv(socketFD, readBuffer, MAX - 1, 0);
        if (charsRead < 0) {
            showError("CLIENT: ERROR reading from socket");
        }
        if (strstr(readBuffer, "0")) {
            fprintf(stderr, "Error: could not contact enc_server on port %s\n", argv[3]);
            exit(2);
        }
        strcat(outputBuffer, readBuffer);
    }
    outputBuffer[strlen(outputBuffer) - 1] = '\n';
    fprintf(stdout, "%s", outputBuffer);

    // Close the socket
    close(socketFD);
    return 0;
}
