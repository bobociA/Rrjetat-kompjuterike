import java.net.*;
import java.io.*;
import java.util.*;

public class Client {

    public static void main(String[] args) {
        try {
            Socket socket = new Socket("localhost", 1234);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            Scanner sc = new Scanner(System.in);

            String message;
            while ((message = in.readLine()) != null) {
                System.out.println(message);

                if (message.equals("YOUR TURN")) {
                    System.out.print("Enter move (0-8): ");
                    out.println(sc.nextLine());
                }
            }

            socket.close();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
