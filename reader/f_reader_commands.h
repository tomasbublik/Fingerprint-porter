//
// Created by tomas.bublik on 14.12.2015.
//

#ifndef READER_NEWCPPCLASS_H
#define READER_NEWCPPCLASS_H

#include <time.h>
#include <iostream>

#define FINGERPRINT_STARTCODE 0xEF01

static const int HEADER_HIGH = 0xEF;
static const int HEADER_LOW = 0x01;

static const uint32_t READER_ADDRESS = 0xFFFFFFFF;

static const int PACKAGE_IDENTIFIER = 0x01;
using namespace std;

class f_reader_commands {

private:
    int com_port;
    unsigned char buf[4096];
public:
    f_reader_commands(int);

    f_reader_commands() {
    }

    ~f_reader_commands() {
    }

    void initialize();

    void handshake();

    void print_system_parameters();

    void detect_fingerprint(int round);

    bool match_both_characters_file_to_template();

    bool store_to_memory(int page_id);

    void upload_char(int id);

    int search();

    void sleepy(unsigned long timing);

    void move_to_char_buffer(int round);

    void delete_all_fingers();

    bool read_notepad(char *data_from_notepad);

    bool write_notepad(unsigned char *data_to_notepad);

    void convert(int returnCode, unsigned char *tempBuf, char *dest) const;

    int timeDifference(const clock_t begin_time) const;

    void read_template_from_char_buffer(int char_buffer_id, char *dataFromReader);

    bool load_template_to_char_buffer(int char_buffer_id, int page_id);

    bool write_template_to_reader(int page_id, unsigned char *dataFromReader);
};


#endif //READER_NEWCPPCLASS_H
