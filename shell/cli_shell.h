#pragma once


#include "../cli_config.h"
#include <emblib/algorithm.h>
#include <emblib/static_vector.h>
#include <algorithm>
#include <initializer_list>


#define CLI_CMD_OUTPUT_LENGTH CLI_OUTBUT_BUFFER_LENGTH
extern char cli_cmd_output[CLI_CMD_OUTPUT_LENGTH];


namespace cli {


struct Cmd {
    const char* name;
    int (*exec)(int argc, const char** argv);
    const char* help;
};


// inline bool operator<(const Cmd& lhs, const Cmd& rhs) {
//     return strcmp(lhs.name, rhs.name) < 0;
// }


// inline bool operator<(const char* name, const Cmd& cmd) {
//     return strcmp(name, cmd.name) < 0;
// }


// inline bool operator==(const char* name, const Cmd& cmd) {
//     return strcmp(name, cmd.name) == 0;
// }


class shell {
    friend int list(int, const char**);
private:
    static inline emb::static_vector<const Cmd*, 16> _commands{};
public:
    static void initialize(std::initializer_list<const Cmd*> cmds);
    static int exec(int argc, const char** argv);
    static inline char cmd_output_buf[CLI_CMD_OUTPUT_LENGTH]{0};
};


} // namespace cli
