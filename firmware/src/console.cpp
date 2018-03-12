/////////////////////////////////////////////////////////////////////////////
/** @file
Serial console

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "console.h"
#include <cctype>
#include <cassert>

//- statics
Console* Console::instance_ = nullptr;

//- constants
const char* PROMPT = "> ";

/////////////////////////////////////////////////////////////////////////////
Console::Console(Stream& stream)
: stream_(stream)
{
    assert(!instance_);
    instance_ = this;

    input_.reserve(MAX_INPUT_LENGTH);
}

/// destructor
Console::~Console() = default;

/////////////////////////////////////////////////////////////////////////////
void Console::begin(const Command* commands, size_t commandsCount) {
    commands_      = commands;
    commandsCount_ = commandsCount;

    stream_.write(PROMPT);
}

/////////////////////////////////////////////////////////////////////////////
void Console::tick() {
    while (stream_.available()) {
        const auto r = stream_.read();
        if (r < 0) break;

        const auto ch = static_cast<char>(r);
        switch (ch) {
        case '\b':
            if (input_.length() > 0) {
                input_.remove(input_.length() - 1);
                stream_.write("\b \b");
            }
            break;
        case '\n':
            break;
        case '\r':
            stream_.write("\r\n");
            evalCommand_(input_);
            input_.remove(0);
            stream_.write(PROMPT);
            break;
        default:
            if (   input_.length() < MAX_INPUT_LENGTH
                && isprint(ch)
                && input_.concat(ch)
            ) {
                stream_.write(ch); // echo
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// evaluate received command
void Console::evalCommand_(String& input) {
    // process arguments
    const char* args[MAX_INPUT_ARGS] = { };
    int argsCount = 0;
    if (!splitInputArgs(input, args, &argsCount)) return;

    // lookup command
    for (size_t i = 0; i < commandsCount_; ++i) {
        if (0 == strcasecmp(commands_[i].name, args[0])) {
            commands_[i].func(args + 1, argsCount - 1); // skip first (command)
            return;
        }
    }

    // no such command
    stream_.write("command not found: ");
    stream_.write(input.c_str());
    stream_.write("\r\n");
}

/////////////////////////////////////////////////////////////////////////////
/// split into into arguments
/// @returns false if input is empty
bool Console::splitInputArgs(String& input, const char** outArgs, int* outArgc) {
    assert(outArgs && outArgc);

    *outArgc = 0;

    // unsigned int idx = 0;
    char* beg = const_cast<char*>(input.c_str()); // evil
    char* end = beg + input.length();
    char* p = beg;
    while (*p) {
        while (isspace(*p)) ++p;
        if (0 == *p) break; // end of string

        outArgs[*outArgc] = p;
        ++(*outArgc);

        char inQuote = 0;
        while (*p) {
            const auto ch = *p;
            if ('\'' == ch || '"' == ch) {
                if (!inQuote || ch == inQuote) {
                    // remove quote character, shuffle everything down
                    // yeah, we could do this more efficiently...
                    memmove(p, p + 1, end - p);
                    *(--end) = 0;
                    inQuote = (!inQuote) ? ch : 0;
                } else {
                    ++p;
                }
            } else if ('\\' == ch) {
                // remove escape character, shuffle everything down
                // yeah, we could do this more efficiently...
                memmove(p, p + 1, end - p);
                *(--end) = 0;
                if (*p) ++p; // escaped next
            } else {
                if (!inQuote && isspace(ch)) break;
                ++p;
            }
        }
        if (*p) *p++ = 0; // null terminate
    }

    return (*outArgc > 0);
}

/////////////////////////////////////////////////////////////////////////////
/// list available commands
void Console::cmdHelp(const char* [], int) {
    for (size_t i = 0; i < instance_->commandsCount_; ++i) {
        instance_->stream_.write(instance_->commands_[i].name);
        instance_->stream_.write("\r\n");
    }
}
