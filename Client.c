#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")
#define CIPHER_KEY 5

unsigned __stdcall ReceiveMessages(void* arg) {
    SOCKET s = *(SOCKET*)arg;
    char server_reply[1024];
    int recv_size;

    while (1) {
        // Receive a reply from the server
        if ((recv_size = recv(s, server_reply, 1024, 0)) == SOCKET_ERROR) {
            printf("Failed to receive data\n");
            break;
        }
        else if (recv_size == 0) {
            printf("Server disconnected\n");
            break;
        }

        // printf("Received encrypted message: %s\n", server_reply);


        // Decrypt the received data using substitution cipher
        for (int i = 0; i < recv_size; i++) {
            server_reply[i] -= CIPHER_KEY;
        }

        server_reply[recv_size] = '\0';

        // Print the server's reply
        printf("%s\n", server_reply);
    }

    _endthreadex(0);
    return 0;
}

int main(int argc, char* argv[]) {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char message[1024];
    HANDLE thread;
    unsigned threadID;
    char name[256];

    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';



    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // Create socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket\n");
        return 1;
    }

    // Set server information
    server.sin_addr.s_addr = inet_addr("192.168.1.5");
    server.sin_family = AF_INET;
    server.sin_port = htons(8500);

    // Connect to remote server
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Failed to connect to server\n");
        return 1;
    }

    // Create a thread to handle receiving messages
    thread = (HANDLE)_beginthreadex(NULL, 0, &ReceiveMessages, &s, 0, &threadID);
    if (thread == NULL) {
        printf("Failed to create receive thread\n");
        return 1;
    }


    while (1) {
        // printf("Type: ");
        fgets(message, sizeof(message), stdin);

        // Remove newline character from the message
        message[strcspn(message, "\n")] = '\0';

        // Exit loop if "exit" is entered
        if (strcmp(message, "exit") == 0)
            break;

        // Encrypt the message using substitution cipher
        // Create a new message buffer with enough space for the name and message
        char combined_message[1280];
        snprintf(combined_message, sizeof(combined_message), "%s: %s", name, message);


        for (int i = 0; i < strlen(combined_message); i++) {
            combined_message[i] += CIPHER_KEY;
        }


        if (send(s, combined_message, strlen(combined_message), 0) < 0) {
            printf("Failed to send data\n");
            break;
        }

    }

    closesocket(s);
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    WSACleanup();

    return 0;
}