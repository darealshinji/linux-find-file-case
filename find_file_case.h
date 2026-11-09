/**
 Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 SPDX-License-Identifier: MIT
 Copyright (c) 2025 Carsten Janssen

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
 * Define 'FIND_FILE_CASE_IMPLEMENTATION' before including this header if you
 * want the function's implementation to be part of the compile unit:
 *
 * #define FIND_FILE_CASE_IMPLEMENTATION
 * #include "find_file_case.h"
 */


#ifndef FIND_FILE_CASE_H_INCLUDED
#define FIND_FILE_CASE_H_INCLUDED

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>


/* Flags */
#define FIND_FILE_CASE_CLEANUP 1
#define FIND_FILE_CASE_NONNULL 2


/**
 * Use this struct to set custom functions.
 */
struct match_fptrs {
    int (*strcasecmp)(const char *, const char *);  /* strcasecmp(3) */
    int (*stat)(const char *, struct stat *);       /* stat(2) */
    int (*lstat)(const char *, struct stat *);      /* lstat(2) */
    DIR *(*opendir)(const char *);                  /* opendir(3) */
    int (*closedir)(DIR *);                         /* closedir(3) */
    struct dirent *(*readdir)(DIR *);               /* readdir(3) */
};


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Do a case-insensitive search for 'path'.
 *
 * path:
 *    Path or filename to search for.
 *
 * buffer:
 *    Pointer to buffer to store the output. The size of this buffer should be
 *    at least strlen(path) + 1 byte for the terminating NUL.
 *    If 'buffer' is NULL the output will be allocated and the result must
 *    later be freed with free(3).
 *
 * buflen:
 *    Size of buffer in bytes. If buffer is NULL this value is ignored.
 *
 * flags:
 *    Zero or more of the following values may be ORed in flags:
 *
 *    FIND_FILE_CASE_CLEANUP
 *     Remove superfluous "." entries and path delimiters from output.
 *
 *    FIND_FILE_CASE_NONNULL
 *     Never return NULL. On failure a copy of 'path' is returned instead.
 *     If 'path' is set to NULL a copy of an empty string is returned.
 *
 * fptrs:
 *    Pointer to struct holding pointers to alternative versions of the following
 *    used functions: strcasecmp, stat, lstat, opendir, closedir, readdir.
 *    The pointer or any of the struct's entries can be NULL in which case the
 *    default function(s) will be used instead.
 *
 * return value:
 *    On success it returns a pointer to 'buffer' or an allocated string
 *    holding the matched path. The output is always NUL terminated.
 *    On failure NULL is returned and errno is set to indicate the error.
 */
extern char *find_file_case(const char *path, char *buffer, size_t buflen, int flags, struct match_fptrs *fptrs);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FIND_FILE_CASE_H_INCLUDED */



#ifdef FIND_FILE_CASE_IMPLEMENTATION

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


static inline void *_find_file_case_malloc(size_t len)
{
    void *p = malloc(len);

    if (!p) {
        perror("malloc()");
        abort();
    }

    return p;
}


static inline char *_find_file_case_strdup(const char *s)
{
    char *p = strdup(s);

    if (!p) {
        perror("strdup()");
        abort();
    }

    return p;
}


static int _find_file_case_path_exists(const char *p1, const char *p2, struct match_fptrs *f)
{
    struct stat st;
    size_t len;
    char *buf;
    int rv;

    len = strlen(p1) + strlen(p2) + 2;
    buf = (char *)_find_file_case_malloc(len);
    snprintf(buf, len, "%s/%s", p1, p2);

    rv = f->lstat(buf, &st);
    free(buf);

    return rv;
}


static char *_find_file_case_readdir(const char *dir, char *token, struct match_fptrs *f)
{
    static struct dirent *d;
    DIR *dirp;
    char *match = NULL;
    int errsav;

    if ((dirp = f->opendir(dir)) == NULL) {
        return NULL;
    }

    errno = 0;

    while ((d = f->readdir(dirp)) != NULL) {
        if (f->strcasecmp(d->d_name, token) == 0) {
            match = d->d_name;
            break;
        }
    }

    if (!d) {
        errsav = errno;
        f->closedir(dirp);
        errno = errsav;
        return NULL;
    }

    f->closedir(dirp);

    return match;
}


static char *_find_file_case_iterate_path(char *pbuf, size_t buflen, size_t path_len, int flags, struct match_fptrs *f)
{
    char *p, *prev, *last, *token;
    const char *dir;

    /* initial path is either root or current working directory */
    dir = (*pbuf == '/') ? "/" : ".";

    /* set pointers initially to begin of path */
    prev = last = p = pbuf;

    /* iterate through path */
    for ( ; (token = strtok(p, "/")) != NULL; dir=pbuf, last=token, p=NULL) {
        char *match = NULL;

        /* iterate the directory if path doesn't exist */
        if (_find_file_case_path_exists(dir, token, f) == -1) {
            match = _find_file_case_readdir(dir, token, f);
        }

        /* restore delimiters */
        for ( ; prev != token; prev++) {
            if (*prev == 0) {
                *prev = '/';
            }
        }

        /* append match */
        if (match) {
            *token = 0;
            strlcat(pbuf, match, buflen);
        }
    }

    if (!(flags & FIND_FILE_CASE_CLEANUP)) {
        /* append trailing delimiters */
        char *end = pbuf + path_len;
        last += strlen(last);

        if (end > last) {
            memset(last, '/', end - last);
            *(end+1) = 0;
        }
    }

    return pbuf;
}


static void _find_file_case_path_cleanup(char *pbuf, size_t buflen)
{
    char *temp, *p, *token;

    temp = (char *)_find_file_case_malloc(buflen);
    temp[0] = 0;

    for (p = pbuf; ; p = NULL) {
        if ((token = strtok(p, "/")) == NULL) {
            break;
        }

        /* skip "." entries */
        if (strcmp(token, ".") == 0) {
            continue;
        }

        /* path delimiter or root directory */
        if (*temp != 0 || (*temp == 0 && *pbuf == '/')) {
            strlcat(temp, "/", buflen);
        }

        strlcat(temp, token, buflen);
    }

    if (*temp == 0) {
        if (*pbuf == '/') {
            strlcpy(pbuf, "/", buflen);
        } else {
            strlcpy(pbuf, ".", buflen);
        }
    } else {
        strlcpy(pbuf, temp, buflen);
    }

    free(temp);
}


static char *_find_file_case_error(const char *path, char *buffer, char *copy, size_t buflen, int flags)
{
    free(copy);

    if (errno == 0) {
        errno = ENOENT;
    }

    /* return a copy of "path" instead of NULL */
    if (flags & FIND_FILE_CASE_NONNULL) {
        if (!path) {
            path = "";
        }

        if (buffer) {
            strlcpy(buffer, path, buflen);
            return buffer;
        }

        return _find_file_case_strdup(path);
    }

    return NULL;
}


char *find_file_case(const char *path, char *buffer, size_t buflen, int flags, struct match_fptrs *fptrs)
{
    struct stat st;
    struct match_fptrs f;
    size_t path_len;
    char *pbuf = NULL;
    char *copy = NULL;

    if (!path || *path == 0) {
        errno = EINVAL;
        return _find_file_case_error(path, buffer, NULL, buflen, flags);
    }

    path_len = strlen(path);

    if (buffer && (path_len == 0 || buflen < (path_len+1))) {
        errno = ENOMEM;
        return _find_file_case_error(path, buffer, NULL, buflen, flags);
    }

    /* set function pointers */
#define XSET(FUNC) \
    do { \
        f.FUNC = (fptrs && fptrs->FUNC) ? fptrs->FUNC : FUNC; \
    } while (0)

    XSET(strcasecmp);
    XSET(stat);
    XSET(lstat);
    XSET(opendir);
    XSET(closedir);
    XSET(readdir);

#undef XSET

    /* copy path */
    if (buffer) {
        pbuf = (char *)memcpy(buffer, path, path_len + 1);
    } else {
        buflen = path_len + 1;
        pbuf = copy = _find_file_case_strdup(path);
    }

    /* if given path does not exist do case insensitive search */
    if (f.lstat(pbuf, &st) == -1 &&
        (pbuf = _find_file_case_iterate_path(pbuf, buflen, path_len, flags, &f)) == NULL)
    {
        return _find_file_case_error(path, buffer, copy, buflen, flags);
    }

    /* treat non-directory with trailing delimiters as an error */
    if (pbuf[path_len-1] == '/') {
        if (f.stat(pbuf, &st) == -1) {
            return _find_file_case_error(path, buffer, copy, buflen, flags);
        }

        if ((st.st_mode & S_IFMT) != S_IFDIR) {
            errno = ENOTDIR;
            return _find_file_case_error(path, buffer, copy, buflen, flags);
        }
    }

    /* remove superfluous "." entries and path delimiters */
    if ((flags & FIND_FILE_CASE_CLEANUP) && strchr(pbuf, '/')) {
        _find_file_case_path_cleanup(pbuf, buflen);
    }

    return pbuf;
}

#endif /* FIND_FILE_CASE_IMPLEMENTATION */
