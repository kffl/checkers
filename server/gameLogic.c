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

char getXCoords(char field) { //wsp. x z numeru pola
    if ((field / 4) % 2 == 0) {
	        return (field % 4) * 2 + 1;
	    }
	    return (field % 4) * 2;
}

char getYCoords(char field) { //wsp. y z numeru pola
    return field/4;
}

char XYtoFieldNum(char x, char y) { //x, y do numeru pola
    return y * 4 + (x / 2);
}

void switchPlayers(game *g) {
    if (g->player_move == 1) {
        g->player_move = 2;
    } else if (g->player_move == 2) {
        g->player_move = 1;
    } 
}

char areEnemies(game *g, char x1, char y1, char x2, char y2) {
    char state1 = g->board[XYtoFieldNum(x1, y1)];
    char state2 = g->board[XYtoFieldNum(x2, y2)];
    if ((state1 == 1 || state2 == 2) 
            && (state2 == 3 || state2 == 4)) {
        return 1;
    }
    if ((state1 == 3 || state1 == 4) 
            && (state2 == 1 || state2 == 2)) {
        return 1;
    }
    return 0;
}

char canGoDown(game *g, char field) {
    char state = g->board[field];
    if (state == 1 || state == 2 || state == 4)
		return 1;
	return 0;
}

char canGoUp(game *g, char field) {
    char state = g->board[field];
    if (state == 2 || state == 3 || state == 4)
		return 1;
	return 0;
}

char getStateByCoords(game *g, char x, char y) {
    return g->board[XYtoFieldNum(x, y)];
}

char isMovePossible(game *g, char x1, char y1, char x2, char y2, char perform) { //czy ruch z (x1, y1) na (x2, y2) jest dozwolony
    if (x2 < 0 || x2 >= 8 || y2 < 0 || y2 >= 8)
        return 0;
    char fieldNum = XYtoFieldNum(x1, y1);
    char fieldNum2 = XYtoFieldNum(x2, y2);
    char pawn = g->board[fieldNum];
    if (y1 < y2) {
        if (!canGoDown(g, fieldNum)) {
            return 0;
        }
    } else {
        if (!canGoUp(g, fieldNum)) {
            return 0;
        }
    }
    if (abs(x1 - x2) == 1 && abs(y1 - y2) == 1) {
        if (getStateByCoords(g, x2, y2) == 0) {
            if (perform) {
                g->board[fieldNum] = 0;
                g->board[fieldNum2] = pawn;
            }
            return 1; //wykonano/możliwy do wykonania ruch
        }
    }
    if (abs(x1 - x2) == 2 && abs(y1 - y2) == 2) {
        int midx = (x1+x2) / 2;
        int midy = (y1+y2) / 2;
        char fieldNumMid = XYtoFieldNum(midx, midy);
        if (getStateByCoords(g, x2, y2) == 0) {
            if (areEnemies(g, x1, y1, midx, midy)) {
                if (perform) {
                    g->board[fieldNum] = 0;
                    g->board[fieldNumMid] = 0;
                    g->board[fieldNum2] = pawn;
                }
                return 2; //wykonano/możliwe do wykonania bicie
            }
        }
    }
    
    return 0;
}

char canMakeAnyMove(game *g, char fieldNum) { //czy pionek może wykonać jakikolwiek ruch
    if (g->board[fieldNum] == 0) //jeśli puste pole
		return 0;
    char x = getXCoords(fieldNum);
    char y = getYCoords(fieldNum);

    if (isMovePossible(g, x, y, x+1, y+1, 0)) {
        return 1;
    }
    if (isMovePossible(g, x, y, x-1, y+1, 0)) {
        return 1;
    }
    if (isMovePossible(g, x, y, x+1, y-1, 0)) {
        return 1;
    }
    if (isMovePossible(g, x, y, x-1, y-1, 0)) {
        return 1;
    }
    if (isMovePossible(g, x, y, x+2, y+2, 0)) {
        return 1;
    }
    if (isMovePossible(g, x, y, x-2, y+2, 0)) {
        return 1;
    }
    if (isMovePossible(g, x, y, x+2, y-2, 0)) {
        return 1;
    }
    if (isMovePossible(g, x, y, x-2, y-2, 0)) {
        return 1;
    }

    return 0;
}

char canPlayerMakeAnyMove(game *g, char player) {
    for(int i = 0; i < 32; i++) {
        if (player == 1) {
            if (g->board[i] == 1 || g->board[i] == 2) {
                if (canMakeAnyMove(g, i)) {
                    return 1;
                }
            }
        } else {
            if (g->board[i] == 3 || g->board[i] == 4) {
                if (canMakeAnyMove(g, i)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

char whosePawn(char pawn) {
    if (pawn == 1 || pawn == 2)
        return 1;
    if (pawn == 2 || pawn == 3)
        return 2;
    return 0;
}

char makeMove2(game *g, char player, char field1, char field2) {
    char players_pawn = whosePawn(g->board[field1]);
    if (players_pawn != player) {
        return 0;
    }
    char x1 = getXCoords(field1);
    char x2 = getXCoords(field2);
    char y1 = getYCoords(field1);
    char y2 = getYCoords(field2);
    char move_res = isMovePossible(x1, x2, y1, y2, 1);
    if (move_res == 1) { //wykonano ruch bez bicia
        switchPlayers(g); //zamiana gracza
        g->move_count++;
    } else if (move_res == 2) { //ruch z biciem
        g->last_attack = field2;
        g->move_count++;
    } else {
        //niewlasciwy ruch
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

void updateHistory(game *g) { //aktualizuje zajętość pól do heatmapy
    for (int i = 0; i < 32; i++) {
        if (g->board[i] == 1 || g->board[i] == 2) {
            g->board_history_p1[i]++;
        }
        if (g->board[i] == 3 || g->board[i] == 4) {
            g->board_history_p2[i]++;
        }
    }
}