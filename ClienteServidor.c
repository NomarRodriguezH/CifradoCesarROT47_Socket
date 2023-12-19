#include <stdio.h>
#include <string.h>
#include <winsock2.h>
// RodriguezH Nomar         
/*
 Notas: 1.- Winsock2 es una libreria para windows, es posible que este codigo no funciones para sistemas operativos Unix-Linux. 
 2.- Desactivar windows defender tanto en clinte y sevridor antes de ejecutar.
 3.- Cambiar la IP del servidor
*/
#pragma comment(lib, "ws2_32.lib")
#define PORT 12345
#define MAX_BUFFER_SIZE 1024

void cifrarMensaje(char *mensaje) {
    for (int i = 0; mensaje[i] != '\0'; i++) {
        mensaje[i] = (mensaje[i] + 47 - 33) % 94 + 33;
    }
}

char descifrarRot47(char caracter) {
    if (caracter >= 33 && caracter <= 126) {
        return (caracter - 47 - 33 + 94) % 94 + 33;
    } else {
        return caracter;
    }
}

void enviarArchivo(const char *ip, const char *nombreArchivo) {
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Error al crear el socket: %d\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_port = htons(PORT);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        fprintf(stderr, "Error al conectar con la otra persona/servidor: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    printf("Conectado al servidor!\n");

    FILE* file = fopen(nombreArchivo, "rb");
    if (!file) {
        fprintf(stderr, "Error al abrir el archivo que contiene el mensaje encriptado.\n");
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    char buffer[MAX_BUFFER_SIZE];
    int bytesRead;

    do {
        bytesRead = fread(buffer, 1, MAX_BUFFER_SIZE, file);
        if (bytesRead > 0) {
            send(clientSocket, buffer, bytesRead, 0);
        } else if (bytesRead == 0) {
            printf("Transferencia de archivo completada.\n");
        } else {
            fprintf(stderr, "Error al leer el archivo: %d\n", GetLastError());
        }
    } while (bytesRead > 0);

    fclose(file);
    closesocket(clientSocket);
    WSACleanup();
}

void recibirMensajeCifrado() {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Error al crear el socket: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        fprintf(stderr, "Error al vincular el socket: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Error al escuchar conexiones: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    printf("Esperando mensaje...\n");

    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Error al aceptar la conexión: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    printf("**Cliente conectado**\n");

    FILE *fileCifrado = fopen("mensaje_cifrado2.txt", "w");
    char buffer[MAX_BUFFER_SIZE];
    int bytesRead;
    char *contenidoArchivo = NULL;
    size_t contenidoSize = 0;

    if (!fileCifrado) {
        fprintf(stderr, "Error al abrir archivos para escritura.\n");
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    do {
        bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytesRead > 0) {
            contenidoArchivo = realloc(contenidoArchivo, contenidoSize + bytesRead + 1);
            if (contenidoArchivo == NULL) {
                fprintf(stderr, "Error al asignar memoria para el búfer dinámico.\n");
                fclose(fileCifrado);
                closesocket(clientSocket);
                closesocket(serverSocket);
                WSACleanup();
                exit(1);
            }

            memcpy(contenidoArchivo + contenidoSize, buffer, bytesRead);
            contenidoSize += bytesRead;
        } else if (bytesRead == 0) {
            printf("Transferencia de archivo completada.\n");
        } else {
            fprintf(stderr, "Error al recibir datos: %d\n", WSAGetLastError());
        }
    } while (bytesRead > 0);

    contenidoArchivo[contenidoSize] = '\0';

    for (size_t i = 0; i < contenidoSize; i++) {
        contenidoArchivo[i] = descifrarRot47(contenidoArchivo[i]);
    }

    printf("Contenido descifrado: %s\n", contenidoArchivo);

    
    char mensajeRespuesta[100];
    printf("Ingrese la repuesta al mensaje: ");
    scanf("%s", mensajeRespuesta);

     char resultado[200];
    snprintf(resultado, sizeof(resultado), "%s%s", contenidoArchivo, mensajeRespuesta);
        printf("\n");
    printf("%s \n", resultado);

    cifrarMensaje(resultado);
    printf("EL mensaje con la respuesta cifrada es:  %s\n", resultado);

    fprintf(fileCifrado, "%s\n", resultado);
    fclose(fileCifrado);

    printf("Respuesta escrita en el archivo exitosamente.\n");

    free(contenidoArchivo);
    fclose(fileCifrado);
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "Error al inicializar Winsock.\n");
        return 1;
    }

    FILE *archivo;
    char menu[2];
    char mensajeACifrar[100];
    char respuesta[100];
    char mensaje[1000];

    printf("1.-Cifrar Mensaje\n2.-Descifrar Mensaje\n3.-Salir\n");
    scanf(" %1s", menu);

    if (strcmp(menu, "1") == 0) {
        archivo = fopen("mensaje_cifrado.txt", "w");
        if (archivo == NULL) {
            perror("Error al abrir el archivo");
            return 1;  // Salir con código de error
        }

        while (getchar() != '\n'); // Limpiar el búfer de entrada
        printf("Ingresa el mensaje que quieres mandar y cifrar: ");
        fgets(mensajeACifrar, sizeof(mensajeACifrar), stdin);

        size_t len = strlen(mensajeACifrar);
        if (len > 0 && mensajeACifrar[len - 1] == '\n') {
            mensajeACifrar[len - 1] = '\0';
        }

        cifrarMensaje(mensajeACifrar);
        printf("El mensaje cifrado es: %s\n", mensajeACifrar);

        fprintf(archivo, "%s", mensajeACifrar);
        fclose(archivo);

        enviarArchivo("", "mensaje_cifrado.txt"); // En el primer parametro colocar la ip del servidor

        
    } else if (strcmp(menu, "2") == 0) {
        printf("Vas a desifrar el mensaje que te envien en ROT47\n");

        recibirMensajeCifrado();

        
    }

    return 0;
}