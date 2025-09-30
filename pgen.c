#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char lower[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', '\0'};
char upper[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z', '\0'};
char digits[] = {'0','1','2','3','4','5','6','7','8','9', '\0'};
char special[] = {' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',':',';','<','=','>','?','@','[','\\',']','^','_','`','{','|','}','~', '\0'};

typedef struct Options {
    char exclude[128];
    unsigned int upper;
    unsigned int lower;
    unsigned int digits;
    unsigned int special;
    unsigned int help;
    unsigned int len;
    char *custom_alphabet;
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
                case 's': opt->special = 1; break;
                case 'c': 
                    if(argc - i <= 1) return -3;
                    opt->custom_alphabet = argv[i+1];
                    skip = 1;
                    break;
                case 'x': 
                    if(argc - i <= 1) return -3;
                    int z = 0;
                    while(argv[i+1][z] != '\0') {
                        opt->exclude[argv[i+1][z] % 128] = 1;
                        ++z;
                    }
                    skip = 1;
                    break;
                case 'h': return 1;
                default: return 1;
            }
            ++j;
        }

        if(skip) ++i;
    }

    if(!opt->custom_alphabet && !opt->digits && !opt->lower && ! opt->special && !opt->upper) {
        opt->digits = 1;
        opt->lower = 1;
        opt->upper = 1;
        opt->special = 1;
    }

    int len = atoi(argc[argv-1]);
    if(len <= 0) return -2;
    opt->len = (unsigned int) len;
    return 0;
}

char* build_alphabet(Options *opt) {
    char *buffer;
    buffer = (char *) malloc(sizeof(char) * 96);
    if(!buffer) {
        printf("Error generating the random string.\n");
        exit(1);
    }

    for(int i = 0; i < 96; i++) buffer[i] = '\0';

    if(opt->lower) {
        if(!opt->exclude) strcat(buffer, lower);
        else {
            int j = slen(buffer);
            for(unsigned int i = 0; i < slen(lower); i++) {
                if(opt->exclude[(unsigned int) lower[i]]) continue;
                buffer[j] = lower[i];
                ++j;
            }
        }
    }
    if(opt->upper) {
        if(!opt->exclude) strcat(buffer, upper);
        else {
            int j = slen(buffer);
            for(unsigned int i = 0; i < slen(upper); i++) {
                if(opt->exclude[(unsigned int) upper[i]]) continue;
                buffer[j] = upper[i];
                ++j;
            }
        }
    }
    if(opt->digits) {
        if(!opt->exclude) strcat(buffer, digits);
        else {
            int j = slen(buffer);
            for(unsigned int i = 0; i < slen(digits); i++) {
                if(opt->exclude[(unsigned int) digits[i]]) continue;
                buffer[j] = digits[i];
                ++j;
            }
        }
    }
    if(opt->special) {
        if(!opt->exclude) strcat(buffer, special);
        else {
            int j = slen(buffer);
            for(unsigned int i = 0; i < slen(special); i++) {
                if(opt->exclude[(unsigned int) special[i]]) continue;
                buffer[j] = special[i];
                ++j;
            }
        }
    }

    return buffer;
}

void generate_password(char *p, Options *opt) {
    FILE *f = fopen("/dev/urandom", "rb");
    unsigned int i = 0;
    char *alphabet;

    if(f == 0) {
        printf("Error generating the random string.\n");
        exit(1);
    }
    
    srand((unsigned int) fgetc(f));
    fclose(f);

    alphabet = build_alphabet(opt);

    if(slen(alphabet) == 0) {
        printf("Impossible to create a password, no alphabet to work on.\n");
        exit(0);
    }

    while(i < opt->len) {
        unsigned int rnd = rand();
        
        char c = alphabet[rnd % (slen(alphabet) - 1)];

        p[i++] = c;
    }

    p[opt->len] = '\0';
}

int main(int argc, char* argv[]) {

    char *p;
    Options opt = {{0}, 0, 0, 0, 0, 0, 0, NULL};

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
