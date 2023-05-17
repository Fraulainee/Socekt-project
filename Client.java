import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.net.InetAddress;

public class Client {
    private static final int PORT = 8500;
    private static final String SERVER = "192.168.1.5";
    private static final int BUFFER_SIZE = 1024;

    private Socket clientSocket;
    private InputStream input;
    private OutputStream output;
    private ExecutorService executorService;
    private boolean running;
    int key = 5;

    public Client() {
        try {
            this.clientSocket = new Socket(SERVER, PORT);
            this.input = clientSocket.getInputStream();
            this.output = clientSocket.getOutputStream();
            this.executorService = Executors.newFixedThreadPool(2);
            this.running = true;
        } catch (UnknownHostException e) {
            System.err.println("Unknown host: " + SERVER);
            System.exit(1);
        } catch (IOException e) {
            System.err.println("Error connecting to server");
            System.exit(1);
        }
    }

    private void receive() {
        byte[] buffer = new byte[BUFFER_SIZE];
        while (running) {
            try {
                int bytesRead = input.read(buffer);
                // int key = 6;
                if (bytesRead > 0) {
                    byte[] receivedBytes = Arrays.copyOf(buffer, bytesRead);
                    String message = new String(receivedBytes);
                    // System.out.println(message);
                    char[] chars = message.toCharArray();

                    for(char c: chars){
                        c -= key;
                        System.out.print(c);
                    }
                    System.out.println();
                }
            } catch (IOException e) {
                System.err.println("Error receiving message from server");
                stop();
            } 
            
        }
    }
    
    private void send() {
        try (Scanner scanner = new Scanner(System.in)) {
            String hostname;
            try {
                hostname = InetAddress.getLocalHost().getHostName();
            }
            catch (UnknownHostException e) {
                System.err.println("Unknown host: " + SERVER);
                System.exit(1);
                return;
            }
            while (running) {
                String message = scanner.nextLine();
                message = "[" + hostname + "]: " + message;
                char [] chars = message.toCharArray();

                try {
                    for(char c: chars){
                        c += key;
                        output.write(c);
                        
                    }
                  
                } catch (IOException e) {
                    System.err.println("Error sending message to server");
                    stop();
                }
            }
        }
    }    
        
    public void start() {
        executorService.submit(this::receive);
        executorService.submit(this::send);
    }
    
    public void stop() {
        running = false;
        executorService.shutdownNow();
        try {
            clientSocket.close();
        } catch (IOException e) {
            System.err.println("Error closing client socket");
        }
    }
    
    public static void main(String[] args) {
        Client client = new Client();
        client.start();
        Runtime.getRuntime().addShutdownHook(new Thread(client::stop));
    }
}