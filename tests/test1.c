#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utf8.h"

#define FIND_FILE_CASE_IMPLEMENTATION
#include "find_file_case.h"

/**
 * example: ./a.out ~/.wine/DOSdevices/C:/WINDOWS/System32/NtDll.dll
 */
int main(int argc, char **argv)
{
    char buf[4096];
    char *ptr;
    int errsav, flags;
    struct match_fptrs fptrs;

    if (argc < 2) {
        printf("usage: %s <path>\n", argv[0]);
        return 1;
    }

    printf("original: %s\n", argv[1]);

    /* test 1 */
    errno = 0;
    flags = FIND_FILE_CASE_NONNULL | FIND_FILE_CASE_CLEANUP;
    ptr = find_file_case(argv[1], buf, sizeof(buf), flags, NULL);
    errsav = errno;

    printf("test 1:   %s\n", ptr);

    if (errsav != 0) {
        errno = errsav;
        perror("Error");
    }

    /* test 2 */
    memset(&fptrs, 0, sizeof(struct match_fptrs));
    fptrs.strcasecmp = utf8casecmp;

    errno = 0;
    ptr = find_file_case(argv[1], NULL, 0, 0, &fptrs);
    errsav = errno;

    printf("test 2:   %s\n", ptr ? ptr : "(null)");

    if (!ptr) {
        errno = errsav;
        perror("Error");
        return 1;
    }

    free(ptr);

    return 0;
}
