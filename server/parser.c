#include <string.h>
#include <stdio.h>

char parseCommandName(char *s) {
    char move[] = "move;";
    char quit[] = "quit;";
    if (strncmp(move, s, strlen(move)) == 0) 
        return 1;
    if (strncmp(quit, s, strlen(quit)) == 0)
        return 2;
    return 0;
}

void parseMove(char *s, char *pos1, char *pos2) {
    int n = strlen(s);
    char buf[20];
    int l = 0;
    int i;
    for (i = 5; i < n; i++) {
        if (s[i] == ';') {
            break;
        } else {
            buf[l++] = s[i];
        }
    }
    buf[l] = '\0';
    sscanf(buf, "%d", pos1);
    int j = i;
    l = 0;
    for (i = j; i < n; i++) {
        if (s[i] == ';') {
            break;
        } else {
            buf[l++] = s[i];
        }
    }
    buf[l] = '\0';
    sscanf(buf, "%d", pos2);
}