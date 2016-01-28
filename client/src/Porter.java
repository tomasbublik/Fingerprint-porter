import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

/**
 * Created by tomas.bublik on 1.11.2015.
 */
public class Porter {

    public static void main(String[] args) {
        Connection c = null;
        System.out.println("Starting database connection");
        c = getConnection(c);

        String newLine = System.getProperty("line.separator");
        System.out.println("Reading Strings from console");

        //You use System.in to get the Strings entered in console by user
        try {
            //You need to create BufferedReader which has System.in to get user input
            BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
            String userInput;
            System.out.println("Starting porter");
            String splited[];

            do {
                //reader .readLine() would wait for the user to enter strings.
                //keep and entering text and when you press enter, all those would be displayed
                //so it is keeping it in buffer until the user types 'quit' and presses enter
                userInput = (String) br.readLine();

                boolean saved = saveString(c, userInput);

            } while (!userInput.equals("quit"));
        } catch (Exception e) {
            try {
                c.close();
            } catch (SQLException e1) {
                e1.printStackTrace();
            }
        }
    }

    public static Connection getConnection(Connection c) {
        try {
            Class.forName("org.sqlite.JDBC");
            try {
                c = DriverManager.getConnection("jdbc:sqlite:porter.db");
            } catch (SQLException e) {
                e.printStackTrace();
            }
            System.out.println("Opened database successfully");
        } catch (Exception e) {
            System.err.println(e.getClass().getName() + ": " + e.getMessage());
        }
        return c;
    }

    public static boolean saveString(Connection c, String userInput) throws SQLException {
        String[] splited;
        Statement stmt;
        StringBuilder stringBuilder = new StringBuilder();
        if (userInput != null && userInput.contains("|")) {
            System.out.println("Entered: " + userInput);
            splited = userInput.split("\\|");
            if (splited.length == 4) {
                stmt = c.createStatement();
                stringBuilder.append("INSERT INTO porter (action, status, time, humanid) VALUES (");
                stringBuilder.append("\"");
                // appending action
                stringBuilder.append(splited[0]).append("\",\"");
                // appending status
                stringBuilder.append(splited[1]).append("\",\"");
                // appending time
                stringBuilder.append(splited[2]).append("\",\"");
                // appending id
                stringBuilder.append(splited[3]).append("\")");
                String sql = stringBuilder.toString();
                try {
                    stmt.executeUpdate(sql);
                } catch (SQLException e) {
                    e.printStackTrace();
                }
                stmt.close();
                return true;
            }
        }
        return false;
    }

}
