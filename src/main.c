/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 *
 * Created on 9. září 2025, 12:21
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "log.h"
#include "extract.h"
#include "utils.h"
#include "convert.h"
#include "patch.h"

#define APP         "dbipatcher"
#define APP_LOG     APP ".log"
#define ARRLEN(arr) (sizeof(arr)/sizeof(*arr))

typedef struct {
    char * binary;
    char * patch;
    char * output;
    char * extract;
    char * convert;
    char * keys;
    int slot;
    int help;
} Args;

static Args args;

void print_help(const char *program_name) {
    printf("Usage: %s [OPTIONS]" CRLF, program_name);
    printf(CRLF "Options:" CRLF);
    printf("  -b, --binary FILE      Input binary file to patch" CRLF);
    printf("  -p, --patch FILE       Patch file to apply" CRLF);
    printf("  -o, --output FILE      Output file or directory" CRLF);
    printf("  -k, --keys FILE        Output file or directory" CRLF);
    printf("  -s, --slot NUMBER      Slot index for patch application" CRLF);
    printf("  -e, --extract FILE     Extract payloads from a DBI binary" CRLF);
    printf("  -c, --convert FILE     Convert payload or translation file" CRLF);
    printf("  -h, --help             Display this help message" CRLF);

    printf(CRLF "Examples:" CRLF);
    printf("  # Extract payloads from DBI.nro into folder DBI_extract" CRLF);
    printf("     %s --extract DBI.nro --output DBI_extract" CRLF, program_name);

    printf(CRLF "  # Convert extracted payload 6.bin into an editable text file" CRLF);
    printf("     %s --convert DBI_extract/6.bin --output translation.txt --keys keylist.txt" CRLF, program_name);

    printf(CRLF "  # Convert edited translations back into binary form" CRLF);
    printf("     %s --convert translation.txt --output DBI_extract/6.bin --keys keylist.txt" CRLF, program_name);

    printf(CRLF "  # Apply patch 6.bin to DBI.nro at slot 6 and write patched binary" CRLF);
    printf("     %s --patch 6.bin --binary DBI.nro --slot 6 --output DBI.patched.nro" CRLF, program_name);
}

static struct option long_options[] = {
    {"help",        no_argument,        0, 'h'},
    {"binary",      required_argument,  0, 'b'},
    {"patch",       required_argument,  0, 'p'},
    {"output",      required_argument,  0, 'o'},
    {"keys",        required_argument,  0, 'k'},
    {"slot",        required_argument,  0, 's'},
    {"extract",     required_argument,  0, 'e'},
    {"convert",     required_argument,  0, 'c'},
    {0, 0, 0, 0}  
};

static void free_args(Args * args) {
    if(args->binary) {
        free(args->binary);
    }
    
    if(args->convert) {
        free(args->convert);
    }
    
    if(args->extract) {
        free(args->extract);
    }
    
    if(args->output) {
        free(args->output);
    }
    
    if(args->patch) {
        free(args->patch);
    }
    
    if(args->keys) {
        free(args->keys);
    }
    
    memset(args, 0, sizeof(*args));
}

int main(int argc, char** argv) {
    
    int ret = EXIT_SUCCESS;
    
    log_init(APP_LOG);
    
    memset(&args, 0, sizeof(args));
    args.slot = -1;
    
    const char * program_name = argc ? argv[0] : APP;
    
    char short_options[ARRLEN(long_options) * 2 + 1];
    char * short_ptr = short_options;
    
    for(uint32_t i = 0; i < ARRLEN(long_options); i++) {
        struct option * opt = &long_options[i];
        *(short_ptr++) = (char)opt->val;
        if(opt->has_arg == required_argument) {
            *(short_ptr++) = ':';
        }
    }
    
    int opt_cnt = 0;
    int opt;
    int opt_idx = 0;
    while ((opt = getopt_long(argc, argv, short_options, long_options, &opt_idx)) != -1) {
        ++opt_cnt;
        
        switch (opt) {
            case 'b':   args.binary     = strdup(optarg);               break;
            case 'p':   args.patch      = strdup(optarg);               break;
            case 'o':   args.output     = strdup(optarg);               break;
            case 'e':   args.extract    = strdup(optarg);               break;
            case 'c':   args.convert    = strdup(optarg);               break;
            case 'k':   args.keys       = strdup(optarg);               break;
            case 's':   args.slot       = strtoul(optarg, NULL, 10);    break;
            case 'h':   
            default:    args.help       = 1;                            break;
        }
    }
    
    if(opt_cnt == 0) {
        lf_e("optidx = 0");
        args.help = 1;
    }
    
    if(args.help) {
        print_help(program_name);
        goto exit;
    }
    
    if(args.extract) {
        if(args.convert || args.patch) {
            printf("%s: only one option from '--extract', '--convert' or '--patch' allowed" CRLF, program_name);
            goto exit_failure;
        }
        
        if(!(args.output)) {
            printf("%s: missing option '--output'" CRLF, program_name);
            goto exit_failure;
        }
        
        // handle extract
        ret = extract(args.extract, args.output);
    }
    
    if(args.convert) {
        if(args.extract || args.patch) {
            printf("%s: only one option from '--extract', '--convert' or '--patch' allowed" CRLF, program_name);
            goto exit_failure;
        }
        
        if(!(args.output)) {
            printf("%s: missing option '--output'" CRLF, program_name);
            goto exit_failure;
        }
        
        // handle convert
        ret = convert(args.convert, args.output, args.keys);
    }
    
    if(args.patch) {
        if(args.convert || args.extract) {
            printf("%s: only one option from '--extract', '--convert' or '--patch' allowed" CRLF, program_name);
            goto exit_failure;
        }
        
        if(!(args.binary)) {
            printf("%s: missing option '--binary'" CRLF, program_name);
            goto exit_failure;
        }
        
        if(args.slot < 0) {
            printf("%s: missing option '--slot'" CRLF, program_name);
            goto exit_failure;
        }
        
        if(!(args.output)) {
            printf("%s: missing option '--output'" CRLF, program_name);
            goto exit_failure;
        }
        
        // handle patch
        ret = patch(args.binary, args.patch, args.slot, args.output);
    }
    
exit:
    free_args(&args);
    return ret;
    
exit_failure:
    free_args(&args);
    return (EXIT_FAILURE);
}

