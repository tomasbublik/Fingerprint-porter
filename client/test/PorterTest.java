/**
 * Created by tomas.bublik on 1.11.2015.
 */

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;

import static org.junit.Assert.*;

public class PorterTest {

    private static Connection c = null;

    @BeforeClass
    public static void createConnection() {
        System.out.println("Starting database connection");
        inner("CREATE TABLE IF NOT EXISTS porter (id INTEGER PRIMARY KEY ASC, action TEXT, status TEXT, TIMESTAMP TEXT, humanid TEXT)");
    }

    @AfterClass
    public static void cleanUp() {
        System.out.println("Cleaning up");
        inner("DELETE FROM porter");
    }

    public static void inner(String command) {
        c = Porter.getConnection(c);
        try {
            Statement stmt = c.createStatement();
            String sql = command;
            stmt.executeUpdate(sql);
            stmt.close();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    @Test
    public void testSavePositive() {
        try {
            assertTrue(Porter.saveString(c, "ENTRANCE|VENKOVNI|2015-09-15T13:33:01|098765"));
        } catch (SQLException e) {
            fail();
        }
    }

    @Test
    public void testSaveNegative() {
        try {
            assertFalse(Porter.saveString(c, "ENTRANCE|2015-09-15T13:33:01|098765"));
        } catch (SQLException e) {
            fail();
        }
    }


    @Test
    public void testSavePerformance() {
        try {
            for (int i = 0; i < 1000; i++) {
                final String userInput = "ENTRANCE|VENKOVNI|2015-09-15T13:33:01|" + i;
                assertTrue(Porter.saveString(c, userInput));
            }
        } catch (SQLException e) {
            fail();
        }
    }

    @Test
    public void testSaveDoorSwitch() {
        try {
            assertTrue(Porter.saveString(c, "DOOR_ACTIVATION|CLOSED|2016-01-21 21:27:13|DOOR"));
        } catch (SQLException e) {
            fail();
        }
    }

}
