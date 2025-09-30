#include <stdio.h>
#include <stdlib.h>

typedef struct Options {
    unsigned int upper;
    unsigned int lower;
    unsigned int digits;
    unsigned int special_char;
    unsigned int help;
    unsigned int len;
    char *custom_alphabet;
    char *exclude;
} Options;

unsigned int slen(char *s) {
    unsigned int i = 0;
    while(s[i] != '\0') ++i;
    return i;
}

void print_help() {
    printf("pgen usage:\
    \npgen [opt] <int> [the parameters past the integer are ignored]\
    \n\t-u\tInclude uppercase letters.\
    \n\t-l\tInclude lowercase letters.\
    \n\t-d\tInclude digits.\
    \n\t-s\tInclude special characters.\
    \n\t-c str\tInclude custom alphabet. (Will overwrite other params)\
    \n\t-x str\tExclude specific characters.\
    \n\t-h\tDisplay this help page.\n");
}

unsigned int in(char c, char *s) {
    for(unsigned int i = 0; i < slen(s); i++) {
        if(c == s[i]) return 1;
    }
    return 0;
}

int parse_argument(Options *opt, int argc, char *argv[]) {
    if(argc < 2) return -1;

    for(int i = 1; i < argc - 1; i++) {
        int skip = 0;
        int j = 1;
        while(argv[i][j] != '\0') {
            switch(argv[i][j]) {
                case '-': ++j; break;
                case 'u': opt->upper = 1; break;
                case 'l': opt->lower = 1; break;
                case 'd': opt->digits = 1; break;
                case 's': opt->special_char = 1; break;
                case 'c': 
                    if(argc - i <= 1) return -3;
                    opt->custom_alphabet = argv[i+1];
                    skip = 1;
                    break;
                case 'x': 
                    if(argc - i <= 1) return -3;
                    opt->exclude = argv[i+1];
                    skip = 1;
                    break;
                case 'h': return 1;
                default: return 1;
            }
            ++j;
        }

        if(skip) ++i;
    }

    int len = atoi(argc[argv-1]);
    if(len <= 0) return -2;
    opt->len = (unsigned int) len;

    return 0;
}

int truth_machine(char c, Options *opt) {
    return (
        ((c >= 48 && c <= 57) && opt->digits) ||
        ((c >= 65 && c <= 90) && opt->upper == 1) ||
        ((c >= 97 && c <= 122) && opt->lower == 1) ||
        (((c >= 32 && c <= 47) ||(c >= 58 && c <= 64) ||(c >= 91 && c <= 96) ||(c >= 123 && c <= 126)) && opt->special_char) ||
        ((!opt->upper) && (!opt->lower) && (!opt->special_char) && (!opt->digits) && (!opt->custom_alphabet))
    );
}

void generate_password(char *p, Options *opt) {
    FILE *f = fopen("/dev/urandom", "rb");
    unsigned int i = 0;

    if(f == 0) {
        printf("Error generating the random string.\n");
        exit(1);
    }

    while(i < opt->len) {
        char c = fgetc(f);

        if(opt->custom_alphabet) c = opt->custom_alphabet[c % slen(opt->custom_alphabet)];
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
    Options opt = {0, 0, 0, 0, 0, 0, NULL, NULL};

    switch(parse_argument(&opt, argc, argv)) {
        case 1:
            print_help();
            return 0;
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

    return 0;
}
