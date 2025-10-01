#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char lower[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','\0'};
char upper[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','\0'};
char digits[] = {'0','1','2','3','4','5','6','7','8','9','\0'};
char special[] = {' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',':',';','<','=','>','?','@','[','\\',']','^','_','`','{','|','}','~','\0'};

typedef struct Options {
    char exclude[128];
    unsigned int len;
    char flags; // This variable contains 7 flags, from the least valuable: upper, lower, digits, special, help, custom, exclude
    char *custom_alphabet;
} Options;

int get_flag(char c, Options *opt) {
    switch(c) {
        case 'u': return opt->flags & (1 << 0); // Return upper flag status
        case 'l': return opt->flags & (1 << 1); // Return lower flag status
        case 'd': return opt->flags & (1 << 2); // Return digit flag status
        case 's': return opt->flags & (1 << 3); // Return special flag status
        case 'h': return opt->flags & (1 << 4); // Return help flag status
        case 'c': return opt->flags & (1 << 5); // Return custom flag status
        case 'x': return opt->flags & (1 << 6); // Return exclude flag status
        default: return 0;
    }
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

void error_handler(int err_no) {
    switch(err_no) {
        case 1: printf("Error generating the random string.\n"); break;
        case 2: printf("Impossible to create a password, no alphabet to work on.\n"); break;
        case 3: printf("Add an argument to specify the password lenght.\n"); break;
        case 4: printf("The string length must be an integer greater than 0.\n"); break;
        case 5: printf("Specify a custom alphabet.\n"); break;
        default: return;
    }
    print_help();
    exit(-1);
}

unsigned int slen(char *s) {
    unsigned int i = 0;
    while(s[i] != '\0') ++i;
    return i;
}

void scat(char *buffer, char *alpha, char *exclude) {
    int j = slen(buffer);
    for(unsigned int i = 0; i < slen(alpha); i++) {
        if(exclude[(unsigned int) alpha[i]]) continue;
        buffer[j] = alpha[i];
        ++j;
    }

}

int parse_argument(Options *opt, int argc, char *argv[]) {
    if(argc < 2) return 3;

    for(int i = 1; i < argc - 1; i++) {
        int skip = 0;
        int j = 1;
        while(argv[i][j] != '\0') {
            switch(argv[i][j]) {
                case '-': break;
                case 'u': opt->flags = opt->flags ^ (1 << 0); break;
                case 'l': opt->flags = opt->flags ^ (1 << 1); break;
                case 'd': opt->flags = opt->flags ^ (1 << 2); break;
                case 's': opt->flags = opt->flags ^ (1 << 3); break;
                case 'c': 
                if(argc - i <= 1) return 5;
                opt->custom_alphabet = argv[i+1];
                skip = 1;
                opt->flags = opt->flags ^ (1 << 5);
                break;
                case 'x': 
                if(argc - i <= 1) return 5;
                int z = 0;
                while(argv[i+1][z] != '\0') {
                    opt->exclude[argv[i+1][z] % 128] = 1;
                    ++z;
                }
                skip = 1;
                opt->flags = opt->flags ^ (1 << 6);
                break;
                case 'h': opt->flags = opt->flags ^ (1 << 4); return 1;
                default: return 1;
            }
            ++j;
        }

        if(skip) ++i;
    }

    if(opt->flags == '\0') opt->flags = 0b00001111;

    int len = atoi(argc[argv-1]);
    if(len <= 0) return 4;
    opt->len = (unsigned int) len;
    return 0;
}

char* build_alphabet(Options *opt) {
    char *buffer;
    buffer = (char *) malloc(sizeof(char) * 96);
    
    if(!buffer) error_handler(1);

    for(int i = 0; i < 96; i++) buffer[i] = '\0';

    if(get_flag('l', opt)) scat(buffer, lower, opt->exclude);
    if(get_flag('u', opt)) scat(buffer, upper, opt->exclude);
    if(get_flag('d', opt)) scat(buffer, digits, opt->exclude);
    if(get_flag('s', opt)) scat(buffer, special, opt->exclude);

    return buffer;
}

void generate_password(char *p, Options *opt) {
    FILE *f = fopen("/dev/urandom", "rb");
    uint64_t state = 0;
    char *alphabet;
    unsigned int alphabet_len = 0;

    if(f == 0) error_handler(1);
    
    while(state == 0) fread(&state, sizeof(uint64_t), 1, f);
    fclose(f);

    alphabet = build_alphabet(opt);
    alphabet_len = slen(alphabet);

    if(alphabet_len == 0) error_handler(2);

    for(unsigned int i = 0; i < opt->len; i++) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;

        p[i] = alphabet[state % alphabet_len];
    }

    p[opt->len] = '\0';
}

int main(int argc, char* argv[]) {

    char *p;
    Options opt = {{0}, 0, '\0', NULL};

    error_handler(parse_argument(&opt, argc, argv));
    
    p = (char *) malloc((sizeof(char) * opt.len) + 1);
    generate_password(p, &opt);
    printf("%s\n", p);
    free(p);

    return 0;
}
