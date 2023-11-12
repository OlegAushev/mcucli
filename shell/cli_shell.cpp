#include "cli_shell.h"
#include "../server/cli_server.h"
#include <iterator>


int cli_sysinfo(int argc, const char** argv);
int cli_reboot(int argc, const char** argv);

int cli_syslog(int argc, const char** argv);
int cli_sysctl(int argc, const char** argv);


namespace cli {


int list(int, const char**);
const Cmd cli_list = {
    .name = "list",
    .exec = list,
    .help = "Prints all available commands."
};


// Cmd shell::_commands[] = {
// {"list",		shell::list, 		"Prints all available commands."},
// {"sysinfo",		cli_sysinfo,		"Prints basic information about system."},
// {"reboot",		cli_reboot,		"Reboots device."},
// {"uptime",		cli_uptime,		"Shows system uptime."},
// {"syslog",		cli_syslog,		"Syslog control utility."},
// {"sysctl",		cli_sysctl,		"System control utility."},
// };


// const size_t shell::command_count = sizeof(shell::_commands) / sizeof(shell::_commands[0]);
// Cmd* shell::_commands_end = shell::_commands + shell::command_count;


void shell::init(std::initializer_list<const Cmd*> cmds) {
    for (const auto& cmd : cmds) {
        _commands.push_back(cmd);
    }
    _commands.push_back(&cli_list);

    std::sort(_commands.begin(), _commands.end());
}


int shell::exec(int argc, const char** argv) {
    if (argc == 0) return 0;

    //const Cmd** cmd = emb::binary_find(_commands.begin(), _commands.end(), argv[0]);
    
    const Cmd** cmd = std::find_if(_commands.begin(), _commands.end(), [argv](const Cmd* cmd_){
        return strcmp(argv[0], cmd_->name) == 0;
    });

    if (cmd == _commands.end()) {
        cli::nextline();
        cli::print(argv[0]);
        cli::print(": command not found");
        return -1;
    }

    if (argc == 1) {
        return (*cmd)->exec(--argc, ++argv);
    } else if (strcmp(argv[1], "--help") == 0) {
        cli::nextline();
        cli::print((*cmd)->help);
        return 0;
    } else {
        return (*cmd)->exec(--argc, ++argv);
    }
}


int list(int argc, const char** argv) {
    cli::nextline();
    cli::print("Available commands are:");
    for (size_t i = 0; i < shell::_commands.size(); ++i) {
        cli::nextline();
        cli::print(shell::_commands[i]->name);
    }
    return 0;
}


} // namespace cli
