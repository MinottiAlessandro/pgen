#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Options {
    unsigned int alpha;
    unsigned int up_low;
    unsigned int digits;
    unsigned int special_char;
    unsigned int help;
    unsigned int len;
    char *custom_alphabet;
    char *exclude;
} Options;

void print_help() {
    printf("pgen usage:\
    \npgen [opt] <int> [the parameters past the integer are ignored]\
    \n\t-a\tInclude all alpha characters.\
    \n\t-u\tInclude uppercase letters.\
    \n\t-l\tInclude lowercase letters.\
    \n\t-d\tInclude digits.\
    \n\t-s\tInclude special characters.\
    \n\t-c str\tInclude custom alphabet. (Will overwrite other params)\
    \n\t-x str\tExclude specific characters.\
    \n\t-h\tDisplay this help page.\n");
}

unsigned int in(char c, char *s) {
    for(int i = 0; i < strlen(s); i++) {
        if(c == s[i]) return 1;
    }
    return 0;
}

int parse_argument(Options *opt, int argc, char *argv[]) {
    
    int len = -1;

    if(argc < 2) return -1;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-a") == 0) {
            opt->alpha = 1;
            continue;
        }
        if(strcmp(argv[i], "-u") == 0) {
            opt->up_low = 1;
            continue;
        }
        if(strcmp(argv[i], "-l") == 0) {
            opt->up_low = 2;
            continue;
        }
        if(strcmp(argv[i], "-d") == 0) {
            opt->digits = 1;
            continue;
        }
        if(strcmp(argv[i], "-s") == 0) {
            opt->special_char = 1;
            continue;
        }
        if(strcmp(argv[i], "-c") == 0) {
            if(argc - i <= 1) {
                return -3;
            }
            opt->custom_alphabet = (char *) malloc(strlen(argv[i+1]) * sizeof(char));
            strcpy(opt->custom_alphabet, argv[i+1]);
            ++i;
            continue;
        }
        if(strcmp(argv[i], "-x") == 0) {
            if(argc - i <= 1) {
                return -3;
            }
            opt->exclude = (char *) malloc(strlen(argv[i+1]) * sizeof(char));
            strcpy(opt->exclude, argv[i+1]);
            ++i;
            continue;
        }
        if(strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        }
        if((len = atoi(argv[i])) <= 0) {
            return -2;
        } else {
            return len;
        }
    }

    return len;
}

int truth_machine(char c, Options *opt) {
    return (
        ((c >= 48 && c <= 57) && opt->digits) ||
        ((c >= 65 && c <= 90) && opt->up_low == 1) ||
        ((c >= 97 && c <= 122) && opt->up_low == 2) ||
        (((c >= 65 && c <= 90) || (c >= 97 && c <= 122)) && opt->alpha) ||
        (((c >= 32 && c <= 47) ||(c >= 58 && c <= 64) ||(c >= 91 && c <= 96) ||(c >= 123 && c <= 126)) && opt->special_char) ||
        ((!opt->alpha) && (!opt->up_low) && (!opt->special_char) && (!opt->digits) && (!opt->custom_alphabet))
    );
}

void generate_password(char *p, Options *opt) {
    FILE *f = fopen("/dev/urandom", "rb");
    int i = 0;

    if(f == 0) {
        printf("Error generating the random string.\n");
        exit(1);
    }

    while(i < opt->len) {
        char c = fgetc(f);

        if(opt->custom_alphabet) c = opt->custom_alphabet[c % strlen(opt->custom_alphabet)];
        else {
            c = c % 128;
            if((c < 32 || c > 126) || !truth_machine(c, opt)) continue;
        }

        if(opt->exclude && in(c, opt->exclude)) continue;

        p[i++] = c;
    }

    fclose(f);
}

int main(int argc, char* argv[]) {

    char *p;
    Options opt = {0, 0, 0, 0, 0, 0, NULL};

    switch(opt.len = parse_argument(&opt, argc, argv)) {
        case -1:
            printf("Add an argument to specify the password lenght.\n");
            print_help();
            return -1;
        case -2:
            printf("The string length must be an integer greater than 0.\n");
            print_help();
            return -2;
        case -3:
            printf("Specify a custom alphabet.\n");
            print_help();
            return -3;
        default: break;
    }
    
    p = (char *) malloc((sizeof(char) * opt.len) + 1);

    generate_password(p, &opt);

    printf("%s\n", p);

    free(p);
    if(opt.custom_alphabet) free(opt.custom_alphabet);
    if(opt.exclude) free(opt.exclude);

    return 0;
}
