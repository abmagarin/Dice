#include "server.h"

// Nombres y contraseñas de usuarios
struct User users[100] = {
    {"alberto", "alberto"},
    {"juan", "juan"},
    {"pedro", "pedro"},
    {"maria", "maria"},
    {"lucia", "lucia"},
    {"carlos", "carlos"},
    {"ana", "ana"},
    {"luis", "luis"},
    {"sofia", "sofia"},
    {"javier", "javier"}};

// Struct de clientes conectados
struct Client clients[MAX_CLIENTS];

int checkOption(char *buffer, int descriptor)
{
    char command[20];
    char parameter[50];

    sscanf(buffer, "%s", command);

    if (strcmp(command, "USUARIO") == 0)
    {
        return 1;
    }
    else if (strcmp(command, "PASSWORD") == 0)
    {
        return 2;
    }
    else
    {
        return -1;
    }
}
// Comprueba que haya algún usuario con ese nombre
int checkUser(char *parameter, int descriptor)
{
    for (int i = 0; i < 100; i++)
    {
        if (strcmp(users[i].usuario, parameter) == 0)
        {
            printf("Usuario %s encontrado.\n", parameter);
            return 1;
        }
    }
    printf("Usuario %s no encontrado.\n", parameter);
    return 0;
}

// Comprueba que haya algún cliente con ese socket
int checkClientSocket(int socket)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == socket)
        {
            return 1;
        }
    }
    return 0;
}

// Comprueba que haya algún cliente en linea con ese nombre
int checkClientUser(char *username)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (strcmp(clients[i].usuario, username) == 0 && clients[i].registered == 1)
        {
            return 1;
        }
    }
    return 0;
}

// Añade el cliente a la lista
int registerClient(int socket)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == 0)
        {
            clients[i].socket = socket;
            clients[i].registered = 0;
            strcpy(clients[i].usuario, "");
            strcpy(clients[i].contraseña, "");
            printf("Cliente registrado en la posición %d (socket: %d)\n", i, socket);
            return 1;
        }
    }
    return 0;
}

// Borra al cliente de la lista
int unregisterClient(int socket)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == socket)
        {
            clients[i].socket = 0;
            clients[i].registered = 0;
            strcpy(clients[i].usuario, "");
            strcpy(clients[i].contraseña, "");
            printf("Cliente eliminado de la posición %d (socket: %d)\n", i, socket);
            return 1;
        }
    }
    printf("No se encontró el cliente con socket %d.\n", socket);
    return 0;
}

// Asigna el nombre user con el socket
int addUser(int socket, char *user)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == socket)
        {
            strcpy(clients[i].usuario, user);
            return 1;
        }
    }
    return 0;
}

// Comprueba que exista un usuario con las características dadas
int findUser(char *usuario, char *password)
{
    for (int i = 0; i < 100; i++)
    {
        if ((strcmp(users[i].usuario, usuario) == 0) && (strcmp(users[i].contraseña, password) == 0))
        {
            return 1;
        }
    }
    return 0;
}

// Añade la contraseña al cliente y cambia su estado a online
int addPassword(int socket, char *password)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == socket && findUser(clients[i].usuario, password))
        {
            strcpy(clients[i].contraseña, password);
            clients[i].registered = 1;
            return 1;
        }
    }
    return 0;
}

void printClients()
{
    printf("\n=== Lista de clientes conectados ===\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket != 0) // Solo muestra los que están en uso
        {
            printf("Posición %d:\n", i);
            printf("  Socket: %d\n", clients[i].socket);
            printf("  Registrado: %s\n", clients[i].registered ? "Sí" : "No");
            printf("  Usuario: %s\n", strlen(clients[i].usuario) > 0 ? clients[i].usuario : "(sin usuario)");
            printf("-----------------------------------\n");
        }
    }
    printf("=== Fin de la lista ===\n\n");
}
