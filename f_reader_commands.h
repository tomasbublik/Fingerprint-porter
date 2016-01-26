//
// Created by tomas.bublik on 14.12.2015.
//

#ifndef READER_NEWCPPCLASS_H
#define READER_NEWCPPCLASS_H

#include <time.h>
#include <iostream>

#define FINGERPRINT_STARTCODE 0xEF01

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

    void convert(int returnCode, unsigned char *tempBuf, char *string) const;

    int timeDifference(const clock_t begin_time) const;

    void read_template_from_char_buffer(int char_buffer_id, char *dataFromReader);

    bool load_template_to_char_buffer(int char_buffer_id, int page_id);

};


#endif //READER_NEWCPPCLASS_H
