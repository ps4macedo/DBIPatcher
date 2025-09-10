/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"

#define ESC             "\x1B"
#define LOG_SUFFIX      ESC "[0m"
#define LOG_BUFFER      1024

#define LF_FILE         0x01
#define LF_CLI          0x02

static FILE * log_file = NULL;

static const char * log_get_prefix(LogLevel lvl) {
    switch(lvl) {
        case LOG_TRACE:     return ESC "[1;35m";
        case LOG_DEBUG:     return ESC "[1;34m";
        case LOG_INFO:      return ESC "[0m";
        case LOG_NOTICE:    return ESC "[0;32m";
        case LOG_WARNING:   return ESC "[1;33m";
        case LOG_ERROR:     
        default:            return ESC "[1;31m";
    }
}

static const char * log_get_level(LogLevel lvl) {
    switch(lvl) {
        case LOG_TRACE:     return "TRACE";
        case LOG_DEBUG:     return "DEBUG";
        case LOG_INFO:      return "INFO";
        case LOG_NOTICE:    return "NOTICE";
        case LOG_WARNING:   return "WARNING";
        case LOG_ERROR:     
        default:            return "ERROR";
    }
}

static const char * log_get_time(void) {
    static char timebuff[10];
    
    time_t t;
    struct tm tm;
    
    t = time(NULL);
    tm = *localtime(&t);

    snprintf(timebuff, sizeof(timebuff), "%02d:%02d:%02d ", tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    return timebuff;
}

static void log_write(uint8_t flags, const char * str) {    
    if(flags & LF_CLI) {
        printf("%s", str);
    }
    
    if(flags & LF_FILE) {
        if(log_file) {
            fwrite(str, 1 , strlen(str), log_file);
        }
    }
}

void log_init(const char * file) {
    log_file = fopen(file, "a+");
}

void log_close(void) {
    if(log_file) {
        fflush(log_file);
        fclose(log_file);
        log_file = NULL;
    }
}

void lf_s(LogLevel lvl, const char * fmt, ...) {
    char tmp[LOG_BUFFER];
    va_list args;
    va_start(args, fmt);
    
    log_write(LF_CLI, log_get_prefix(lvl));
    log_write(LF_CLI | LF_FILE, log_get_time());
    
    snprintf(tmp, sizeof(tmp), " [%c] ", log_get_level(lvl)[0]);
    
    log_write(LF_CLI | LF_FILE, tmp);
    
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    
    log_write(LF_CLI | LF_FILE, tmp);
    
    log_write(LF_CLI, LOG_SUFFIX);
    log_write(LF_CLI | LF_FILE, CRLF);
    
    va_end(args);
}