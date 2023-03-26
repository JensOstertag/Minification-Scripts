#include <stdlib.h>
#include <stdio.h>

static int inString;
static char stringDelimiters[];
static int inCommentBlock;
static int inCommentLine;
static int placedSpace;
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

    char c;
    while((c = get()) != EOF) {
        if(inCommentBlock == 1 && c == '*' && peek() == '/') {
            // Finished Comment Block
            inCommentBlock = 0;
        }

        if(inCommentLine == 1 && (c == '\n' || c == '\r')) {
            // Finished Comment Line
            inCommentLine = 0;
        }

        // All Cases where the current Character is ignored
        if(inCommentBlock == 1 || inCommentLine == 1) {
            // Ignoring Comments
            continue;
        }

        if(!(inString) && c == '/' && peek() == '*') {
            // Entering a Comment Block
            inCommentBlock = 1;
            continue;
        }

        if(!(inString) && c == '/' && peek() == '/') {
            // Entering Comment Line
            inCommentLine = 1;
            continue;
        }

        if(!(inString) && c == '*' && peek() == '/') {
            // Ignore closed Comment Block
            skip();
            continue;
        }

        if(!(inString) && (c == '\n' || c == '\r')) {
            // Ignore Line Endings except in Strings
            placedSpace = 1;
            continue;
        }

        if(!(inString) && placedSpace == 1 && c == ' ') {
            // Ignore concurrent Spaces
            continue;
        }

        // Only place one Space
        if(!(inString) && placedSpace == 1 && c != ' ') {
            placedSpace = 0;
        } else if(!(inString) && placedSpace == 0 && c == ' ') {
            placedSpace = 1;
        }

        // Write the current Character to the minified File
        addCharacter(c);

        // All Cases where at least the next Character is ignored
        if(!(inString) && (c == ':' || c == ';' || c == ',') && peek() == ' ') {
            // Ignore Spaces after Doublecolons, Semicolons and Commas
            placedSpace = 1;
        }
    }

    fclose(original);

    printf("Done.");

    return 0;
}