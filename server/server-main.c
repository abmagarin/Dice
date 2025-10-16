#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "server.h"
#include <sys/select.h>
#include <errno.h>

int main()
{
    int server_socket, new_socket, client_sockets[MAX_CLIENTS] = {0};
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d\n", PORT);

    fd_set read_fds;
    int max_sd;

    while (1)
    {
        FD_ZERO(&read_fds);

        FD_SET(server_socket, &read_fds);
        max_sd = server_socket;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_sockets[i];
            if (sd > 0)
                FD_SET(sd, &read_fds);
            if (sd > max_sd)
                max_sd = sd;
        }

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR)
        {
            perror("select");
        }

        if (FD_ISSET(server_socket, &read_fds))
        {
            new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (new_socket < 0)
            {
                perror("accept");
                continue;
            }

            printf("Nuevo cliente conectado: %s\n", inet_ntoa(client_addr.sin_addr));
            registerClient(new_socket);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &read_fds))
            {
                char buffer[BUFFER_SIZE];
                int bytes_read = recv(sd, buffer, sizeof(buffer), 0);

                if (bytes_read <= 0)
                {
                    printf("Cliente desconectado.\n");
                    close(sd);
                    client_sockets[i] = 0;
                }
                else
                {
                    buffer[bytes_read] = '\0';
                    printf("Mensaje recibido: %s\n", buffer);

                    char command[20];
                    char parameter[50];
                    char parameter2[50];
                    sscanf(buffer, "%s %s", command, parameter);

                    switch (checkOption(command, client_sockets[i]))
                    {
                    case 1: // USUARIO
                        if (readUser(parameter, client_sockets[i]))
                        {
                            writeUser(client_sockets[i], parameter);
                            char msg[] = "Usuario correcto, introduce la contraseña\n";
                            send(client_sockets[i], msg, strlen(msg), 0);
                        }
                        else
                        {
                            char msg[] = "Usuario incorrecto.\n";
                            send(client_sockets[i], msg, strlen(msg), 0);
                        }
                        printClients();
                        break;

                    case 2: // PASSWORD
                        if (addPassword(client_sockets[i], parameter))
                        {
                            char msg[] = "Contrasña correcta, ha iniciado sesión\n";
                            send(client_sockets[i], msg, strlen(msg), 0);
                        }
                        else
                        {
                            char msg[] = "Contrasña incorrecta y usuario no coinciden\n";
                            send(client_sockets[i], msg, strlen(msg), 0);
                        }
                        printClients();
                        break;

                    case 3: // REGISTRO
                        char option1[50];
                        char option2[50];

                        sscanf(buffer, "%s %s %s %s %s", command, option1, parameter, option2, parameter2);

                        if (strcmp(option1, "-u") == 1 || strcmp(option2, "-p") == 1)
                        {
                            char msg[] = "El formato correcto es: REGISTRO –u usuario –p password\n";
                            send(client_sockets[i], msg, strlen(msg), 0);
                        }
                        else
                        {
                            if (readUser(parameter, client_sockets[i]) == 0)
                            {
                                addUser(parameter, parameter2);
                                char msg[] = "Usuario registrado correctamente, ya puede iniciar sesión\n";
                                send(client_sockets[i], msg, strlen(msg), 0);
                            }
                            else
                            {
                                char msg[] = "El usuario ya existe.\n";
                                send(client_sockets[i], msg, strlen(msg), 0);
                            }
                        }

                        break;
                    default:
                        printf("Opción no reconocida: %s\n", buffer);
                        char msg[] = "Comando no reconocido.\n";
                        send(client_sockets[i], msg, strlen(msg), 0);
                        break;
                    }
                }
            }
        }
    }

    return 0;
}
