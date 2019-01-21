#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct //struktura reprezentująca grę
{
    int player1_socket;
    int player2_socket;
    int move_count; //licznik ruchów
    char player_move; //który gracz obecnie wykonuje ruch
    char board[32]; //plansza
    int board_history_p1[32];
    int board_history_p2[32];
    char last_attack; //czy ostatnio wykonano bicie (jeśli tak to gdzie się ono zakończyło)
    char is_terminated; //flaga informujaca o zakonczeniu gry
    pthread_mutex_t *mutex; //zamek struktury
} game;

//TODO

char getXCoords(char field) {
    if ((field / 4) % 2 == 0) {
        return (field % 4) + 1;
    }
    return (field % 4);
}

char getYCoords(char field) {
    return field/4;
}

char XYtoFieldNum(char x, char y) {
    return y * 4 + (x / 2);
}

void switchPlayers(game *g) {
    if (g->player_move == 1) {
        g->player_move = 2;
    } else if (g->player_move == 2) {
        g->player_move = 1;
    } 
}

char canAttack(game *g, char player, char field) {
    
}

//funkcja wykonująca ruch 0-jeśli nie można wykonać ruchu 1- jeśli wykonano
char makeMove(game *g, char player, char field1, char field2) {
    char players_pawn = g->board[field1];
    if (player != players_pawn && player + 2 != players_pawn) {
        return 0; //to pole nie należy do gracza
    }
    if (g->board[field2] != 0) { //jeśli miejsce docelowe jest zajęte
        return 0;
    }
    if (g->player_move != player) {
        return 0;
    }
    if (g->last_attack != 60) {
        if (canAttack(g, player, g->last_attack)) { //jeżeli gracz może kontynuować bicie to musi to zrobić
            if (g->last_attack != field1) {
                return 0;
            }
        }
    }

    char x1 = getXCoords(field1);
    char x2 = getXCoords(field2);
    char y1 = getYCoords(field1);
    char y2 = getYCoords(field2);
    char pawn = (g->board[field1] - 1) / 2; //0 jeśli pionek 1 jeśli król
    char enemy_pawn = 2;
    char enemy_king = 4;
    if (player == 2) {
        enemy_pawn = 1;
        enemy_king = 3;
    }

    if (y1 - y2 == -1) { //ruch w dół
        if (player == 2 && pawn != 1) {
            return 0;
        }
        if (x1 - x2 == 1 || x2 - x1 == 1) {
            g->board[field1] = 0;
            g->board[field2] = players_pawn;
            switchPlayers(g); //zmiana stron
            return 1;
        }
    }     
    if (y1 - y2 == 1) { //ruch w górę
        if (player == 1 && pawn != 1) {
            return 0;
        }
        if (x1 - x2 == 1 || x2 - x1 == 1) {
            g->board[field1] = 0;
            g->board[field2] = players_pawn;
            switchPlayers(g); //zmiana stron
            return 1;
        }
    }

    if (y1 - y2 == -2) { //bicie w dół
        if (player == 2 && pawn != 1) {
            return 0;
        }
        if (x1 - x2 == 2 || x2 - x1 == 2) {
            char middle_field = XYtoFieldNum(((x1 + x2)/2), (y1 + y2)/2);
            char middle = g->board[middle_field];
            if (middle == enemy_king || middle == enemy_pawn) { //wykonujemy bicie - nie zmieniamy stron
                g->board[field1] = 0;
                g->board[field2] = players_pawn;
                g->board[middle_field] = 0;
                g->last_attack = field2;
                return 1;
            }
        }
    }     
    if (y1 - y2 == 2) { //bicie w górę
        if (player == 1 && pawn != 1) {
            return 0;
        }
        if (x1 - x2 == 2 || x2 - x1 == 2) { 
            char middle_field = XYtoFieldNum(((x1 + x2)/2), (y1 + y2)/2);
            char middle = g->board[middle_field];
            if (middle == enemy_king || middle == enemy_pawn) { //wykonujemy bicie - nie zmieniamy stron
                g->board[field1] = 0;
                g->board[field2] = players_pawn;
                g->board[middle_field] = 0;
                g->last_attack = field2;
                return 1;
            }
        }
    }
    return 0;
}

void playerQuit(game *g, char player) {

}

int serializeGame(game *g, char *buf) {
    char prefix[] = "state;";
    strcpy (buf, prefix);
    char num[16];
    sprintf(num, "%d", g->player_move);
    strcat(buf, num);
    char separator[] = ";";
    strcat(buf, separator);
    sprintf(num, "%d", g->move_count);
    strcat(buf, num);
    strcat(buf, separator);
    for (int i = 0; i < 32; i++) {
        sprintf(num, "%d", g->board[i]);
        strcat(buf, num);
        strcat(buf, separator);
    }
    char newline[] = "\n";
    strcat(buf, newline);
    return strlen(buf);
}

void updateHistory(game *g) {

}