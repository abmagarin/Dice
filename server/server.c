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

struct Partida partidas[MAX_PARTIDAS];

int checkOption(char *command, int descriptor)
{

    if ((strcmp(command, "USUARIO") == 0) && (checkClientRegistered(descriptor) == 0))
    {
        return 1;
    }
    else if ((strcmp(command, "PASSWORD") == 0) && (checkClientRegistered(descriptor) == 0))
    {
        return 2;
    }
    else if ((strcmp(command, "REGISTRO") == 0) && (checkClientRegistered(descriptor) == 0))
    {
        return 3;
    }
    else if ((strcmp(command, "INICIAR-PARTIDA") == 0) && (checkClientRegistered(descriptor) == 1))
    {
        return 4;
    }
    else if ((strcmp(command, "TD") == 0) && (checkClientRegistered(descriptor) == 1))
    {
        return 5;
    }
    else if ((strcmp(command, "NO-TIRAR-DADOS") == 0) && (checkClientRegistered(descriptor) == 1))
    {
        return 6;
    }
    else if ((strcmp(command, "PLANTARME") == 0) && (checkClientRegistered(descriptor) == 1))
    {
        return 7;
    }
    else if ((strcmp(command, "SALIR") == 0) && (checkClientRegistered(descriptor) == 1))
    {
        return 8;
    }
    else
    {
        return -1;
    }
}

// Comprueba que haya algún usuario con ese nombre
int readUser(char *parameter, int descriptor)
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

// Recorre todas las partidas para saber donde está el jugador que ha actuado para guardar los datos
int getIdPartidaDeJugador(int socket)
{
    for (int i = 0; i < MAX_PARTIDAS; i++)
    {
        if (partidas[i].jugadores[0] == socket || partidas[i].jugadores[1] == socket)
            return i;
    }
    return -1;
}

void resetPartida(struct Partida *p)
{
    p->estado = 0;
    p->jugadores[0] = 0;
    p->jugadores[1] = 0;
    p->puntuaciones[0] = 0;
    p->puntuaciones[1] = 0;
    p->plantado[0] = 0;
    p->plantado[1] = 0;
    p->turno = 0;
    p->puntuacionMax = 0;
}

// Borra al cliente de la lista

int unregisterClient(int socket)
{

    int partidaIndex = getIdPartidaDeJugador(socket);
    if (partidaIndex != -1)
    {
        struct Partida *p = &partidas[partidaIndex];
        int jugador = getIndiceJugador(p, socket);
        int rival = 1 - jugador;

        printf("[DEBUG] Cliente con socket %d abandonó la partida %d\n", socket, partidaIndex);

        // Si la partida sigue activa, avisar al rival
        if (p->estado == 2 && p->jugadores[rival] != 0)
        {
            char msg[120];
            sprintf(msg, "+Ok. Tu oponente se ha desconectado. Has ganado.\n");
            send(p->jugadores[rival], msg, strlen(msg), 0);
        }

        // Limpiar estructura de partida
        resetPartida(p);

        printf("[DEBUG] Partida %d finalizada por desconexión.\n", partidaIndex);
    }

    // quitarlo de la lista
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == socket)
        {
            clients[i].socket = 0;
            clients[i].registered = 0;
            strcpy(clients[i].usuario, "");
            strcpy(clients[i].contraseña, "");
            printf("[DEBUG] Cliente desconectado (índice %d, socket %d)\n", i, socket);
            return 1;
        }
    }
    printf("[DEBUG] No se encontró el cliente con socket %d.\n", socket);
    return 0;
}

// Asigna el nombre user con el socket
int writeUser(int socket, char *user)
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
    if (usuario == NULL || password == NULL || strlen(password) == 0)
        return 0;

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

int addUser(char *username, char *password)
{
    for (int i = 0; i < 100; i++)
    {
        if (strcmp(users[i].usuario, "") == 0)
        {
            strcpy(users[i].usuario, username);
            strcpy(users[i].contraseña, password);
            printf("Usuario %s añadido.\n", username);
            return 1;
        }
    }
    return 0;
}

int checkClientRegistered(int socket)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].socket == socket)
        {
            return clients[i].registered;
        }
    }
    return 0;
}

int addPlayerToGame(int playerSocket)
{
    int oldMatch = getIdPartidaDeJugador(playerSocket);
    if (oldMatch != -1)
    {
        printf("[DEBUG] Jugador %d aún estaba en la partida %d. Limpiando...\n", playerSocket, oldMatch);
        resetPartida(&partidas[oldMatch]);
    }

    for (int i = 0; i < MAX_PARTIDAS; i++)
    {
        if (partidas[i].estado == 0)
        {
            partidas[i].jugadores[0] = playerSocket;
            partidas[i].estado = 1;
            return 1;
        }
        else if (partidas[i].estado == 1)
        {
            partidas[i].jugadores[1] = playerSocket;
            partidas[i].estado = 2;
            partidas[i].puntuacionMax = numeroAleatorio(20, 30);
            partidas[i].puntuaciones[0] = partidas[i].puntuaciones[1] = 0;
            partidas[i].plantado[0] = partidas[i].plantado[1] = 0;
            partidas[i].turno = 0;
            char msg[100];
            sprintf(msg, "+Ok. Empieza la partida. NÚMERO OBJETIVO: [%d]\n", partidas[i].puntuacionMax);

            send(partidas[i].jugadores[1], msg, strlen(msg), 0);
            send(partidas[i].jugadores[0], msg, strlen(msg), 0);

            printf("PARTIDA INICIADA \n\n");
            return 2;
        }
    }
    return 0;
}

int numeroAleatorio(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

// Se encarga de cuando se tira un dado

int procesarTirada(int socket, char *buffer)
{
    int partidaIndex = getIdPartidaDeJugador(socket);
    if (partidaIndex == -1)
    {
        send(socket, "-Err. No estás en una partida.\n", 40, 0);
        return -1;
    }

    struct Partida *p = &partidas[partidaIndex];
    int jugador = getIndiceJugador(p, socket);
    if (p->plantado[jugador] == 0)
    {
        if (p->estado != 2)
        {
            send(socket, "-Err. La partida no está activa.\n", 40, 0);
            return -1;
        }

        if (p->turno != jugador)
        {
            send(socket, "-Err. No es tu turno.\n", 40, 0);
            return -1;
        }

        printf("[DEBUG] Jugador %d (socket %d) tirando dados en partida %d\n", jugador, socket, partidaIndex);

        int nDados = 1;
        sscanf(buffer, "%*s %d", &nDados);
        if (nDados < 1 || nDados > 2)
        {
            nDados = 1;
        }

        int dado1 = numeroAleatorio(1, 6);
        int dado2 = 0;
        if (nDados == 2)
        {
            dado2 = numeroAleatorio(1, 6);
        }
        int suma = dado1 + dado2;

        p->puntuaciones[jugador] += suma;

        char msg[200];
        if (nDados == 1)
        {
            sprintf(msg, "+Ok.[<DADO1,%d>] Puntuación total: %d\n +Ok. Has terminado tu turno.\n", dado1, p->puntuaciones[jugador]);
        }
        else
        {
            sprintf(msg, "+Ok.[<DADO1,%d>,<DADO2,%d>] Puntuación total: %d\n +Ok. Has terminado tu turno.\n", dado1, dado2, p->puntuaciones[jugador]);
        }

        // Enviar el resultado de la tirada a ambos jugadores
        send(p->jugadores[jugador], msg, strlen(msg), 0);

        printf("[DEBUG] Puntuación jugador %d: %d / Objetivo: %d\n", jugador, p->puntuaciones[jugador], p->puntuacionMax);

        // Si se pasa del objetivo
        if (p->puntuaciones[jugador] > p->puntuacionMax)
        {
            char msgErr[100];
            sprintf(msgErr, "-Err. Excedido el valor de %d\n", p->puntuacionMax);
            send(p->jugadores[jugador], msgErr, strlen(msgErr), 0);
            finalizarPartida(p, "+Ok. Tu oponente se ha pasado. Has ganado.\n");
            printf("[DEBUG] Partida %d finalizada: jugador %d se pasó del límite\n", partidaIndex, jugador);
            return 1;
        }

        // Cambiar turno
    }
    if (p->plantado[jugador] == 1)
    {
        char msgPL[100];
        sprintf(msgPL, "-Err. Te habías plantado \n");
        send(p->jugadores[jugador], msgPL, strlen(msgPL), 0);
    }
    p->turno = 1 - jugador;

    printf("Turno cambiado. Ahora juega jugador %d (socket %d)\n", p->turno, p->jugadores[p->turno]);
    return 0;
}

int procesarNoTirar(int socket)
{
    int partidaIndex = getIdPartidaDeJugador(socket);
    if (partidaIndex == -1)
    {
        send(socket, "-Err. No estás en una partida.\n", 40, 0);
        return -1;
    }

    struct Partida *p = &partidas[partidaIndex];
    int jugador = getIndiceJugador(p, socket);

    if (p->turno != jugador)
    {
        send(socket, "-Err. No es tu turno.\n", 40, 0);
        return -1;
    }

    // Mensaje claro de que el jugador no tira dados
    char msg[100];
    sprintf(msg, "+Ok. Jugador %d ha decidido no tirar dados este turno.\n", jugador + 1);

    // Enviar a ambos jugadores
    send(p->jugadores[jugador], msg, strlen(msg), 0);
    // Cambiar turno

    p->turno = 1 - jugador;
    return 0;
}

int procesarPlantarme(int socket)
{
    int partidaIndex = getIdPartidaDeJugador(socket);
    if (partidaIndex == -1)
    {
        send(socket, "-Err. No estás en una partida.\n", 40, 0);
        return -1;
    }

    struct Partida *p = &partidas[partidaIndex];
    int jugador = getIndiceJugador(p, socket);

    p->plantado[jugador] = 1;
    send(socket, "+Ok. Te has plantado.\n", 30, 0);

    // Si ambos se plantaron → finalizar partida
    if (p->plantado[0] && p->plantado[1])
    {
        if (p->puntuaciones[0] > p->puntuaciones[1])
        {
            finalizarPartida(p, "+Ok. Partida finalizada. Jugador 1 gana.\n");
        }
        else if (p->puntuaciones[1] > p->puntuaciones[0])
        {
            finalizarPartida(p, "+Ok. Partida finalizada. Jugador 2 gana.\n");
        }
        else
        {
            finalizarPartida(p, "+Ok. Empate.\n");
        }
        return 1;
    }

    // Si no, turno para el otro
    p->turno = 1 - jugador;
    return 0;
}

void procesarSalida(int socket)
{
    int partidaIndex = getIdPartidaDeJugador(socket);
    if (partidaIndex == -1)
        return;

    struct Partida *p = &partidas[partidaIndex];
    int jugador = getIndiceJugador(p, socket);
    int otro = 1 - jugador;

    send(socket, "+Ok. Has salido de la partida.\n", 40, 0);
    send(p->jugadores[otro], "+Ok. Tu oponente ha abandonado. Ganas la partida.\n", 60, 0);

    resetPartida(p);
}

// devuelve el en cual de las dos posciones está el jugador
int getIndiceJugador(struct Partida *p, int socket)
{
    return (p->jugadores[0] == socket) ? 0 : 1;
}

// Avisa a los jugadores de que la partida ha terminado ya sea por abandono o por derrota o vicoria del otro
void finalizarPartida(struct Partida *p, const char *mensajeFinal)
{
    for (int i = 0; i < 2; i++)
    {
        send(p->jugadores[i], mensajeFinal, strlen(mensajeFinal), 0);
    }
    resetPartida(p);
}
