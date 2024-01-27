#include "cli_server.h"


namespace cli {


const char* prompt_begin = CLI_PROMPT_BEGIN;
const char* prompt_end = CLI_PROMPT_END;


int (*server::_exec)(int argc, const char** argv) = server::_exec_null;


const server::EscSeq server::escseq_list[] = {
{.str = "\x0D",         .len = 1,   .handler = server::_esc_return},
{.str = "\x0A",         .len = 1,   .handler = server::_esc_return},
{.str = CLI_ESC"[D",	.len = 3,   .handler = server::_esc_move_cursor_left},
{.str = CLI_ESC"[C",	.len = 3,   .handler = server::_esc_move_cursor_right},
{.str = CLI_ESC"[H",	.len = 3,   .handler = server::_esc_home},
{.str = CLI_ESC"[F",	.len = 3,   .handler = server::_esc_end},
{.str = "\x08",         .len = 1,   .handler = server::_esc_back},
{.str = "\x7F",         .len = 1,   .handler = server::_esc_back},
{.str = CLI_ESC"[3~",   .len = 4,   .handler = server::_esc_del},
{.str = CLI_ESC"[A",    .len = 3,   .handler = server::_esc_up},
{.str = CLI_ESC"[B",    .len = 3,   .handler = server::_esc_down},
};


const size_t escseq_list_size = sizeof(server::escseq_list) / sizeof(server::escseq_list[0]);


void server::init(const char* device_name, emb::tty* tty,
                  emb::gpio::output_pin* pin_rts, emb::gpio::input_pin* pin_cts) {
    _tty = tty;
    _pin_rts = pin_rts;	// output
    _pin_cts = pin_cts;	// input

    memset(_prompt, 0, CLI_PROMPT_MAX_LENGTH);
    strcat(_prompt, prompt_begin);
    strncat(_prompt, device_name, CLI_DEVICE_NAME_MAX_LENGTH);
    strcat(_prompt, prompt_end);

    _print_welcome();
    _print_prompt();
}


void server::run() {
    if (!_tty || !_enabled) return;

    if (!_output_buf.empty()) {
        if (_tty->putchar(_output_buf.front()) != EOF) {
            _output_buf.pop();
        }
    } else {
        int ch = _tty->getchar();
        if (ch != EOF) {
            _process_char(static_cast<char>(ch));
        }
    }
}


void server::enable() {
    _enabled = true;
}


void server::disable() {
    _enabled = false;
}


void server::_print(char ch) {
    if (!_output_buf.full()) {
        _output_buf.push(ch);
    }
}


void server::_print(const char* str) {
    while ((*str != '\0') && !_output_buf.full()) {
        _output_buf.push(*str++);
    }
}


void server::_print_blocking(const char* str)
{
    if (!_tty) return;
    while (*str != '\0') {
        while (_tty->putchar(*str) != *str) {}
        ++str;
    }
}


void server::_process_char(char ch)
{
    if (_cmdline.full()) {
        return;
    }

    if (_escseq.empty()) {
        // Check escape signature
        if (ch <= 0x1F || ch == 0x7F) {
            _escseq.push_back(ch);
        }
        // Print symbol if escape sequence signature is not found
        if (_escseq.empty()) {
            if (_cursor_pos < _cmdline.lenght()) {
                _cmdline.insert(_cursor_pos, ch);
                _save_cursor_pos();
                _print(_cmdline.begin() + _cursor_pos);
                _load_cursor_pos();
            } else {
                _cmdline.push_back(ch);
            }
            _print(ch);
            ++_cursor_pos;
        }
    } else {
        _escseq.push_back(ch);
    }

    // Process escape sequence
    if (!_escseq.empty()) {
        int possible_escseq_count = 0;
        size_t escseq_idx = 0;
        for (size_t i = 0; i < escseq_list_size; ++i) {
            if ((_escseq.lenght() <= escseq_list[i].len)
                    && (strncmp(_escseq.data(), escseq_list[i].str, _escseq.lenght()) == 0)) {
                ++possible_escseq_count;
                escseq_idx = i;
            }
        }

        switch (possible_escseq_count) {
        case 0: // no sequence - display all symbols
            /*for (size_t i = 0; (i < m_escseq.lenght()) && (!m_cmdline.full()); ++i)
            {
                if (m_escseq[i] <= 0x1F || m_escseq[i] == 0x7F)
                {
                    m_escseq[i] = '?';
                }
                m_cmdline.insert(m_cursorPos + i, m_escseq[i]);
            }
            _print(m_cmdline.begin() + m_cursorPos);
            m_cursorPos += m_escseq.lenght();*/
            _escseq.clear();
            break;

        case 1: // one possible sequence found - check size and call handler
            if (_escseq.lenght() == escseq_list[escseq_idx].len) {
                _escseq.clear();
                escseq_list[escseq_idx].handler();
            }
            break;

        default: // few possible sequences found
            break;
        }
    }
}


void server::_move_cursor(int offset) {
    char str[16] = {0};
    if (offset > 0) {
        snprintf(str, 16, CLI_ESC"[%dC", offset);
    } else if (offset < 0) {
        snprintf(str, 16, CLI_ESC"[%dD", -(offset));
    }
    _print(str);
}


void server::_print_welcome() {
    cli::nextline_blocking();
    cli::nextline_blocking();
    cli::nextline_blocking();
    cli::print_blocking(CLI_WELCOME_STRING);
    cli::nextline_blocking();
}


void server::_print_prompt() {
    _print(CLI_ENDL);
    _print(_prompt);
    _cmdline.clear();
    _cursor_pos = 0;
}


int server::_tokenize(const char** argv, emb::static_string<CLI_CMDLINE_MAX_LENGTH>& cmdline) {
    int argc = 0;
    size_t idx = 0;

    if (cmdline.empty()) {
        return 0;
    }

    // replace all ' ' with '\0'
    for (size_t i = 0; i < cmdline.lenght(); ++i) {
        if (cmdline[i] == ' ') {
            cmdline[i] = '\0';
        }
    }

    while (true) {
        // go to the first not-whitespace (now - '\0')
        while (cmdline[idx] == '\0') {
            if (++idx >= cmdline.lenght())
                return argc;
        }

        if (argc >= CLI_TOKEN_MAX_COUNT) {
            return -1;
        }

        argv[argc++] = cmdline.begin() + idx;

        // go to the first whitespace (now - '\0')
        while ((cmdline[idx] != '\0') && (idx < cmdline.lenght())) {
            if (++idx >= cmdline.lenght())
                return argc;
        }
    }
}


#ifdef CLI_USE_HISTORY
void server::search_history(HistorySearchDirection dir) {
    static size_t pos;

    switch (dir) {
    case HistorySearchDirection::up:
        if (_new_cmd_saved) {
            pos = _history_position;
        } else {
            _history_position = (_history_position + (_history.size() - 1)) % _history.size();
            pos = _history_position;
        }
        break;
    case HistorySearchDirection::down:
        _history_position = (_history_position + 1) % _history.size();
        pos = _history_position;
        break;
    }

    _new_cmd_saved = false;

    // move cursor to line beginning
    if (_cursor_pos > 0) {
        _move_cursor(-static_cast<int>(_cursor_pos));
        _cursor_pos = 0;
    }

    int remainder = int(_cmdline.size()) - int(_history.data()[pos].size());
    _cmdline = _history.data()[pos];
    _print(_cmdline.data());
    _cursor_pos = _cmdline.size();

    // clear remaining symbols
    _save_cursor_pos();
    for (int i = 0; i < remainder; ++i) {
        _print(" ");
    }
    _load_cursor_pos();
}
#endif


} // namespace cli
