#include <pthread.h>

typedef struct //struktura reprezentująca grę
{
    int player1_socket;
    int player2_socket;
    int move_count; //licznik ruchów
    char player_move; //który gracz obecnie wykonuje ruch
    char board[32]; //plansza
    char last_attack; //czy ostatnio wykonano bicie (jeśli tak to gdzie się ono zakończyło)
    pthread_mutex_t *mutex; //zamek struktury
} game;

//TODO

char getXCoords(char field) {
    return field;
}

char getYCoords(char field) {
    return field;
}

char canAttack(game *g) {

}

char isMoveLegal(game *g, char player, char fiel1, char field2) {

}

void processMove(game *g, char player, char field1, char field2) {

}

void playerQuit(game *g, char player) {

}

void serializeGame(game *g, char *buf) {

}