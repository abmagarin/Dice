#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8016
#define MAX_CLIENTS 20
#define BUFFER_SIZE 200

struct User
{
    char usuario[50];
    char contraseña[50];
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

#endif