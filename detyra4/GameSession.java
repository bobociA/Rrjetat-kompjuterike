import java.net.*;
import java.io.*;

public class GameSession implements Runnable {

    private Socket player1;
    private Socket player2;
    private char[] board = new char[9];
    private char currentPlayer = 'X';

    public GameSession(Socket p1, Socket p2) {
        this.player1 = p1;
        this.player2 = p2;
        for (int i = 0; i < 9; i++) board[i] = ' ';
    }

    @Override
    public void run() {
        try {
            BufferedReader in1 = new BufferedReader(new InputStreamReader(player1.getInputStream()));
            BufferedReader in2 = new BufferedReader(new InputStreamReader(player2.getInputStream()));
            PrintWriter out1 = new PrintWriter(player1.getOutputStream(), true);
            PrintWriter out2 = new PrintWriter(player2.getOutputStream(), true);

            out1.println("START You are X");
            out2.println("START You are O");

            while (true) {
                PrintWriter currentOut = (currentPlayer == 'X') ? out1 : out2;
                BufferedReader currentIn = (currentPlayer == 'X') ? in1 : in2;

                sendBoard(out1, out2);
                currentOut.println("YOUR TURN");

                int move = Integer.parseInt(currentIn.readLine());

                if (isValidMove(move)) {
                    board[move] = currentPlayer;

                    if (checkWin()) {
                        sendBoard(out1, out2);
                        currentOut.println("WIN");
                        getOtherOut(currentOut, out1, out2).println("LOSE");
                        break;
                    }

                    if (isDraw()) {
                        sendBoard(out1, out2);
                        out1.println("DRAW");
                        out2.println("DRAW");
                        break;
                    }

                    switchPlayer();
                } else {
                    currentOut.println("INVALID MOVE");
                }
            }

            player1.close();
            player2.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void sendBoard(PrintWriter o1, PrintWriter o2) {
        String b =
                board[0] + "|" + board[1] + "|" + board[2] + "\n" +
                board[3] + "|" + board[4] + "|" + board[5] + "\n" +
                board[6] + "|" + board[7] + "|" + board[8];
        o1.println(b);
        o2.println(b);
    }

    private boolean isValidMove(int m) {
        return m >= 0 && m < 9 && board[m] == ' ';
    }

    private boolean checkWin() {
        int[][] wins = {
            {0,1,2},{3,4,5},{6,7,8},
            {0,3,6},{1,4,7},{2,5,8},
            {0,4,8},{2,4,6}
        };

        for (int[] w : wins) {
            if (board[w[0]] == currentPlayer &&
                board[w[1]] == currentPlayer &&
                board[w[2]] == currentPlayer)
                return true;
        }
        return false;
    }

    private boolean isDraw() {
        for (char c : board)
            if (c == ' ') return false;
        return true;
    }

    private void switchPlayer() {
        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    }

    private PrintWriter getOtherOut(PrintWriter current, PrintWriter o1, PrintWriter o2) {
        return (current == o1) ? o2 : o1;
    }
}
