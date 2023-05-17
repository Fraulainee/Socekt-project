
#include <iostream>
#include <cstring>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Winsock Library

#define PORT 8500
#define MAX_BUFFER_SIZE 2048
#define KEY 5
#define IP_address "192.168.1.5"

using namespace std;


void receive_messages(SOCKET sock)
{
    char buffer[MAX_BUFFER_SIZE];
    int recv_size;

    while (true)
    {
        // Receive a message from the server
        if ((recv_size = recv(sock, buffer, MAX_BUFFER_SIZE, 0)) == SOCKET_ERROR)
        {
            fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
            break;
        }

        // Add a terminating null byte to the received data
        buffer[recv_size] = '\0';

        // cout << "This is the encrypted message from the server: " << buffer << endl;
    
        // Decrypt the received message using the substitution cipher
        for (int i = 0; i < recv_size; i++)
        {
            buffer[i] -= KEY;
        }

        cout << buffer << endl;
    }
}

int main(int argc, char *argv[])
{
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char message[MAX_BUFFER_SIZE];
    string clientName;

    // Get the client's name
    cout << "Enter your name: ";
    getline(cin, clientName);

    // Initialize Winsock
    cout << "Initializing Winsock..." << endl;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "Failed. Error Code : %d\n", WSAGetLastError());
        return 1;
    }
    cout << "Initialized." << endl;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        fprintf(stderr, "Could not create socket : %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    cout << "Socket created." << endl;

    // Set up the server address
    server.sin_addr.s_addr = inet_addr(IP_address); // server IP address
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); // server port number

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        fprintf(stderr, "Connect error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    cout << "Connected to server." << endl;

    // Start the thread that listens to incoming messages
    HANDLE thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) receive_messages, (LPVOID) sock, 0, NULL);
    if (thread_handle == NULL)
    {
        fprintf(stderr, "Failed to create thread.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }




    // Send and receive messages
    while (true)
    {
        // Get the user's input
        // cout <<"Enter your message: ";
        fgets(message, MAX_BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';

        // Add the client name to the message
        string fullMessage = clientName + ": " + message + "\n";
        // cout << fullMessage << endl;

        char arr[fullMessage.length() + 1]; 
        strcpy(arr, fullMessage.c_str()); 

        // Encrypt the message using the substitution cipher
        for (int i  = 0; i < fullMessage.length(); i++)
        {
            arr[i] += KEY;
        }

        string encrypted_msg = arr;
        // cout << "This is the encrypted message: " << encrypted_msg << endl;

        // Send the message to the server
        if (send(sock, encrypted_msg.c_str(), strlen(encrypted_msg.c_str()), 0) < 0)
        {
            fprintf(stderr, "Send failed : %d\n", WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return 1;
        }
    }

// Close the socket and clean up Winsock
    closesocket(sock);
    WSACleanup();
    return 0;
}

