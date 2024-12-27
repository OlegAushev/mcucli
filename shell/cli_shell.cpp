#include "cli_shell.hpp"
#include "../server/cli_server.hpp"
#include <iterator>

int cli_sysinfo(int argc, const char** argv);
int cli_reboot(int argc, const char** argv);

int cli_syslog(int argc, const char** argv);
int cli_sysctl(int argc, const char** argv);

namespace cli {

int list(int, const char**);
const Cmd cli_list = {
    .name = "list", .exec = list, .help = "Prints all available commands."};

void shell::init(std::initializer_list<const Cmd*> cmds) {
    for (const auto& cmd : cmds) {
        _commands.push_back(cmd);
    }
    _commands.push_back(&cli_list);

    std::sort(_commands.begin(), _commands.end());
}

int shell::exec(int argc, const char** argv) {
    if (argc == 0)
        return 0;

    //const Cmd** cmd = emb::binary_find(_commands.begin(), _commands.end(), argv[0]);

    const Cmd** cmd = std::find_if(
            _commands.begin(), _commands.end(), [argv](const Cmd* cmd_) {
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
