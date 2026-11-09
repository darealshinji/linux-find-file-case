/**
 Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 SPDX-License-Identifier: MIT
 Copyright (c) 2026 Carsten Janssen

 Permission is hereby  granted, free of charge, to any  person obtaining a copy
 of this software and associated  documentation files (the "Software"), to deal
 in the Software  without restriction, including without  limitation the rights
 to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
 copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
 IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
 FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
 AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
 LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
**/

/**
 * Define 'FIND_FILE_CASEXX_IMPLEMENTATION' before including this header if you
 * want the function's implementation to be part of the compile unit:
 *
 * #define FIND_FILE_CASEXX_IMPLEMENTATION
 * #include "find_file_case++.hpp"
 */


#ifndef FIND_FILE_CASEXX_HPP_INCLUDED
#define FIND_FILE_CASEXX_HPP_INCLUDED

#include <filesystem> /* C++17 or newer required */
#include <string>


namespace casecmp
{
    /**
     * Pointer to string compare function.
     * Default implementation is a wrapper around strcasecmp(3).
     */
    typedef bool (compare_funcion_t)(const std::string &s1, const std::string &s2);


    /**
     * Do a case-insensitive search for 'path'.
     *
     * path:
     *    Path or filename to search for.
     *
     * remove_dots:
     *    Remove superfluous "." entries if set 'true'.
     *    Note that superfluous path delimiters are always removed
     *    by std::filesystem::path.
     *
     * compare:
     *    Pointer to a custom string compare function.
     *    You can use this to i.e. provide a UTF-8 aware function.
     *    If not set or set to NULL a default implementation is used.
     *
     * return value:
     *    On success the matched path is returned.
     *    If an error occurs an std::filesystem::filesystem_error
     *    exception is thrown and what() can be used to display
     *    an explanatory string.
     */
    std::filesystem::path find(std::filesystem::path const &path,
                               bool remove_dots = false,
                               compare_funcion_t compare = nullptr);


    inline
    std::filesystem::path find(std::filesystem::path const &&path,
                               bool remove_dots = false,
                               compare_funcion_t compare = nullptr)
    {
        return find(path, remove_dots, compare);
    }
}

#endif /* FIND_FILE_CASEXX_HPP_INCLUDED */



#ifdef FIND_FILE_CASEXX_IMPLEMENTATION

#include <system_error>
#include <errno.h>
#include <strings.h>


namespace casecmp
{
    namespace fs = std::filesystem;


    namespace /* anonymous namespace */
    {
        static inline fs::path remove_dots_from_path(const fs::path &path)
        {
            fs::path out;

            /* copy path and skip "." entries */
            for (auto &e : path) {
                if (e.empty() || e == ".") {
                    continue;
                } else {
                    out /= e;
                }
            }

            /* only dot entries */
            if (out.empty()) {
                out = ".";
            }

            return out;
        }


        static inline std::string find_in_directory(const fs::path &entry, const fs::path *directory, compare_funcion_t compare)
        {
            fs::path want_path{*directory / entry};

            if (fs::exists(fs::status(want_path))) {
                /* no need to iterate directory */
                return entry.string();
            }

            for (auto &dir_ent : fs::directory_iterator{*directory}) {
                auto s1 = entry.string();
                auto s2 = dir_ent.path().filename().string();

                if (compare(s1, s2)) {
                    return s2;
                }
            }

            errno = ENOENT;
            std::error_code ec(errno, std::system_category());

            std::string msg = "no match for entry [" + entry.string() + "]"
                " in path [" + directory->string() + "]";

            throw fs::filesystem_error(msg, want_path, ec);
        }

    } /* anonymous namespace */


    fs::path find(fs::path const &path, bool remove_dots, compare_funcion_t compare)
    {
        /* default compare function using strcasecmp(3) */
        auto compare_default = [] (const std::string &s1, const std::string &s2) -> bool {
            return (s1.length() == s2.length() &&
                    ::strcasecmp(s1.c_str(), s2.c_str()) == 0);
        };

        fs::path out;

        /* treat an empty path as an error */
        if (path.empty()) {
            errno = EINVAL;
            std::error_code ec(errno, std::system_category());
            throw fs::filesystem_error("empty path received as argument", ec);
        }

        /* first check if full path exists (even if it's a dead link) */
        if (fs::exists(fs::symlink_status(path))) {
            return remove_dots ? remove_dots_from_path(path) : path;
        }

        /* initial path is either root or current working directory */
        fs::path root{path.is_absolute() ? "/" : "."};
        fs::path *dir = &root;

        if (!compare) {
            compare = compare_default;
        }

        /* iterate path elements */
        for (auto &e : path) {
            if (e.empty()) {
                continue;
            } else if (e == ".") {
                /* remove "." entries? */
                if (!remove_dots) {
                    out /= e;
                }
            } else if (e == ".." || e == "/") {
                /* nothing to check on ".." and "/" entries */
                out /= e;
            } else {
                /* throws exception on error */
                out /= find_in_directory(e, dir, compare);
                dir = &out;
            }
        }

        return out;
    }
}

#endif /* FIND_FILE_CASEXX_IMPLEMENTATION */

