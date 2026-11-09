# Case-insensitive filename lookup for POSIX.

``` C
char *find_file_case(const char *path, char *buffer, size_t buflen, int flags, struct match_fptrs *fptrs);
```

The function `find_file_case()` will do a path lookup and compare pathnames with `strcasecmp()`.

The main intention is to use it for porting case-unaware software from Windows to *nix.

You can set custom functions in the `fptrs` argument, i.e. if you want
to use a UTF-8 aware version of `strcasecmp()` or if you want to replace `lstat()` with `stat()`.


Simple example:
``` C
#include <stdio.h>
#include <stdlib.h>
#define FIND_FILE_CASE_IMPLEMENTATION
#include "find_file_case.h"

int main()
{
    char *p = find_file_case("/Usr/Bin/BASH", NULL, 0, 0, NULL);

    if (!p) {
        perror("find_file_case");
        return 1;
    }

    puts(p); /* "/usr/bin/bash" */
    free(p);

    return 0;
}
```


## C++

A C++17 function making use of std::filesystem is available too.

``` C++
typedef bool (casecmp::compare_funcion_t)(const std::string &s1, const std::string &s2);

std::filesystem::path casecmp::find(std::filesystem::path const &path,
                                    bool remove_dots = false,
                                    casecmp::compare_funcion_t compare = nullptr);

std::filesystem::path casecmp::find(std::filesystem::path const &&path,
                                    bool remove_dots = false,
                                    casecmp::compare_funcion_t compare = nullptr);
```

Simple example:
``` C++
#include <filesystem>
#include <iostream>
#define FIND_FILE_CASEXX_IMPLEMENTATION
#include "find_file_case++.hpp"

int main()
{
    std::filesystem::path path;

    try {
        path = casecmp::find("/USR/./BIN/BASH", true);
    }
    catch (std::filesystem::filesystem_error const &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << path << std::endl; /* "/usr/bin/bash" */

    return 0;
}
```
