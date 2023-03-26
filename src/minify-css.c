#include <stdlib.h>
#include <stdio.h>

/**
 * 0 = Default
 * 1 = In String - Non-escaped
 * 2 = In String - Escaped
 * 3 = In Comment Block
 * 4 = In Comment Line
 * 5 = Already placed a Space
*/
static int currentState = 0;
static int escaped = -1;

static int inString;
static int stringDelimiter;
static FILE *original;
static FILE *minified;
static int buffer = -1;

char get() {
    if(buffer == -1) {
        return fgetc(original);
    } else {
        char temp = (char) buffer;
        buffer = -1;
        return temp;
    }
}

char peek() {
    if(buffer == -1) {
        buffer = fgetc(original);
    }

    return (char) buffer;
}

void skip() {
    get();
}

void addCharacter(char c) {
    fputc((int) c, minified);
}

// Change the Parsing State
// Return whether the currently evaluated Character should be irgnored
int changeStates(char character, char lookahead) {
    switch(currentState) {
        case 0:
            if(character == '\"' || character == '\'' || character == '`') {
                // Entering a String
                currentState = 1;
                stringDelimiter = character;
            }

            if(character == '/' && lookahead == '*') {
                // Entering a Comment Block
                currentState = 3;
            }

            if(character == '/' && lookahead == '/') {
                // Entering a Comment Line
                currentState = 4;
            }

            if((character == ' ') && (lookahead == ' ' || lookahead == '\r' || lookahead == '\n')) {
                // Placed a Space
                currentState = 5;
            }
            
            break;
        case 1:
            if(character == stringDelimiter) {
                // Leaving the String
                currentState = 5;
            }

            if(character == '\\') {
                // Escaping the next Character
                currentState = 2;
            }

            break;
        case 2:
            // Leaving the Escaped State
            currentState = 1;
            break;
        case 3:
            if(character == '*' && lookahead == '/') {
                // Leaving the Comment Block
                skip();
                currentState = 5;

                return 1;
            }

            break;
        case 4:
            if(character == '\r' || character == '\n') {
                // Leaving the Comment Line
                currentState = 5;
            }

            break;
        case 5:
            if(lookahead != ' ' && lookahead != '\r' && lookahead != '\n') {
                // Leaving the Placed Space State
                currentState = 0;
            }

            break;
    }

    return 0;
}

void minify() {
    char c;
    while((c = get()) != EOF) {
        // Change the current Parsing State
        if(changeStates(c, peek()) == 1) {
            continue;
        }

        if(currentState == 0) {
            // Place if no Line Break
            if(c == '\r' || c == '\n') {
                continue;
            }


        } else if(currentState == 1) {
            // Place normally
        } else if(currentState == 2) {
            // Place normally
        } else if(currentState == 3) {
            // Don't place
            continue;
        } else if(currentState == 4) {
            // Don't place
            continue;
        } else if(currentState == 5) {
            // Place if no Space or Line Break
            if(c == ' ' || c == '\r' || c == '\n') {
                continue;
            }
        } else {
            // Error
            printf("Error: Unknown State");
            return;
        }

        // Write the current Character to the minified File
        addCharacter(c);

        // Skip a Space after Colons, Doublecolons and Semicolons
        if((c == ',' || c == ':' || c == ';') && (peek() == ' ' || peek() == '{')) {
            skip();
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc == 3) {
        // Open original File
        original = fopen(argv[1], "r");

        // Recreate minified File
        remove(argv[2]);
        minified = fopen(argv[2], "w");
    }

    if(original == NULL || minified == NULL) {
        printf("Error: No such file");
        return -1;
    }

    minify();

    fclose(original);
    fclose(minified);

    printf("Done.");

    return 0;
}