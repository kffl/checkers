Uproszczona wersja protokołu

Komunikacja między klientem a serwerem odbywa się przy użyciu komend tekstowych:
```
komenda;parametr1;parametr2;[...];parametrN;\n
```
Znak nowej linii informuje o końcu komendy.

# Komendy następujące po połączeniu klienta z serwerem:

## await;

Wysyłana przez serwer - informuje klienta o oczekiwaniu na połączenie się drugiego gracza (a także że jest graczem nr. 1)

## start;

Wysyłana przez serwer - informuje o rozpoczęciu gry

## move;

Wysyłana przez klienta, składnia:

```
move;pos1;pos2;
```

Informuje o wykonaniu ruchu z pos1 na pos2

## state;

Wysyłana przez serwer - informuje o aktualnym stanie gry

```
state;status;move_no;pos0;pos1; ... ;pos31;
```

- `status`: 1 - ruch gracza 1; 2 - ruch gracza 2; 3 - wygrana gracza 1; 4 - wygrana gracza 2; 5- remis; 6 - wyjście gracza 1; 7 - wyjście gracza 2;
- `move_no`: numer ruchu od początku gry (numerowane od 1)
- `posn`: pionek znajdujący się na polu o numerze n (numeracja od lewego, górnego rogu); 0 - puste; 1 - pionek gracza 1; 2 - pionek gracza 2; 3 - król gracza 1; 4 - król gracza 2;

## end;

Wysyłana przez serwer po zakończeniu gry

## quit;

Wysyłana przez klientów przed wyjściem
