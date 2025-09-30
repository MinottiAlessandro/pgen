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
    unsigned int len = 0;
    char *buffer;

    if(opt->lower) len += slen(lower) - 1;
    if(opt->upper) len += slen(upper) - 1;
    if(opt->digits) len += slen(digits) - 1;
    if(opt->special) len += slen(special) - 1;

    buffer = (char *) malloc((sizeof(char) * len) + 1);
    if(!buffer) {
        printf("Error generating the random string.\n");
        exit(1);
    }
    buffer[0] = '\0';

    if(opt->lower) strcat(buffer, lower);
    if(opt->upper) strcat(buffer, upper);
    if(opt->digits) strcat(buffer, digits);
    if(opt->special) strcat(buffer, special);

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

    while(i < opt->len) {
        char c = rand();

        if(opt->custom_alphabet) c = opt->custom_alphabet[c % slen(opt->custom_alphabet)];
        else c = alphabet[c % (slen(alphabet) - 1)];

        if(opt->exclude[(unsigned int) c]) continue;

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
