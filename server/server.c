#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "gameLogic.c"
#include "parser.c"

#define MAX_GAMES 1024
#define QUEUE_SIZE 10

//tablica rozgrywanych gier
struct game *games[MAX_GAMES];
//zamek chroniący dostęp do tablicy rozgrywanych gier
pthread_mutex_t *games_mutex;

//struktura przekazywana do wątku gameManager (oraz do gameTimer)
typedef struct
{
    game* current_game;
    int player; //którego gracza nasłuchuje gameManager, w przypadku gameTimer - 0
} game_manager_data;

//inicjalizacja nowej struktury game
void initializeGame(game *new_game, int player1_sd) {
    new_game->player1_socket = player1_sd;
    new_game->move_count = 0;
    new_game->last_attack = 60;
    new_game->player_move = 0;
    new_game->is_terminated = 0;
    new_game->dead_threads_count = 0;
    new_game->mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    for (int i = 0; i < 32; i++) { //ustawianie pionków
        if (i < 12) {
            new_game->board[i] = 1; //pionki gracza 1 w górnej części
            new_game->board_history_p1[i] = 1;
            new_game->board_history_p2[i] = 0;
        } else if (i >= 20) {
            new_game->board[i] = 3; //gracza 2 w dolnej
            new_game->board_history_p1[i] = 0;
            new_game->board_history_p2[i] = 1;
        } else {
            new_game->board[i] = 0; //pozostałe pola puste
            new_game->board_history_p1[i] = 0;
            new_game->board_history_p2[i] = 0;
        }
    }    
}

//znajduje wolny slot na grę w tablicy games
int findAvailiableGame() {
    pthread_mutex_lock(games_mutex);
    for (int i = 0; i < MAX_GAMES; i++) {
        if (games[i] == NULL) {
            pthread_mutex_unlock(games_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(games_mutex);
    return -1; //nie znaleziono
}

//wysyła komendę do klienta pilnując fragmentacji 1 jeśli ok, 0 jeśli niepowodzenie
char sendToClient(int socket, char *buf, int n) {
    int i = 0;
    int s;
    while (n > 0) { //zabezpiecza przed pofragmentowaną wysyłką
        s = send(socket, buf+i, n, 0);
        if (s <= 0) {
            return 0;
        }
        i += s;
        n -= s;
    }
    return 1;
}

//kończy grę z powodu = status
void killGame(game *g, int status) {
    g->player_move = status;
    char buf[1024];
    int i;
    i = serializeGame(g, buf);
    sendToClient(g->player1_socket, buf, i);
    sendToClient(g->player2_socket, buf, i);
    char buf2[] = "end;\n";
    sendToClient(g->player1_socket, buf2, 5);
    sendToClient(g->player2_socket, buf2, 5);
    sleep(1);
    printf("Gra %d: zakonczona. Sockety - shutdown.\n", g->game_id);
    //Sutdown z SHUT_RD żeby obudzić oczekujące wątki gameManager
    //oba wątki gameManager obudzą się i odczytają dead_threads_count > 0
    shutdown(g->player1_socket, SHUT_RD);
    shutdown(g->player2_socket, SHUT_RD);
}

//procedura wykonywana przez wątek nasłuchujący komend gracza (2 szt / grę)
void *gameManager(void *t_data) {
    int i;
    game_manager_data *th_data = (game_manager_data*)t_data;
    game *current_game = th_data->current_game;
    int player = th_data->player;
    int game_id = current_game->game_id;

    pthread_detach(pthread_self());
    printf("Uruchomiono gameManager gry %d, gracza %d.\n", current_game->game_id, (*th_data).player);
    int v;
    char buf[1024];
    int player_socket = 0;
    int other_player_socket = 0;
    if (player == 1) {
        player_socket = current_game->player1_socket;
        other_player_socket = current_game->player2_socket;
    } else {
        player_socket = current_game->player2_socket;
        other_player_socket = current_game->player1_socket;
    }

    //na początek wyślij stan gry do swojego klienta

    pthread_mutex_lock(current_game->mutex);
    i = serializeGame(current_game, buf);
    sendToClient(other_player_socket, buf, i);
    pthread_mutex_unlock(current_game->mutex);

    while(1) {
        v = recv(player_socket, buf, 1024, 0);
        pthread_mutex_lock(current_game->mutex);
        //mutex gry założony
        //na czas przetwarzania odebranego zapytania


        //sprawdzamy czy gra się nie skończyła
        if (current_game->dead_threads_count) {
        //jeśli jakiś wątek skończył pracę w międzyczasie
        //oznacza to że sockety już są pozamykane
            if (current_game->dead_threads_count == 1) {
                current_game->dead_threads_count++;
                printf("GameManager gry %d kończy pracę.\n", current_game->game_id);
                pthread_mutex_unlock(current_game->mutex);
                pthread_exit(NULL);
            } else if (current_game->dead_threads_count == 2) {
                //pozostałe 2 wątki już się skończyły, trzeba posprzątać po grze
                //shutdown był wcześniej, teraz close aby klient się dowiedział
                close(current_game->player1_socket);
                close(current_game->player2_socket);                
                pthread_mutex_unlock(current_game->mutex);
                pthread_mutex_lock(games_mutex);
                free(current_game);
                games[game_id] = NULL;
                pthread_mutex_unlock(games_mutex);
                printf("GameManager gry %d posprzątał - teraz kończy pracę.\n", game_id);
                pthread_exit(NULL);
            }

        } 
        
        if (v>0) {
            //jeżeli odebrano wiadomość od klienta
            char cmd;
            cmd = parseCommandName(buf);
            if (cmd == 1) { //komenda move
                int pos1, pos2;
                parseMove(buf, &pos1, &pos2);

                if (makeMove(current_game, (*th_data).player, pos1, pos2) > 0) {
                    //wykonano ruch
                    printf("Gra %d : gracz %d wykonał poprawny ruch: %d, %d\n", game_id, (*th_data).player, pos1, pos2);
                    if (current_game->player_move > 2) { //jeśli gra się zakończyła
                        printf("Gra %d : koniec gry \n", game_id);
                        killGame(current_game, current_game->player_move);   
                        current_game->dead_threads_count++;   
                        printf("GameManager gry %d kończy pracę jako pierwszy.\n", current_game->game_id);
                        pthread_mutex_unlock(current_game->mutex);
                        pthread_exit(NULL);
                    }
                    i = serializeGame(current_game, buf);
                    sendToClient(other_player_socket, buf, i);
                    sendToClient(player_socket, buf, i);
                    
                } else {
                    //nie wykonano ruchu
                    printf("Gra %d : gracz %d wykonał niepoprawny ruch: %d, %d, ignorujemy\n", game_id, (*th_data).player, pos1, pos2);
                }                
            } else if (cmd == 2) {
                //komenda quit
                printf("Gra %d : gracz %d opuścił grę\n", game_id, (*th_data).player);
                killGame(current_game, ((*th_data).player)+5);   
                current_game->dead_threads_count++;   
                printf("GameManager gry %d kończy pracę jako pierwszy.\n", current_game->game_id);
                pthread_mutex_unlock(current_game->mutex);
                pthread_exit(NULL);
            }
        } else {
            //klient się rozłączył
            printf("Gra %d : gracz %d - utracono połączenie\n", game_id, (*th_data).player);
            killGame(current_game, ( (*th_data).player) + 5);
            current_game->dead_threads_count++;
            printf("GameManager gry %d kończy pracę jako pierwszy.\n", current_game->game_id);
            pthread_mutex_unlock(current_game->mutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(current_game->mutex);
    }
    pthread_exit(NULL);
}

//procedura wykonywana przez wątek timera - co 1s sprawdza czy czas gry nie został przekroczony
//1 szt. na grę
void *gameTimer(void *t_data) {
    
    pthread_detach(pthread_self());
    game_manager_data *th_data = (game_manager_data*)t_data;
    game *g = (*th_data).current_game;
    printf("Uruchomiono gameTimer gry %d.\n", g->game_id);
    int game_end = 0;

    while (!game_end) {
        sleep(1);
        pthread_mutex_lock(g->mutex);

        if (g->dead_threads_count) {
        //jeśli jakiś wątek skończył się w międzyczasie
        //oznacza to że sockety już są pozamykane
            if (g->dead_threads_count == 1) {
                g->dead_threads_count++;
                printf("GameTimer gry %d kończy pracę.\n", g->game_id);
                pthread_mutex_unlock(g->mutex);
                pthread_exit(NULL);
            } else if (g->dead_threads_count == 2) {
                //pozostałe 2 wątki już się skończyły, trzeba posprzątać po grze
                //shutdown był wcześniej, teraz close aby klient się dowiedział
                close(g->player1_socket);
                close(g->player2_socket);   
                pthread_mutex_unlock(g->mutex);
                pthread_mutex_lock(games_mutex);
                int game_id = g->game_id;
                free(g);
                games[game_id] = NULL;
                pthread_mutex_unlock(games_mutex);
                printf("GameTimer gry %d posprzątał - teraz kończy pracę.\n", game_id);
                pthread_exit(NULL);
            }

        } else {
            //wpp
            if (isTimeUp(g)) {
                //jeśli czas na wykonanie ruchu upłynął
                printf("GameTimer gry %d : czas ruchu gracza %d upłynął.\n", g->game_id, g->player_move);
                int status = g->player_move + 5;
                killGame(g, status);
                g->dead_threads_count++;
                printf("GameTimer gry %d kończy pracę.\n", g->game_id);
                pthread_mutex_unlock(g->mutex);
                pthread_exit(NULL);
            }
        }        

        pthread_mutex_unlock(g->mutex);
    }
    pthread_exit(NULL);
}

//oczekuje na nowe połączenia i przydziela graczy do lobby
void connectionListener(int server_socket_descriptor) {

    int connection_socket_descriptor;
    int awaiting_game = -1; //gra, która czeka na dołączenie graczy
    int awaiting_player_socket_descriptor;
    int create_result1 = 0;
    int create_result2 = 0;
    int create_result_timer = 0;

    while(1)
    {
        connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
        if (connection_socket_descriptor < 0)
        {
            fprintf(stderr, "Błąd przy próbie utworzenia gniazda dla połączenia.\n");
            exit(1);
        }
        printf("Połączenie...\n");

        if (awaiting_game == -1) {
            //żadna gra nie oczekuje, trzeba otworzyć nową
            printf("Tworzenie nowej poczekalni...\n");
            awaiting_game = findAvailiableGame();
            if (awaiting_game == -1) {
                //wszystkie zajęte
                //serwer zapełniony, odrzuca połączenie
                close(connection_socket_descriptor);
            } else {
                //mamy gracza 1, każemy mu czekać
                awaiting_player_socket_descriptor = connection_socket_descriptor;
                game *new_game = (game *) malloc(sizeof(game));
                new_game->game_id = awaiting_game;
                initializeGame(new_game, awaiting_player_socket_descriptor);

                games[awaiting_game] = new_game;
                char buf[128] = "await;\n";
                send(awaiting_player_socket_descriptor, buf, strlen(buf), 0);
            }
        } else {
            //jakaś gra już oczekuje na gracza
            printf("Poczekalnia pełna, tworzenie gry...\n");
            game *aw_game = (game *) games[awaiting_game];
            aw_game -> player2_socket = connection_socket_descriptor;
            aw_game->time_deadline = time(NULL) + 60;
            awaiting_game = -1;
            //obaj gracze się połączyli, odpalamy gameManager dla każdego z nich
            game_manager_data *data_p1 = malloc(sizeof(game_manager_data));
            game_manager_data *data_p2 = malloc(sizeof(game_manager_data));
            data_p1 -> current_game = aw_game;
            data_p1 -> player = 1;
            data_p2 -> current_game = aw_game;
            data_p2 -> player = 2;
            pthread_t thread1;
            pthread_t thread2;

            //odpalamy gameTimer dla gry
            game_manager_data *data_timer = malloc(sizeof(game_manager_data));
            pthread_t thread_timer;
            data_timer -> current_game = aw_game;
            data_timer -> player = 0;

            //wysyłka komendy start
            char buf[128] = "start;\n";
            sendToClient(awaiting_player_socket_descriptor, buf, strlen(buf));
            sendToClient(connection_socket_descriptor, buf, strlen(buf));
            aw_game -> player_move = 1;

            //tworzenie trzech wątków
            create_result1 = pthread_create(&thread1, NULL, gameManager, (void *)data_p1);
            if (create_result1){
                printf("Błąd przy próbie utworzenia wątku gameManager, kod błędu: %d\n", create_result1);
                close(awaiting_player_socket_descriptor);
                exit(-1);
            }
            create_result2 = pthread_create(&thread2, NULL, gameManager, (void *)data_p2);
            if (create_result2){
                printf("Błąd przy próbie utworzenia wątku gameManager, kod błędu: %d\n", create_result2);
                close(connection_socket_descriptor);
                exit(-1);
            }

            create_result_timer = pthread_create(&thread_timer, NULL, gameTimer, (void *)data_timer);
            if (create_result_timer){
                printf("Błąd przy próbie utworzenia wątku gameTimer, kod błędu: %d\n", create_result_timer);
                exit(-1);
            }
        }
        
    }
}

int main(int argc, char *argv[])
{
    int server_socket_descriptor;
    long server_port = 1234;
    int bind_result;
    int listen_result;
    char reuse_addr_val = 1;
    struct sockaddr_in server_address;
    
    //port serwera z argumentów linii poleń
    if(argc == 2) server_port = strtol(argv[1], NULL, 0);

    //inicjalizacja gniazda serwera

    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(server_port);

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda..\n", argv[0]);
        exit(1);
    }
    setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

    bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
    if (bind_result < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
        exit(1);
    }

    listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
    if (listen_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
        exit(1);
    }
    
    printf("Zainicjowano serwer na porcie %ld \n", server_port);
    

    for (int i = 0; i < MAX_GAMES; i++) {
        //zapełnianie tablicy wskaźników na aktywne gry nullami
        games[i] = NULL;
    }

    //mutex chroniący tablicę games
    games_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    
    printf("Odbieranie...\n");

    connectionListener(server_socket_descriptor);

    close(server_socket_descriptor);
    return(0);
}