/**
* Tokenizer.h
*
* A lightweight tokenizer for strings.
* Internally it uses two different versions,
* one to recognize blank space ' ' and one generic
* to recognize characters placed in a string, ex. " ,."
*
* Author: Robin Skånberg
**/

/**
Copyright (C) 2012-2014 Robin Skånberg

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <vector>
#include <iostream>
#include <string>

class Tokenizer
{
private:
    std::vector<std::string> tokens;

    /** Empty string to return as a failsafe **/
    const std::string defstr;
    unsigned int counter;

public:
    Tokenizer() {}
    Tokenizer(const std::string &str);

    Tokenizer(const std::string &str, const std::string &chars);

    /** Fast version that only looks for space ' ' **/
    void tokenizeFast(const std::string &str);

    /** Generic version that checks for any characters in the string chars. **/
    void tokenize(const std::string &str, const std::string &chars = " ");

    bool getToken( std::string *str );
    const std::string &getToken();

    bool hasMore() { return counter != tokens.size(); }

    void reset() { counter=0; }
    bool end() { return counter==tokens.size(); }
    unsigned int size() { return tokens.size(); }
};

#endif
