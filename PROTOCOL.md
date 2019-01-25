Porzucony protokół (zbyt złożony)

Komunikacja między klientem a serwerem odbywa się przy użyciu komend tekstowych:

komenda;parametr1;parametr2;[...];parametrN;\n

# Komendy wysyłane przez klienta:

## list;
Wysyła zapytanie o listę dostępnych lobby

odpowiedź:

list_r;nazwa_lobby_1;limit_czasu_1;nazwa_lobby_2;limit_czasu_2;[...];nazwa_lobby_N;limit_czasu_N;

## clob;nazwa_lobby;limit_czasu_sec;
Wysyła zapytanie tworzące nową grę o podanych ustawieniach

odpowiedź:

clob_r;status;

status:

0 = OK

1 = FAIL

## join;nazwa_lobby;

odpowiedź:
join_r;status;

status:
0 = OK

1 = LOBBY NOT FOUND

2 = LOBBY FULL

## move;pos1;pos2;

odpowiedź:

move_r;pos1;pos2;status;

status:
0 = OK
1 = WRONG MOVE

# Komendy wysyłane przez serwer

## start;

Informuje o rozpoczęciu gry

## go;

Nakazuje graczowi wykonanie ruchu

## emove;pos1;pos2;

Inforuje o ruchu wykonanym przez przeciwnika

## end;status;move1_1;move1_2;move2_1;move2_2;[...];moveN_1;moveN_2;

Koniec gry + informacja o jej przebiegu

status:

0 = WON

1 = LOST

2 = ENEMY TIMEOUT
