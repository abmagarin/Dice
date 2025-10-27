#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/types.h>

#define PORT 8016
#define MAX_CLIENTS 20
#define MAX_PARTIDAS 10
#define BUFFER_SIZE 200

struct User
{
    char usuario[50];
    char contraseña[50];
};

struct Partida
{
    int id;
    int estado;             
    int jugadores[2];
    int puntuaciones[2];
    int noTirar[2];
    int plantado[2];
    int turno;              
    int puntuacionMax;
};

struct Client
{
    int socket;
    int registered;
    char usuario[50];
    char contraseña[50];
};

int checkOption(char *buffer, int descriptor);

int readUser(char *parameter, int socket);

int checkClientSocket(int socket);

int checkClientUser(char *username);

int registerClient(int socket);

int unregisterClient(int socket);

int findUser(char *usuario, char *password);

int addPassword(int socket, char *password);

void printClients();

int writeUser(int socket, char *user);

int addUser(char *username, char *password);

int checkClientRegistered(int socket);

int addPlayerToGame(int playerSocket);

int numeroAleatorio(int min, int max);

int procesarTirada(int socket, char *buffer);
int procesarNoTirar(int socket);
int procesarPlantarme(int socket);
void procesarSalida(int socket);

int getPartidaDeJugador(int socket);
int getIndiceJugador(struct Partida *p, int socket);
void finalizarPartida(struct Partida *p, const char *mensajeFinal);
#endif