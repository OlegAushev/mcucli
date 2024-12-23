#pragma once


#include "../cli_config.h"
#include <emblib/algorithm.hpp>
#include <emblib/static_vector.hpp>
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


class shell {
    friend int list(int, const char**);
private:
    static inline emb::static_vector<const Cmd*, 16> _commands{};
public:
    static void init(std::initializer_list<const Cmd*> cmds);
    static int exec(int argc, const char** argv);
    static inline char cmd_output_buf[CLI_CMD_OUTPUT_LENGTH]{0};
};


} // namespace cli
