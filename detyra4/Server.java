import java.net.*;
import java.io.*;
import java.util.*;

public class Server {

    private static final int PORT = 1234;
    private static List<Socket> waitingClients = new ArrayList<>();

    public static void main(String[] args) {
        System.out.println("Server started...");

        try (ServerSocket serverSocket = new ServerSocket(PORT)) {

            while (true) {
                Socket client = serverSocket.accept();
                System.out.println("Client connected");

                waitingClients.add(client);

                if (waitingClients.size() >= 2) {
                    Socket p1 = waitingClients.remove(0);
                    Socket p2 = waitingClients.remove(0);

                    GameSession game = new GameSession(p1, p2);
                    new Thread(game).start();
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
