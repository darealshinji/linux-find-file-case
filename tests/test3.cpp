#include <filesystem>
#include <iostream>
#include <string>
#include <ctype.h>
#include <strings.h>

#define FIND_FILE_CASEXX_IMPLEMENTATION
#include "find_file_case++.hpp"


/* replace string */
static void repl(const std::string &from, const std::string &to, std::string &str)
{
    size_t pos = 0;

    while ((pos = str.find(from)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
}

static std::string turkic_casefold(const std::string &in)
{
    std::string out{in};

    /* replace special characters */
    repl("I",      "\u0131", out); /* dotless i */
    repl("i",      "\u0130", out); /* i with dot above */
    repl("\u00C7", "\u00E7", out); /* c with cedilla */
    repl("\u015E", "\u015F", out); /* s with cedilla */
    repl("\u00D6", "\u00F6", out); /* o with diaeresis */
    repl("\u00DC", "\u00FC", out); /* u with diaeresis */
    repl("\u011E", "\u011F", out); /* g with breve */

    for (auto &ch : out) {
        switch (ch)
        {
        /* NOT uppercase i! */
        case 'A'...'H':
        case 'J'...'Z':
            ch = tolower(ch);
            continue;
        }
    }

    return out;
}

/* case-insensitive comparison for Turkic language strings */
static bool turkic_casecmp(const std::string &s1, const std::string &s2)
{
    return (turkic_casefold(s1) == turkic_casefold(s2));
}


int main()
{
    //std::filesystem::path p1{"/USR/BiN/FILE"}; /* this will fail */
    std::filesystem::path p1{"/USR/BiN/FiLE"};
    std::filesystem::path p2;

    try {
        p2 = casecmp::find(p1, true, turkic_casecmp);
    }
    catch (std::filesystem::filesystem_error const &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << p1 << "  ==>  " << p2 << std::endl;

    return 0;
}

