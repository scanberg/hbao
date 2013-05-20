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

#include "Tokenizer.h"

Tokenizer::Tokenizer(const std::string &str)
{
	tokenizeFast(str);
}

Tokenizer::Tokenizer(const std::string &str, const std::string &chars)
{
	tokenize(str, chars);
}

/** Fast version that only looks for space ' ' **/
void Tokenizer::tokenizeFast(const std::string &str)
{
    tokens.clear();

    size_t begin = 0;

    for(size_t i=0; i<str.length(); ++i)
    {
        if(str[i] == ' ')
        {
            if(begin != i)
            {
                tokens.push_back(str.substr(begin,i-begin));
                begin = i+1;
            }
            else
                ++begin;
        }
    }
    if(begin != str.length())
        tokens.push_back(str.substr(begin));

    counter = 0;
}

/** Generic version that checks for any characters in the string chars. **/
void Tokenizer::tokenize(const std::string &str, const std::string &chars)
{
    tokens.clear();

    size_t begin = 0;
    size_t u;

    for(size_t i=0; i<str.length(); ++i)
    {
        for(u=0; u<chars.length(); ++u)
        {
            if(str[i]==chars[u])
            {
                if(begin != i)
                {
                    tokens.push_back(str.substr(begin,i-begin));
                    begin = i+1;
                }
                else
                    ++begin;
            }
        }
    }
    if(begin != str.length())
        tokens.push_back(str.substr(begin));

    counter = 0;
}

bool Tokenizer::getToken( std::string *str )
{
    if(counter < tokens.size())
    {
        if(str)
            *str = tokens[counter++];
        return true;
    }

    return false;
}

const std::string &Tokenizer::getToken()
{
    if(counter < tokens.size())
        return tokens[counter++];

    return defstr;
}
