/////////////////////////////////////////////////////////////////////////////
/** @file
Serial console

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__CONSOLE
#define INCLUDED__CONSOLE

//- includes
#include <Stream.h>

/////////////////////////////////////////////////////////////////////////////
/// Serial console
class Console {
public:
    enum {
        MAX_INPUT_LENGTH = 64,  ///< maximum input length
        MAX_INPUT_ARGS = 5,     ///< maximum number of arguments
    };

    /// console command
    struct Command {
        const char*     name;
        void (*func)(const char* [], int);
    };

    explicit Console(Stream& stream);
    ~Console();

    void begin(const Command* commands, size_t commandsCount);
    void tick();

    static void cmdHelp(const char* argv[], int argc);

    static bool splitInputArgs(String& input, const char** outArgs, int* outArgc);

private:
    void evalCommand_(String& input);

    static Console*     instance_;          ///< singleton instance

    Stream&             stream_;            ///< attached stream
    String              input_;             ///< current input

    const Command*      commands_{nullptr}; ///< available commands
    size_t              commandsCount_{0};  ///< number of available commands
};

#endif // INCLUDED__CONSOLE
