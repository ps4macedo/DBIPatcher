/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils.h"

static int _mkpath(char* file_path, mode_t mode) {
    for (char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = '\0';
        if (mkdir(file_path, mode) == -1) {
            if (errno != EEXIST) {
                *p = '/';
                return -1;
            }
        }
        *p = '/';
    }
    return 0;
}

int mkpath(mode_t mode, const char* fmt, ...) {
    char * path = malloc(FILENAME_MAX);
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(path, FILENAME_MAX, fmt, args);
    va_end(args);
    
    int ret = _mkpath(path, mode);
    free(path);
    return ret;
}