import java.io.*;
import java.net.*;

public class Client {
    public static void main(String[] args) {
        try {
            Socket socket = new Socket("localhost", 5000);
            System.out.println("Lidhja me serverin u krijua!");

            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

            Thread readThread = new Thread(() -> {
                try {
                    String msg;
                    while ((msg = in.readLine()) != null) {
                        System.out.println("Serveri: " + msg);
                        if (msg.equalsIgnoreCase("EXIT")) {
                            break;
                        }
                    }
                } catch (IOException ignored) {}
                finally {
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

            System.out.println("Klienti mbyllet.");

        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
    }
}
