/////////////////////////////////////////////////////////////////////////////
/** @file
Test console

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "doctest_ext.h"
#include "console.h"
#include <string>

/////////////////////////////////////////////////////////////////////////////
TEST_SUITE("Console") {
    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("splitInputArgs") {
        const char* args[Console::MAX_INPUT_ARGS];
        int argsCount = -1;

        // empty case
        {
            String empty;
            CHECK(Console::splitInputArgs(empty, args, &argsCount) == false);
            CHECK(argsCount == 0);

            empty = "    ";
            CHECK(Console::splitInputArgs(empty, args, &argsCount) == false);
            CHECK(argsCount == 0);
        }

        // single command
        {
            String test = "test";
            CHECK(Console::splitInputArgs(test, args, &argsCount));
            CHECK(argsCount == 1);
            CHECK(args[0] == std::string("test"));

            test = "  test   ";
            CHECK(Console::splitInputArgs(test, args, &argsCount));
            CHECK(argsCount == 1);
            CHECK(args[0] == std::string("test"));
        }

        // command with arguments
        {
            String test = "  test 1  2  3  ";
            CHECK(Console::splitInputArgs(test, args, &argsCount));
            CHECK(argsCount == 4);
            CHECK(args[0] == std::string("test"));
            CHECK(args[1] == std::string("1"));
            CHECK(args[2] == std::string("2"));
            CHECK(args[3] == std::string("3"));
        }

        // quote handling
        {
            String test = "  test ' 1\" 2'  \"3 ' 4 \"  5 ";
            CHECK(Console::splitInputArgs(test, args, &argsCount));
            CHECK(argsCount == 4);
            CHECK(args[0] == std::string("test"));
            CHECK(args[1] == std::string(" 1\" 2"));
            CHECK(args[2] == std::string("3 ' 4 "));
            CHECK(args[3] == std::string("5"));
        }
        // cojoined quotes
        {
            String test = "  test ' 1 2'' 3 4'";
            CHECK(Console::splitInputArgs(test, args, &argsCount));
            CHECK(argsCount == 2);
            CHECK(args[0] == std::string("test"));
            CHECK(args[1] == std::string(" 1 2 3 4"));
        }
        // unclosed
        {
            String test = "  test ' 1 2 3";
            CHECK(Console::splitInputArgs(test, args, &argsCount));
            CHECK(argsCount == 2);
            CHECK(args[0] == std::string("test"));
            CHECK(args[1] == std::string(" 1 2 3"));
        }

        // escaping
        {
            String test = "  test \\\'1  2\\ 3\\\\ 4\\";
            CHECK(Console::splitInputArgs(test, args, &argsCount));
            CHECK(argsCount == 4);
            CHECK(args[0] == std::string("test"));
            CHECK(args[1] == std::string("'1"));
            CHECK(args[2] == std::string("2 3\\"));
            CHECK(args[3] == std::string("4"));
        }
    }
}
