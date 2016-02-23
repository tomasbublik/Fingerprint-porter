//
// Created by tomas.bublik on 14.12.2015.
//

#include <iostream>
#include <time.h>
#include "f_reader_commands.h"
#include "rs232.h"

void cstringcpy(char *src, char *dest);

f_reader_commands::f_reader_commands(int i) {
    com_port = i;
}

void f_reader_commands::print_system_parameters() {
    //read system parameters - works perfectly well
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x03,
                                    0x0f, 0x00,
                                    0x13};

    const clock_t begin_time = clock();
    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);

        if (returnCode > 0) {
            printf("Time execution in ms: %d \n", timeDifference(begin_time));
            //cout << "ms time execution: " << float(clock() - begin_time) / CLOCKS_PER_SEC << endl;
            char *converted = NULL;
            convert(returnCode, buf, converted);
            return;
        }
        sleepy(10);
    }

}

void f_reader_commands::detect_fingerprint(int round) {
    //detect and store finger
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x03,
                                    0x01, 0x00,
                                    0x05};
    cout << "Reading fingerprint into chr buffer no.: " << round << "..." << endl;
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
        while (1) {
            returnCode = RS232_PollComport(com_port, buf, 4095);
            if (returnCode > 0) {
                //0x02 means cannot detect finger
                if (buf[9] != 0x02) {
                    char *converted = NULL;
                    convert(returnCode, buf, converted);
                    printf("%s\n", "Successfully loaded into image buffer");
                    //0x00 means success read into imgBuffer
                    if (buf[9] == 0x00) {
                        move_to_char_buffer(round);
                        return;
                    } else {
                        if (buf[9] == 0x01) {
                            memset(&buf[0], 0, sizeof(buf));
                            printf("%s\n", "Error when receiving package");
                            break;
                        }
                    }
                } else {
                    //Finger wasn't detected, lest try it repeatedly
                    break;
                }
            }
            sleepy(40);
        }
        sleepy(200);
    }
}

void f_reader_commands::move_to_char_buffer(int round) {
    cout << "Saving to char buffer..." << endl;
    //now we gonna generate char file and store it into charBuffer1
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x04,
                                    0x02,
                                    (unsigned char) round, 0x00,
                                    (unsigned char) (0x01 + 0x04 + 0x02 + round)};
    const clock_t begin_time = clock();
    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            printf("Time execution in ms: %d \n", timeDifference(begin_time));
            //cout << "ms time execution: " << float(clock() - begin_time) / CLOCKS_PER_SEC << endl;
            char *converted = NULL;
            convert(returnCode, buf, converted);
            printf("Result of round number: %d \n", round);
            //cout << "Vysledek z kola: " << round << endl;
            if (buf[9] != 0x00) {
                printf("Bad :-( \n");
                //cout << "Spatny :-(" << endl;
            } else {
                printf("OK :-) \n");
                //cout << "To by slo:-)" << endl;
            }
            return;
        }
        memset(&buf[0], 0, sizeof(buf));
        sleepy(10);
    }
}


void f_reader_commands::delete_all_fingers() {
    printf("Uwaga, uwaga! All the fingeprint records will be deleted!  \n");
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x03,
                                    0x0D, 0x11};
    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted = NULL;
            convert(returnCode, buf, converted);
            if (buf[9] == 0x00) {
                printf("All the fingeprint records were removed \n");
            } else {
                printf("Error occurred during the fingerprint records erasing \n");
            }
            return;
        }
        sleepy(10);
    }
}

bool f_reader_commands::match_both_characters_file_to_template() {
    cout << "Matching..." << endl;
//To combine information of character files from CharBuffer1 and CharBuffer2 and
//generate a template which is stored back in both CharBuffer1 and CharBuffer2.
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x03,
                                    0x05, 0x00,
                                    0x09};

    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted = NULL;
            convert(returnCode, buf, converted);
            if (buf[9] == 0x00) {
                printf("Positive \n");
                //cout << "Pozitivni" << endl;
                return true;
            } else {
                printf("Unfortunately, must be repeated \n");
                //cout << "Bohuzel, je nutne opakovat" << endl;
                return false;
            }
        }
        sleepy(10);
    }
}

void f_reader_commands::read_template_from_char_buffer(int char_buffer_id, char *dataFromReader) {
    printf("Reading data from char buffer id:%i \n", char_buffer_id);

    int checksum = 0x01 + 0x04 + 0x08 + char_buffer_id;
    unsigned char checksumBytes[2];
    checksumBytes[0] = (unsigned char) ((checksum >> 8) & 0xFF);
    checksumBytes[1] = (unsigned char) (checksum & 0xFF);

    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x04,
                                    0x08,
                                    (unsigned char) char_buffer_id, checksumBytes[0], checksumBytes[1]};
    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        sleepy(10);
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char converted[returnCode * 2 + 1];
            convert(returnCode, buf, converted);
            printf("received %i bytes \n", returnCode);
            //printf("%s\n", converted);
            if (buf[9] == 0x00) {
                printf("Awaiting data packet \n");
                memcpy(dataFromReader, &converted[24], sizeof converted);
                dataFromReader[sizeof converted - 1] = '\0';
                /*strncpy(dataFromReader, converted, sizeof converted);
                dataFromReader[sizeof converted -1] = '\0';*/
            } else {
                printf("Instruction error \n");
            }
            return;
        }
    }
}

bool f_reader_commands::write_template_to_reader(int page_id, unsigned char *dataFromReader) {
    printf("Writing data to char buffer 1 and then to page id:%i \n", page_id);

    int checksum = 0x01 + 0x04 + 0x09 + 0x01;
    unsigned char checksumBytes[2];
    checksumBytes[0] = (unsigned char) ((checksum >> 8) & 0xFF);
    checksumBytes[1] = (unsigned char) (checksum & 0xFF);
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x04,
                                    0x09,
                                    0x01, checksumBytes[0], checksumBytes[1]};

    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        sleepy(10);
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted = NULL;
            convert(returnCode, buf, converted);
            if (buf[9] == 0x00) {
                printf("Going to store the data \n");
                int returnCode = RS232_SendBuf(com_port, (unsigned char *) dataFromReader, sizeof(packetBuffer));
                sleepy(100);
                bool result = store_to_memory(page_id);
                return result;
            } else {
                printf("Instruction error \n");
            }
            return false;
        }
    }
}


bool f_reader_commands::load_template_to_char_buffer(int char_buffer_id, int page_id) {
    printf("Loading data from template page id: %i char buffer id:%i \n", page_id, char_buffer_id);

    unsigned char pageIdBytes[2];
    pageIdBytes[0] = (unsigned char) ((page_id >> 8) & 0xFF);
    pageIdBytes[1] = (unsigned char) (page_id & 0xFF);

    int checksum = 0x01 + 0x06 + 0x07 + char_buffer_id + pageIdBytes[0] + pageIdBytes[1];
    unsigned char checksumBytes[2];
    checksumBytes[0] = (unsigned char) ((checksum >> 8) & 0xFF);
    checksumBytes[1] = (unsigned char) (checksum & 0xFF);

    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x06,
                                    0x07,
                                    (unsigned char) char_buffer_id, pageIdBytes[0], pageIdBytes[1], checksumBytes[0],
                                    checksumBytes[1]};
    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        sleepy(10);
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted = NULL;
            convert(returnCode, buf, converted);
            if (buf[9] == 0x00) {
                printf("Successfully loaded \n");
                memset(&buf[0], 0, sizeof(buf));
                return true;
            } else {
                printf("Loading error error \n");
                return false;
            }
        }
    }
}


bool f_reader_commands::store_to_memory(int page_id) {
    cout << "Storing to page id: " << page_id << "..." << endl;

    unsigned char pageIdBytes[2];
    pageIdBytes[0] = (unsigned char) ((page_id >> 8) & 0xFF);
    pageIdBytes[1] = (unsigned char) (page_id & 0xFF);

    int checksum = 0x01 + 0x06 + 0x06 + 0x01 + pageIdBytes[0] + pageIdBytes[1];
    unsigned char checksumBytes[2];
    checksumBytes[0] = (unsigned char) ((checksum >> 8) & 0xFF);
    checksumBytes[1] = (unsigned char) (checksum & 0xFF);

    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x06,
                                    0x06, 0x01,
                                    pageIdBytes[0], pageIdBytes[1], checksumBytes[0], checksumBytes[1]};

    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted = NULL;
            convert(returnCode, buf, converted);
            if (buf[9] != 0x00) {
                printf("Error. Not saved. \n");
                //cout << "Error. Not saved." << endl;
            } else {
                printf("Saved successfully \n");
                //cout << "Ulozeno" << endl;
                return true;
            }
            return false;
        }
        sleepy(10);
    }
}

bool f_reader_commands::delete_one_finger(int page_id) {
    cout << "The template with page id: " << page_id << " will be deleted" << endl;

    unsigned char pageIdBytes[2];
    pageIdBytes[0] = (unsigned char) ((page_id >> 8) & 0xFF);
    pageIdBytes[1] = (unsigned char) (page_id & 0xFF);

    int checksum = PACKAGE_IDENTIFIER + 0x07 + 0x0C + pageIdBytes[0] + pageIdBytes[1] + 0x01;
    unsigned char checksumBytes[2];
    checksumBytes[0] = (unsigned char) ((checksum >> 8) & 0xFF);
    checksumBytes[1] = (unsigned char) (checksum & 0xFF);

    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x07,
                                    0x0C, pageIdBytes[0], pageIdBytes[1], 0x00, 0x01, checksumBytes[0], checksumBytes[1]};

    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted = NULL;
            convert(returnCode, buf, converted);
            if (buf[9] != 0x00) {
                printf("Error. Cannot be deleted. \n");
                //cout << "Error. Not saved." << endl;
            } else {
                printf("Deleted successfully \n");
                //cout << "Ulozeno" << endl;
                return true;
            }
            return false;
        }
        sleepy(10);
    }
}

void f_reader_commands::upload_char(int id) {
    cout << "Uploading char..." << endl;
    //upload char buffer
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x04,
                                    0x08,
                                    (unsigned char) id,
                                    0x00,
                                    (unsigned char) (0x01 + 0x04 + 0x08 + id)};

    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);

        if (returnCode > 0) {
            buf[returnCode] = 0;   /* always put a "null" at the end of a string! */
            int size = 568;
            char converted[size * 2 + 1];
            int i;

            for (i = 0; i < size; i++) {
                sprintf(&converted[i * 2], "%02X", buf[i]);
            }
            printf("Char data: \n");
            //cout << "Char data: " << endl;
            printf("%s\n", converted);

            //printf("received %i bytes: %s\n", returnCode, (char *) buf);
            printf("received %i bytes \n", returnCode);
            return;
        }
        sleepy(10);
    }
}

int f_reader_commands::search() {
    printf("Searching finger... \n");
    //cout << "Searching finger..." << endl;
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x08,
                                    0x1B, 0x01, 0x00,
                                    0x00, 0x03,
                                    0xE9, 0x01, 0x11};
    /*unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,                                     (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER, 0x00, 0x08, 0x04, 0x01, 0x00, 0x00, 0x03,
                                    0xE9, 0x00, 0xFA};*/
    const clock_t begin_time = clock();
    int timeoutInSeconds = 1000;
    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        float time = timeDifference(begin_time);
        if (returnCode > 0) {
            printf("Time execution in ms: %d \n", timeDifference(begin_time));
            //cout << "ms time execution: " << time << endl;
            char *converted = NULL;
            convert(returnCode, buf, converted);
            if (buf[9] == 0x09) {
                printf("Not found \n");
                //cout << "Nenalezeno" << endl;
                return -1;
            } else {
                unsigned char pageId = buf[10] + buf[11];
                printf("Found id: %i  with score: %i \n", pageId, buf[12] + buf[13]);
                return pageId;
            }
        }
        if (time >= timeoutInSeconds) {
            printf("Timeout occurred \n");
            //cout << "Timeout occurred" << endl;
            return -1;
        }
        sleepy(10);
    }
}

int f_reader_commands::timeDifference(const clock_t begin_time) const { return (int) (float(clock() - begin_time)); }

void f_reader_commands::sleepy(unsigned long timing) {
#ifdef _WIN32
    Sleep(timing);
#else
    usleep(timing*1000);  /* sleep for 100 milliSeconds */
#endif
}

void f_reader_commands::handshake() {
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x04,
                                    0x17, 0x00, 0x00,
                                    0x1C};
    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    printf("Vysledek: %d\n", returnCode);
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted = NULL;
            convert(returnCode, buf, converted);
            printf("Handshake response: \n");
            //cout << "Handshake response: " << endl;
            //printf("received %i bytes: %s\n", returnCode, (char *) buf);
            printf("received %i bytes \n", returnCode);
            return;
        }

#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);  /* sleep for 100 milliSeconds */
#endif
    }
}

void f_reader_commands::initialize() {
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x07,
                                    0x13, 0x00, 0x00,
                                    0x00, 0x00,
                                    0x00, 0x1B};

    int timeoutInSeconds = 1000;
    memset(&buf[0], 0, sizeof(buf));
    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    const clock_t begin_time = clock();
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        float time = float(clock() - begin_time);
        if (returnCode > 0) {
            buf[returnCode] = 0;   /* always put a "null" at the end of a string! */
            char *converted;
            convert(returnCode, buf, converted);
            break;
        }
        if (time >= timeoutInSeconds) {
            printf("Timeout occurred \n");
            //cout << "Timeout occurred" << endl;
            break;
        }
        sleepy(10);
    }
    memset(&buf[0], 0, sizeof(buf));
    sleepy(1000);

    unsigned char packetBufferPasswd[] = {};

    returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    sleepy(200);

    unsigned char packetBuffer2[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                     (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                     0x00, 0x07,
                                     0x13, 0x00,
                                     0x00, 0x00, 0x00,
                                     0x00, 0x1B};

    memset(&buf[0], 0, sizeof(buf));
    int returnCode2 = RS232_SendBuf(com_port, packetBuffer2, sizeof(packetBuffer2));
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted;
            convert(returnCode, buf, converted);
            break;
        }
        sleepy(10);
    }
    memset(&buf[0], 0, sizeof(buf));
    sleepy(300);

    unsigned char packetBuffer3[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                     (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                     0x00, 0x03,
                                     0x16, 0x00,
                                     0x1A};

    int returnCode3 = RS232_SendBuf(com_port, packetBuffer3, sizeof(packetBuffer3));
    sleepy(200);
    while (1) {
        returnCode = RS232_PollComport(com_port, buf, 4095);
        printf("Return code %d \n", returnCode);
        //cout << "Return code: " << returnCode << endl;
        if (returnCode > 0) {
            char *converted;
            convert(returnCode, buf, converted);
            break;
        }
        sleepy(10);
    }
}

void f_reader_commands::convert(int returnCode, unsigned char tempBuf[4096], char *dest) const {
    tempBuf[returnCode] = 0;   /* always put a "null" at the end of a dest! */
    int size = returnCode;
    char converted[size * 2 + 1];
    int i;

    for (i = 0; i < size; i++) {
        sprintf(&converted[i * 2], "%02X", buf[i]);
    }

    printf("Response: \n");
    printf("%s\n", converted);
    if (dest != NULL) {
        strncpy(dest, converted, sizeof converted);
        dest[sizeof converted - 1] = '\0';
    }

    /*char dest[size * 2 + 1];
    strncpy(dest, converted, sizeof dest);
    dest[sizeof dest - 1] = '\0';
*/
    /*for (int i = 0; i < sizeof dest; i++) {
        dest[i] = dest[i];
    }*/
    //printf("received %i bytes: %s\n", returnCode, (char *) buf);
    //printf("received %i bytes \n", returnCode);
}

bool f_reader_commands::read_notepad(char *data_from_notepad) {
    printf("Reading notepad data");

    int checksum = PACKAGE_IDENTIFIER + 0x04 + 0x19;
    unsigned char checksumBytes[2];
    checksumBytes[0] = (unsigned char) ((checksum >> 8) & 0xFF);
    checksumBytes[1] = (unsigned char) (checksum & 0xFF);
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x04,
                                    0x19,
                                    0x00,
                                    checksumBytes[0], checksumBytes[1]};

    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        sleepy(100);
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char converted[returnCode * 2 + 1];
            convert(returnCode, buf, converted);
            if (buf[9] == 0x00) {
                printf("Successfully read \n");
                data_from_notepad[0] = converted[20];
                data_from_notepad[1] = converted[21];
                data_from_notepad[2] = converted[22];
                data_from_notepad[3] = converted[23];
                data_from_notepad[4] = '-';
                data_from_notepad[5] = converted[24];
                data_from_notepad[6] = converted[25];
                data_from_notepad[7] = '-';
                data_from_notepad[8] = converted[26];
                data_from_notepad[9] = converted[27];
                data_from_notepad[10] = 'T';
                data_from_notepad[11] = converted[28];
                data_from_notepad[12] = converted[29];
                data_from_notepad[13] = ':';
                data_from_notepad[14] = converted[30];
                data_from_notepad[15] = converted[31];
                data_from_notepad[16] = ':';
                data_from_notepad[17] = converted[32];
                data_from_notepad[18] = converted[33];
                return true;
            } else {
                printf("Instruction error \n");
            }
            return false;
        }
    }
}

bool f_reader_commands::write_notepad(unsigned char *data_to_notepad) {
    printf("Writing data notepad");

    int checksum = PACKAGE_IDENTIFIER + 0x24 + 0x18 + data_to_notepad[0] +
                   data_to_notepad[1] +
                   data_to_notepad[2] +
                   data_to_notepad[3] +
                   data_to_notepad[4] +
                   data_to_notepad[5] +
                   data_to_notepad[6];
    unsigned char checksumBytes[2];
    checksumBytes[0] = (unsigned char) ((checksum >> 8) & 0xFF);
    checksumBytes[1] = (unsigned char) (checksum & 0xFF);
    unsigned char packetBuffer[] = {HEADER_HIGH, HEADER_LOW, READER_ADDRESS & 0xFF, (READER_ADDRESS >> 8) & 0xFF,
                                    (READER_ADDRESS >> 16) & 0xFF, (READER_ADDRESS >> 24) & 0xFF, PACKAGE_IDENTIFIER,
                                    0x00, 0x24,
                                    0x18,
                                    0x00, data_to_notepad[0],
                                    data_to_notepad[1],
                                    data_to_notepad[2],
                                    data_to_notepad[3],
                                    data_to_notepad[4],
                                    data_to_notepad[5],
                                    data_to_notepad[6],
                                    0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00,
                                    checksumBytes[0], checksumBytes[1]};

    int returnCode = RS232_SendBuf(com_port, packetBuffer, sizeof(packetBuffer));
    memset(&buf[0], 0, sizeof(buf));
    while (1) {
        sleepy(100);
        returnCode = RS232_PollComport(com_port, buf, 4095);
        if (returnCode > 0) {
            char *converted = NULL;
            convert(returnCode, buf, converted);
            if (buf[9] == 0x00) {
                printf("Successfully written \n");
                return true;
            } else {
                printf("Instruction error \n");
            }
            return false;
        }
    }
}
