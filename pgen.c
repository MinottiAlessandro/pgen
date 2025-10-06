#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#define MAX_THREADS 24

char lower[] = "abcdefghijklmnopqrstuvwxyz";
char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char digits[] = "0123456789";
char special[] = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

typedef struct Options {
    char exclude[128];
    unsigned int len;
    int threads;
    char flags;
    char *custom_alphabet;
} Options;

typedef struct Alphabet {
    char *alpha;
    unsigned int len;
} Alphabet;

typedef struct Arguments {
    int id;
    char *p;
    uint64_t seed;
    Options *opt;
    Alphabet *alphabet;
} Arguments;

int get_flag(char c, Options *opt) {
    switch(c) {
        case 'u': return opt->flags & (1 << 0); // Return upper flag status
        case 'l': return opt->flags & (1 << 1); // Return lower flag status
        case 'd': return opt->flags & (1 << 2); // Return digit flag status
        case 's': return opt->flags & (1 << 3); // Return special flag status
        case 'h': return opt->flags & (1 << 4); // Return help flag status
        case 'c': return opt->flags & (1 << 5); // Return custom flag status
        case 'x': return opt->flags & (1 << 6); // Return exclude flag status
        case 't': return opt->flags & (1 << 7); // Return threads flag status
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
    \n\t-t int\tNumber of threads (Defaults: 1)\
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
        if(exclude[(unsigned char) alpha[i]]) continue;
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
                case 'u': opt->flags ^= (1 << 0); break;
                case 'l': opt->flags ^= (1 << 1); break;
                case 'd': opt->flags ^= (1 << 2); break;
                case 's': opt->flags ^= (1 << 3); break;
                case 'c': 
                    if(argc - i <= 1) return 5;
                    opt->custom_alphabet = argv[i+1];
                    skip = 1;
                    opt->flags ^= (1 << 5);
                    break;
                case 'x': 
                    if(argc - i <= 1) return 5;
                    int z = 0;
                    while(argv[i+1][z] != '\0') {
                        opt->exclude[argv[i+1][z] % 128] = 1;
                        ++z;
                    }
                    skip = 1;
                    opt->flags ^= (1 << 6);
                    break;
                case 't': 
                    if(argc - i <= 1) return 5;
                    opt->threads = atoi(argv[i+1]);
                    if(opt->threads > MAX_THREADS) opt->threads = MAX_THREADS;
                    else if(opt->threads <= 0) opt->threads = 1;
                    opt->flags ^= (1 << 7);
                    skip = 1;
                    break;
                case 'h': opt->flags ^= (1 << 4); return 1;
                default: return 1;
            }
            ++j;
        }

        if(skip) ++i;
    }

    if(!(opt->flags & 0b00101111)) opt->flags = 0b00001111;

    unsigned int len = atoi(argv[argc-1]);
    if(len <= 0) return 4;
    opt->len = (unsigned int) len;
    return 0;
}

char* build_alphabet(Options *opt) {
    char *buffer = (char *) malloc(sizeof(char) * 96);
    
    if(!buffer) error_handler(1);

    for(int i = 0; i < 96; i++) buffer[i] = '\0';

    if(get_flag('c', opt)) scat(buffer, opt->custom_alphabet, opt->exclude);
    else {
        if(get_flag('l', opt)) scat(buffer, lower, opt->exclude);
        if(get_flag('u', opt)) scat(buffer, upper, opt->exclude);
        if(get_flag('d', opt)) scat(buffer, digits, opt->exclude);
        if(get_flag('s', opt)) scat(buffer, special, opt->exclude);
    }

    return buffer;
}

void* generate_password(void *arg) {
    Arguments *args = (struct Arguments *) arg;
    unsigned int chunk = args->opt->len / args->opt->threads;
    unsigned int i = chunk * args->id;
    
    if(args->id == args->opt->threads-1) chunk = chunk + (args->opt->len % args->opt->threads);

    for(unsigned int j = i; j < i + chunk; j++) {
        args->seed ^= args->seed << 13;
        args->seed ^= args->seed >> 7;
        args->seed ^= args->seed << 17;

        args->p[j] = args->alphabet->alpha[args->seed % args->alphabet->len];
    }

    if(args->id == args->opt->threads-1) args->p[args->opt->len] = '\0';

    return NULL;
}
int main(int argc, char* argv[]) {
    char *p;
    Options opt = {{0}, 0, 1, '\0', NULL};
    Alphabet alphabet = {NULL, 0};
    Arguments arg[MAX_THREADS];
    pthread_t threads[MAX_THREADS];
    FILE *f = fopen("/dev/urandom", "rb");

    error_handler(parse_argument(&opt, argc, argv));
    
    if(f == 0) error_handler(1);

    p = (char *) malloc((sizeof(char) * opt.len) + 1);
    if(!p) error_handler(1);

    alphabet.alpha = build_alphabet(&opt);
    alphabet.len = slen(alphabet.alpha);
    if(alphabet.len == 0) error_handler(2);

    
    for(int i = 0; i < opt.threads; i++) {
        arg[i].alphabet = &alphabet;
        arg[i].opt = &opt;
        arg[i].p = p;
        arg[i].id = i;
        if(!fread(&arg[i].seed, sizeof(uint64_t), 1, f)) error_handler(1);
        pthread_create(&threads[i], NULL, generate_password, &arg[i]);
    }
    
    for(int i = 0; i < opt.threads; i++) pthread_join(threads[i], NULL);

    printf("%s\n", p);
    
    free(p);
    free(alphabet.alpha);
    fclose(f);

    return 0;
}
