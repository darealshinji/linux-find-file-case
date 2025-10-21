Case-insensitive filename lookup for POSIX.

``` C
char *find_file_case(const char *path, char *buffer, size_t buflen, int flags, struct match_fptrs *fptrs);
```

The function `find_file_case()` will do a path lookup using `readdir()` and
compare pathnames with `strcasecmp()`.

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

    puts(p);
    free(p);

    return 0;
}
```

Output: `/usr/bin/bash`

