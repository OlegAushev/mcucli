#include "cli_server.hpp"

namespace cli {

void server::_esc_return() {
#ifdef CLI_USE_HISTORY
    if (!_cmdline.empty()) {
        if (_history.empty()) {
            _history.push_back(_cmdline);
            _last_cmd_history_pos = 0;
        } else if (_cmdline != _history.back()) {
            _history.push_back(_cmdline);
            _last_cmd_history_pos =
                    (_last_cmd_history_pos + 1) % _history.capacity();
        }
    }
    _new_cmd_saved = true;
    _history_position = _last_cmd_history_pos;
#endif

    const char* argv[CLI_TOKEN_MAX_COUNT];
    int argc = _tokenize(argv, _cmdline);

    switch (argc) {
    case -1:
        _print(CLI_ENDL);
        _print("error: too many tokens");
        break;
    case 0:
        break;
    default:
        _exec(argc, argv);
        break;
    }

    _print_prompt();
}

void server::_esc_move_cursor_left() {
    if (_cursor_pos > 0) {
        --_cursor_pos;
        _print(CLI_ESC "[D");
    }
}

void server::_esc_move_cursor_right() {
    if (_cursor_pos < _cmdline.lenght()) {
        ++_cursor_pos;
        _print(CLI_ESC "[C");
    }
}

void server::_esc_home() {
    if (_cursor_pos > 0) {
        _move_cursor(-int(_cursor_pos));
        _cursor_pos = 0;
    }
}

void server::_esc_end() {
    if (_cursor_pos < _cmdline.lenght()) {
        _move_cursor(int(_cmdline.lenght()) - int(_cursor_pos));
        _cursor_pos = _cmdline.lenght();
    }
}

void server::_esc_back() {
    if (_cursor_pos > 0) {
        memmove(_cmdline.begin() + _cursor_pos - 1,
                _cmdline.begin() + _cursor_pos,
                _cmdline.lenght() - _cursor_pos);
        _cmdline.pop_back();
        --_cursor_pos;

        _print(CLI_ESC "[D"
                       " " CLI_ESC "[D"); // delete symbol
        _save_cursor_pos();
        _print(_cmdline.begin() + _cursor_pos);
        _print(" "); // hide last symbol
        _load_cursor_pos();
    }
}

void server::_esc_del() {
    if (_cursor_pos < _cmdline.lenght()) {
        memmove(_cmdline.begin() + _cursor_pos,
                _cmdline.begin() + _cursor_pos + 1,
                _cmdline.lenght() - _cursor_pos);
        _cmdline.pop_back();

        _save_cursor_pos();
        _print(_cmdline.begin() + _cursor_pos);
        _print(" ");
        _load_cursor_pos();
    }
}

void server::_esc_up() {
#ifdef CLI_USE_HISTORY
    if (!_history.empty()) {
        search_history(HistorySearchDirection::up);
    }
#endif
}

void server::_esc_down() {
#ifdef CLI_USE_HISTORY
    if (!_history.empty()) {
        search_history(HistorySearchDirection::down);
    }
#endif
}

} // namespace cli
