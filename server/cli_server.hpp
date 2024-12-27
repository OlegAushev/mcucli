#pragma once

#include <cstdio>
#include <cstring>
#include <emblib/circular_buffer.hpp>
#include <emblib/queue.hpp>
#include <emblib/static_string.hpp>
#include <mcudrv/generic/gpio.hpp>
#include <mcudrv/generic/uart.hpp>

#include "../cli_config.hpp"

namespace cli {

class server {
    friend void print(const char* str);
    friend void print_blocking(const char* str);
private:
    static inline bool _enabled{true};
    static inline mcu::uart::tty* _tty{nullptr};
    static inline mcu::gpio::output_pin* _pin_rts{nullptr};
    static inline mcu::gpio::input_pin* _pin_cts{nullptr};

    static inline char _prompt[CLI_PROMPT_MAX_LENGTH]{};
    static inline emb::static_string<CLI_CMDLINE_MAX_LENGTH> _cmdline;
    static inline emb::static_string<CLI_ESCSEQ_MAX_LENGTH> _escseq;

    static inline size_t _cursor_pos = 0;

    static inline emb::queue<char, CLI_OUTBUT_BUFFER_LENGTH> _output_buf;

#ifdef CLI_USE_HISTORY
    static inline emb::circular_buffer<
            emb::static_string<CLI_CMDLINE_MAX_LENGTH>,
            CLI_HISTORY_LENGTH>
            _history;
    static inline size_t _last_cmd_history_pos{0};
    static inline size_t _history_position{0};
    static inline bool _new_cmd_saved{false};
#endif
private:
    server() = default;
public:
    server(const server& other) = delete;
    server& operator=(const server& other) = delete;
    static void init(const char* device_name,
                     mcu::uart::tty* tty,
                     mcu::gpio::output_pin* pin_rts,
                     mcu::gpio::input_pin* pin_cts,
                     const char* welcome_message = nullptr);
    static void run();
    static void register_exec_callback(int (*exec)(int argc,
                                                   const char** argv)) {
        _exec = exec;
    }

    static void enable();
    static void disable();
private:
    static void _print(char ch);
    static void _print(const char* str);
    static void _print_blocking(const char* str);

    static void _process_char(char ch);
    static void _save_cursor_pos() { _print(CLI_ESC "[s"); }
    static void _load_cursor_pos() { _print(CLI_ESC "[u"); }
    static void _move_cursor(int offset);
    static void _print_welcome(const char* welcome_message);
    static void _print_prompt();
    static int _tokenize(const char** argv,
                         emb::static_string<CLI_CMDLINE_MAX_LENGTH>& cmdline);

    static int (*_exec)(int argc, const char** argv);
    static int _exec_null(int argc, const char** argv) {
        _print(CLI_ENDL "error: exec-callback not registered");
        _print(CLI_ENDL "tokens:");
        for (auto i = 0; i < argc; ++i) {
            _print(CLI_ENDL);
            _print(argv[i]);
        }
        return -1;
    }

public:
    struct EscSeq {
        const char* str;
        size_t len;
        void (*handler)();
    };
    static const EscSeq escseq_list[];
private:
    static void _esc_return();
    static void _esc_move_cursor_left();
    static void _esc_move_cursor_right();
    static void _esc_home();
    static void _esc_end();
    static void _esc_back();
    static void _esc_del();
    static void _esc_up();
    static void _esc_down();

private:
#ifdef CLI_USE_HISTORY
    enum class HistorySearchDirection {
        up,
        down,
    };
    static void search_history(HistorySearchDirection dir);
#endif
};

inline void print(const char* str) { server::_print(str); }

inline void nextline() { print(CLI_ENDL); }

inline void print_blocking(const char* str) { server::_print_blocking(str); }

inline void nextline_blocking() { print_blocking(CLI_ENDL); }

} // namespace cli
