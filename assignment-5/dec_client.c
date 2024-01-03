#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 1000

// Function to handle errors and exit
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to set up the server address structure
void setupServerAddress(struct sockaddr_in *serverAddress, int portNumber, char *hostname) {
    memset((char *)serverAddress, '\0', sizeof(*serverAddress));
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(portNumber);

    struct hostent *hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        error("CLIENT: ERROR, no such host");
    }
    memcpy((char *)&serverAddress->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

// Function to read text from a file
char* readTextFromFile(const char* filename, long* length) {
    FILE *file = fopen(filename, "r");
    char *text;
    *length = 0;

    if (file == NULL) {
        fprintf(stderr, "CLIENT: ERROR opening text file %s\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);

    text = (char *)malloc(sizeof(char) * (*length + 1));
    fgets(text, *length + 1, file);
    fclose(file);

    if (text) {
        text[*length - 1] = '\0';
        (*length)--;
    }

    return text;
}

int main(int argc, char *argv[]) {
    long input_length, key_length, output_length;
    char *input, *key, *output_buff;
    int socketFD, charsWritten, charsRead;
    struct sockaddr_in serverAddress;

    // Check for correct usage & arguments
    if (argc < 4) {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(1);
    }

    // Read text from input file
    input = readTextFromFile(argv[1], &input_length);

    // Read key from key file
    key = readTextFromFile(argv[2], &key_length);

    if (key_length < input_length) {
        fprintf(stderr, "ERROR: Key '%s' is too short\n", argv[2]);
        exit(1);
    }

    // Allocate memory for the output buffer
    output_length = (input_length * 2) + 4;
    output_buff = (char *)malloc(sizeof(char) * output_length);

    // Prepare the output buffer
    strcpy(output_buff, "D!");
    strcat(output_buff, input);
    strncat(output_buff, key, input_length);
    strcat(output_buff, "@");

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupServerAddress(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting socket");
    }

    // Send message to server
    charsWritten = send(socketFD, output_buff, strlen(output_buff), 0);
    if (charsWritten < 0) {
        error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(output_buff)) {
        fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
    }

    // Clear out the buffer array
    memset(output_buff, '\0', output_length);
    char read_buff[MAX_BUFFER_SIZE];

    while (strstr(read_buff, "@") == NULL) {
        memset(read_buff, '\0', MAX_BUFFER_SIZE);
        charsRead = recv(socketFD, read_buff, MAX_BUFFER_SIZE - 1, 0);
        if (charsRead < 0) {
            error("CLIENT: ERROR reading from socket");
        }
        if (strstr(read_buff, "0")) {
            fprintf(stderr, "Error: could not contact dec_server on port %s\n", argv[3]);
            exit(2);
        }
        strcat(output_buff, read_buff);
        if (strstr(output_buff, "0")) {
            break;
        }
    }

    output_buff[strlen(output_buff) - 1] = '\n';
    fprintf(stdout, "%s", output_buff);

    // Close the socket
    close(socketFD);

    // Free allocated memory
    free(input);
    free(key);
    free(output_buff);

    return 0;
}
