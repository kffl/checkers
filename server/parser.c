#include <string.h>

char parseCommandName(char *s) {
    char move[] = "move;";
    char quit[] = "quit;";
    if (strncmp(move, s, strlen(move)) == 0) 
        return 1;
    if (strncmp(quit, s, strlen(quit)) == 0)
        return 2;
    return 0;
}

char parseMove() {
    
}