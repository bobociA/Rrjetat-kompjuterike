import java.io.*;
import java.net.*;

public class Server {
    public static void main(String[] args) {
        try (ServerSocket serverSocket = new ServerSocket(5000)) {
            System.out.println("Serveri po pret klientin...");
            Socket socket = serverSocket.accept();
            System.out.println("Klienti u lidh!");

            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

            Thread readThread = new Thread(() -> {
                try {
                    String msg;
                    while ((msg = in.readLine()) != null) {
                        System.out.println("Klienti: " + msg);
                        if (msg.equalsIgnoreCase("EXIT")) {
                            break;
                        }
                    }
                } catch (IOException e) {
                    // do nothing
                } finally {
                    try { socket.close(); } catch (IOException ignored) {}
                }
            });
            readThread.start();

            Thread writeThread = new Thread(() -> {
                try (BufferedReader keyboard = new BufferedReader(new InputStreamReader(System.in))) {
                    String msg;
                    while ((msg = keyboard.readLine()) != null) {
                        out.println(msg);
                        if (msg.equalsIgnoreCase("EXIT")) {
                            break;
                        }
                    }
                } catch (IOException ignored) {}
                finally {
                    try { socket.close(); } catch (IOException ignored) {}
                }
            });
            writeThread.start();

            readThread.join();
            writeThread.join();

            System.out.println("Serveri mbyllet.");

        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
    }
}
