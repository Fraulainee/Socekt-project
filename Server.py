import socket
import threading
import select

# Server configurations
HOST = '192.168.1.5'  # Server IP address
PORT = 8500  # Server port
KEY = 5 # Cipher key


# List of connected clients
clients = []
lock = threading.Lock()

def encrypt(text):
    # Apply substitution cipher encryption
    encrypted_text = ""
    for char in text:
        encrypted_char = chr(ord(char) + KEY)
        encrypted_text += encrypted_char
    return encrypted_text

def decrypt(text):
    # Apply substitution cipher encryption
    encrypted_text = ""
    for char in text:
        encrypted_char = chr(ord(char) - KEY)
        encrypted_text += encrypted_char
    return encrypted_text

def broadcast(msg, sender=None):
    # Encrypt the message and send to all connected clients
    encrypted_msg = encrypt(msg)
    with lock:
        for client in clients:
            if client != sender:
                client.send(encrypted_msg.encode('utf-8'))

def handle_client(client):
    while True:
        try:
            # Check if there is data to be received from the client
            readable, _, _ = select.select([client], [], [], 0.1)
            if readable:
                # Receive and decrypt the message from the client
                encrypted_msg = client.recv(1024).decode('utf-8')
                if encrypted_msg:
                    msg = decrypt(encrypted_msg)
                    print(msg)
                    broadcast(msg, client)
                else:
                    # Client disconnected
                    with lock:
                        clients.remove(client)
                    client.close()
                    break
        except ConnectionResetError:
            # Client forcibly closed the connection
            with lock:
                clients.remove(client)
            client.close()
            break


def start_server():
    # Create a socket and start listening for client connections
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_sock.bind((HOST, PORT))
    server_sock.listen(5)
    print(f'Server started on {HOST}:{PORT}')

    threading.Thread(target=send_messages).start()


    try:

        while True:
            # Accept new client connections
            readable, _, _ = select.select([server_sock], [], [], 0.1)

            if readable:
                client, address = server_sock.accept()
                print(f'Connected to {address}')

                # Add client to the list
                with lock:
                    clients.append(client)

                # Start a new thread to handle client communication
                threading.Thread(target=handle_client, args=(client,), daemon=True).start()

                # Send welcome message to the connected client
                welcome_msg = 'Welcome to the chatroom!'
                client.send(encrypt(welcome_msg).encode('utf-8'))

                # Send notification to all clients about the new connection
                notification = f'{address} joined the chatroom.'
                broadcast(notification, sender=client)

    except KeyboardInterrupt:
        # Close the server socket when interrupted by keyboard (Ctrl+C)
        server_sock.close()


    # Start a new thread to handle sending messages from the server
    

def send_messages():
    # Continuously read input from the server and send messages to clients
    while True:
        message = input("")
        if message:
            broadcast(message)

start_server()