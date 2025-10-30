#include <iostream>
#include <stdlib.h>
#define FIND_FILE_CASE_IMPLEMENTATION
#include "find_file_case.h"


int main()
{
    const char *input = "/Usr/Bin/BASH";
    char *p = find_file_case(input, NULL, 0, 0, NULL);

    if (!p) {
        perror("find_file_case()");
        return 1;
    }

    std::cout << "given input: " << input << std::endl;
    std::cout << "result: " << p << std::endl;

    free(p);

    return 0;
}
