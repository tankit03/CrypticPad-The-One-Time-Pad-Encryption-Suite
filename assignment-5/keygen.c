#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// string containing the allowed characters pretaining to assignent. Uppercase letters and a space.

#define ALLOWED_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ "

int main(int argc, char* argv[]) {

    // this checks if the user has inputed an amonut, if not it prints usage message returns with a error code.

    if (argc != 2) {
        fprintf(stderr, "Usage: %s keylength\n", argv[0]);
        return 1;
    }



    int i;
    int length = atoi(argv[1]); // Key length, comverting the command-line to an integer using ATOI.
    char key[length + 1]; // is an arry to store the generated key, with an extra space for the \0.
    srand(time(0)); // seeds random number to the current time.

    // this loops generates random characters for the key. The rand() assigns the corresponding character to i to the position in the key.

    for (i = 0; i < length; i++) {
        int letter = rand() % 27;
        key[i] = ALLOWED_CHARACTERS[letter];
    }

    key[length] = '\0'; // add the null terminator to the end of the 'key'.

    printf("%s\n", key); // prints the generated key to the standard output with a newline character.

    return 0;
}


