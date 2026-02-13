#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#define FIND_FILE_CASE_IMPLEMENTATION
#include "find_file_case.h"


static int my_utf8_casecmp(const char *str1, const char *str2)
{
    if (g_str_is_ascii(str1) && g_str_is_ascii(str2)) {
        /* both strings have only pure ASCII characters */
        return g_ascii_strcasecmp(str1, str2);
    }

    /* convert UTF-8 strings to a form that is independent
     * of case and compare with g_utf8_collate() */
    gchar *fold1 = g_utf8_casefold(str1, -1);
    gchar *fold2 = g_utf8_casefold(str2, -1);

    int rv = g_utf8_collate(fold1, fold2);

    g_free(fold1);
    g_free(fold2);

    return rv;
}


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

    printf("input search string: %s\n", argv[1]);

    /* test 1: use buffer */
    errno = 0;
    flags = FIND_FILE_CASE_NONNULL | FIND_FILE_CASE_CLEANUP;
    ptr = find_file_case(argv[1], buf, sizeof(buf), flags, NULL);
    errsav = errno;

    printf("\ntest 1 (default, buffer): %s\n", ptr);

    if (errsav == 0) {
        puts("Match");
    } else {
        errno = errsav;
        perror("Error");
    }

    /* test 2: use custom strcasecmp function, allocate buffer */
    memset(&fptrs, 0, sizeof(struct match_fptrs));
    fptrs.strcasecmp = my_utf8_casecmp;

    errno = 0;
    ptr = find_file_case(argv[1], NULL, 0, 0, &fptrs);
    errsav = errno;

    printf("\ntest 2 (UTF-8, allocate): %s\n", ptr ? ptr : "(null)");

    if (ptr) {
        puts("Match");
    } else {
        errno = errsav;
        perror("Error");
        return 1;
    }

    free(ptr);

    return 0;
}
