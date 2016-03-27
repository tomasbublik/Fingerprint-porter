#include <iostream>

#ifdef _WIN32

#include <Windows.h>

#else
#include <unistd.h>
#endif

#include "rs232.h"
#include "f_reader_commands.h"
#include <iomanip>
#include "sqlite3.h"


bool loading_process(f_reader_commands &commandsLib, int pageId, const sqlite3 *db, int rc);

void searching_process(f_reader_commands &commandsLib, sqlite3 *db, int rc, char *type);

bool synchronization_process(f_reader_commands &commandsLib, sqlite3 *db, int rc);

// Its callback function is invoked for each result row coming out of the evaluated SQL statement.
static int callback(void *NotUsed, int argc, char **argv, char **azColName);

void open_door(char *name, char *type);

void get_time(const char *buffer);

void select_all(sqlite3 *db, int rc);

void delete_all(sqlite3 *db, int rc, f_reader_commands &commandsLib);

void select_precise_fingerprint(sqlite3 *db, int rc, char *name, int pageId);

void updateNewest(sqlite3 *db, int rc, f_reader_commands &commandsLib, char *dateString);

void create_db(sqlite3 *db, int rc);

int saveToDb(const sqlite3 *db, int rc, int pageId, char *dataFromReader);

bool removing_process(f_reader_commands &commandsLib, int pageId, sqlite3 *db, int rc);

void delete_one(sqlite3 *db, int rc, int pageId);

uint8_t *
        hex_decode(const char *in, size_t len, uint8_t *out);

int hex_to_ascii(char c, char d);

using namespace std;

/**
 * Running with following arguments:
 * <port_number> <type> <master_or_slave_mode>
 *
 * <port_number> if from: doc.txt
 * <type> String identification of the reader
 * <master_or_slave_mode> optional definition for user management console in the case of MASTER mode
 * or synchronization process in the case of SLAVE mode
 */
int main(int argc, char *argv[]) {
    int i, n,
            cport_nr = 1,        /* /dev/ttyS0 (COM1 on windows) */
            bdrate = 115200;       /* 115200 baud */
    bool slaveMode = false; /*Only slave mode of program can synchronize with database before start*/
    bool masterMode = false; /*Only master mode of program can write to database and manage users*/
    char type[20] = "0000000\0";

    if (argc >= 3 && argc < 5) {
        cport_nr = atoi(argv[1]);
        printf("This program runs with following port: %s \n", argv[1]);
        if (argc == 4) {
            if (0 == strcmp("SLAVE", argv[3])) {
                cout << "Running in SLAVE mode" << endl;
                slaveMode = true;
            }
            if (0 == strcmp("MASTER", argv[3])) {
                cout << "Running in MASTER mode" << endl;
                masterMode = true;
            }
        }
        strcpy(type, argv[2]);
        //type[sizeof argv[2]] = '\0';
    } else if (argc > 4) {
        printf("Too many arguments supplied.\n");
    }
    else {
        printf("Warning, this program runs with default COM2 on Windows or ttyS1 on Linux \n");
    }
    char mode[] = {'8', 'N', '1', 0};

    if (RS232_OpenComport(cport_nr, bdrate, mode)) {
        printf("Can not open com port\n");
        return (0);
    }

    sqlite3 *db;
    int rc;

    rc = sqlite3_open("finger.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    create_db(db, rc);
    f_reader_commands serial_commands = f_reader_commands(cport_nr);

    if (slaveMode) {
        synchronization_process(serial_commands, db, rc);
    }
    char temp[2];
    while (temp[0] != 'q') {
        printf("Guide: \n");
        if (masterMode) {
            printf("(1) for operating mode, (2) to set user, (3) to delete user, \n "
                           "(4) to delete all, (5) for database list, (q) for quit \n");
            scanf("%s", temp);
        }

        if (temp[0] == '1' || !masterMode) {
            printf("Entering normal mode \n");
            searching_process(serial_commands, db, rc, type);
        }
        if (temp[0] == '2') {
            printf("User setting mode \n");
            printf("Enter page ID: \n");
            int pageId;
            scanf("%d", &pageId);
            //Because scanf() ignores the trailing newline
            getchar();
            bool success = loading_process(serial_commands, pageId, db, rc);
        }
        if (temp[0] == '3') {
            printf("User delete mode \n");
            printf("Enter page ID: \n");
            int pageId;
            cin >> pageId;
            bool success = removing_process(serial_commands, pageId, db, rc);
        }
        if (temp[0] == '5') {
            printf("List database: \n");
            select_all(db, rc);
        }
        if (temp[0] == '4') {
            printf("Erase fingerprint memory: \n");
            delete_all(db, rc, serial_commands);
        }
        if (temp[0] == 'q') {
            sqlite3_close(db);
            printf("Exiting: \n");
        } else {
            temp[0] = (char) 'a';
        }
    }

    /*f_reader_commands.handshake();*/
    //f_reader_commands.print_system_parameters();
    //f_reader_commands.initialize();
    //f_reader_commands.upload_char(1);

    return (0);
}

int saveToDb(const sqlite3 *db, int rc, int pageId, char *dataFromReader) {
    printf("Enter name: \n");
    //cout << "Enter name:" << endl;
    string fingerName;
    cin >> fingerName;

    char *zErrMsg = 0;
    string sql;
    char pageIdBuff[10];
    sprintf(pageIdBuff, "%d", pageId);

    char buffer[80];
    get_time(buffer);

    /* Create SQL statement */
    sql = "INSERT OR REPLACE INTO FINGERS (ID, NAME, STATUS, DATA, DATE) "  \
         "VALUES (" + string(pageIdBuff) + ", '" + fingerName + "', 'ACTIVE','" + dataFromReader + "','" + buffer +
          "'); ";

    /* Execute SQL statement */
    rc = sqlite3_exec((sqlite3 *) db, sql.c_str(), callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Records created successfully\n");
    }
    return rc;
}

void select_all(sqlite3 *db, int rc) {
    char *zErrMsg = 0;
    char *sql;
    const char *data = "Callback function called";
    /* Create SQL statement */
    sql = (char *) "SELECT * from FINGERS";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void *) data, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
}

void delete_all(sqlite3 *db, int rc, f_reader_commands &commandsLib) {
    commandsLib.delete_all_fingers();

    char *zErrMsg = 0;
    char *sql;
    const char *data = "Callback function called";
    /* Create SQL statement */
    sql = (char *) "UPDATE FINGERS SET STATUS='DELETED'";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void *) data, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
}

void delete_one(sqlite3 *db, int rc, int pageId) {
    sqlite3_stmt *res;
    /* Create SQL statement */
    char *sql = (char *) "UPDATE FINGERS SET STATUS='DELETED', DATA='' WHERE ID = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(res, 1, pageId);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
}

void select_precise_fingerprint(sqlite3 *db, int rc, char *name, int pageId) {
    sqlite3_stmt *res;

    char *sql = (char *) "SELECT ID, NAME FROM FINGERS WHERE ID = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(res, 1, pageId);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    int step = sqlite3_step(res);

    if (step == SQLITE_ROW) {
        printf("%s: ", sqlite3_column_text(res, 0));
        const unsigned char *textName = sqlite3_column_text(res, 1);
        printf("%s\n", textName);
        size_t length = strlen((const char *) textName) + 1;
        strncpy(name, (const char *) textName, length);
        name[length - 1] = '\0';
    } else {
        size_t length = 8;
        strncpy(name, "Unknown", length);
        name[length - 1] = '\0';
    }

    sqlite3_finalize(res);
}


void updateNewest(sqlite3 *db, int rc, f_reader_commands &commandsLib, char *dateString) {
    sqlite3_stmt *res;

    //char *sql = "SELECT ID, DATA, STATUS FROM FINGERS WHERE DATE > ?";
    char *sql = (char *) "SELECT ID, DATA, STATUS FROM FINGERS WHERE datetime(DATE) >= datetime(?)";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
//TODO - no idea how to preserve the dateString value otherwise!!!
    /*char name[20];
    size_t length = 20;
    strncpy(name, (const char *) dateString, 19);
    name[length - 1] = '\0';
*/
    if (rc == SQLITE_OK) {
        //const char *dateString = "2016-01-26T19:26:10";
        sqlite3_bind_text(res, 1, dateString, strlen(dateString), 0);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    for (; ;) {
        int step = sqlite3_step(res);

        if (step == SQLITE_DONE)
            break;
        if (step != SQLITE_ROW) {
            printf("error: %s!\n", sqlite3_errmsg(db));
            break;
        }

        if (step == SQLITE_ROW) {
            int id = sqlite3_column_int(res, 0);
            printf("%i: ", id);
            const unsigned char *finger_data = sqlite3_column_text(res, 1);
            printf("%s: ", finger_data);
            const unsigned char *status = sqlite3_column_text(res, 2);
            printf("%s: ", status);

            if (0 == strcmp("DELETED", (const char *) status)) {
                printf("\n Going to remove the newly deleted user with ID: %i \n", id);
                bool success = removing_process(commandsLib, id, db, rc);
            } else {
                printf("\n Going to write the newly added user with ID: %i \n", id);
                bool success = commandsLib.write_template_to_reader(id, (unsigned char *) finger_data);
            }

        }
        printf("\n");
    }

    sqlite3_finalize(res);
}


void create_db(sqlite3 *db, int rc) {
    char *zErrMsg = 0;
    char *sql;
/* Create SQL statement */
    sql = (char *) "CREATE TABLE IF NOT EXISTS FINGERS("  \
             "ID INT PRIMARY KEY     NOT NULL," \
             "NAME           TEXT    NOT NULL," \
             "STATUS         TEXT," \
                "DATA         TEXT," \
             "DATE         TEXT );";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Table created successfully\n");
    }
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void searching_process(f_reader_commands &commandsLib, sqlite3 *db, int rc, char *type) {
    bool success = false;
    while (!success) {
        commandsLib.detect_fingerprint(1);
        int result = commandsLib.search();
        if (result != -1) {
            char name[200];
            select_precise_fingerprint(db, rc, name, result);
            open_door(name, type);
        }
        printf("\n ");
        /* //cout << endl;
         printf("\n Press any key to continue. . . \n");
         //cout << "Press any key to continue. . .\n";
         cin.get(); //Or "getch()"*/
    }
}

bool synchronization_process(f_reader_commands &commandsLib, sqlite3 *db, int rc) {
    char date_from_reader[20];

    commandsLib.read_notepad(date_from_reader);

    //It's gonna be just page id 2 to test:
    updateNewest(db, rc, commandsLib, date_from_reader);

    char buffer[80];
    get_time(buffer);
    uint8_t res[7];
    hex_decode(buffer, strlen(buffer), res);
    //printf("%s", res);

    commandsLib.write_notepad(res);
}

uint8_t *
hex_decode(const char *in, size_t len, uint8_t *out) {
    unsigned int i, t, hn, ln;
    for (t = 0, i = 0; i < len; i += 2, ++t) {
        if (in[i] == 'T' | in[i] == '-' | in[i] == ':') {
            t--;
            i--;
            continue;
        }
        hn = in[i] > '9' ? in[i] - 'A' + 10 : in[i] - '0';
        ln = in[i + 1] > '9' ? in[i + 1] - 'A' + 10 : in[i + 1] - '0';

        out[t] = (hn << 4) | ln;
    }

    return out;
}

void open_door(char *name, char *type) {
    int timing = 2000;
#ifdef _WIN32
    //nothing to do here, it's just for RPi
    Sleep(timing);
#else
    system("echo 0 > /sys/class/gpio/gpio26/value");
    usleep(timing*1000);  /* sleep for 100 milliSeconds */
    system("echo 1 > /sys/class/gpio/gpio26/value");
#endif
    char buffer[80];
    get_time(buffer);
    printf("ENTRANCE|%s|%s|%s", type, buffer, name);
}

void get_time(const char *buffer) {
    time_t rawTime;
    struct tm *timeInfo;

    time(&rawTime);
    timeInfo = localtime(&rawTime);
    printf("Current time: %s", asctime(timeInfo));

    strftime((char *) buffer, 80, "%Y-%m-%dT%H:%M:%S", timeInfo);
}

bool loading_process(f_reader_commands &commandsLib, int pageId, const sqlite3 *db, int rc) {
    char temp[2];
    temp[0] = 'a';
    while (temp[0] != '1') {
        commandsLib.detect_fingerprint(1);
        //commandsLib.upload_char(1);
        printf("Press any key to continue. . . \n");
        getchar();

        commandsLib.detect_fingerprint(2);
        //commandsLib.upload_char(1);

        bool resultMatching = commandsLib.match_both_characters_file_to_template();

        if (resultMatching) {
            bool storeSuccess = commandsLib.store_to_memory(pageId);
            if (storeSuccess) {
                char dataFromReader[2000];
                commandsLib.read_template_from_char_buffer(1, dataFromReader);
                printf("Data from reader: %s \n", dataFromReader);

                rc = saveToDb(db, rc, pageId, dataFromReader);
            }
        }

        printf("(1) end, (*) repeat \n");
        scanf("%s", temp);
        //Because scanf() ignores the trailing newline
        getchar();
    }
    return true;
}

bool removing_process(f_reader_commands &commandsLib, int pageId, sqlite3 *db, int rc) {
    printf("Removing from database. \n");
    commandsLib.delete_one_finger(pageId);

    delete_one(db, rc, pageId);

    return true;
}


