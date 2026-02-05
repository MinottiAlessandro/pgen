#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#define MAX_THREADS 24

char lower[] = "abcdefghijklmnopqrstuvwxyz";
char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char digits[] = "0123456789";
char special[] = "!@#$%^&*";
char more_special[] = " \"'()+,-./:;<=>?[\\]_`{|}~";

typedef struct Options {
    char exclude[128];
    unsigned int len;
    int threads;
    unsigned short flags;
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

int get_flags(char *s, Options *opt) {
    int result = 0;
    
    while(*s) {
        switch(*s++) {
            case 'u': result += opt->flags & (1 << 0); continue; // Return upper flag status
            case 'l': result += opt->flags & (1 << 1); continue; // Return lower flag status
            case 'd': result += opt->flags & (1 << 2); continue; // Return digit flag status
            case 's': result += opt->flags & (1 << 3); continue; // Return special flag status
            case 'h': result += opt->flags & (1 << 4); continue; // Return help flag status
            case 'c': result += opt->flags & (1 << 5); continue; // Return custom flag status
            case 'x': result += opt->flags & (1 << 6); continue; // Return exclude flag status
            case 't': result += opt->flags & (1 << 7); continue; // Return threads flag status
            case '+': result += opt->flags & (1 << 8); continue; // Return more special flag status
            default: return 0;
        }
    }

    printf("RESULT: %d\n", result);
    return result;
}

void set_flags(char *s, Options *opt) {
    while(*s) {
        switch(*s++) {
            case 'u': opt->flags ^= (1 << 0); continue; // Set upper flag status
            case 'l': opt->flags ^= (1 << 1); continue; // Set lower flag status
            case 'd': opt->flags ^= (1 << 2); continue; // Set digit flag status
            case 's': opt->flags ^= (1 << 3); continue; // Set special flag status
            case 'h': opt->flags ^= (1 << 4); continue; // Set help flag status
            case 'c': opt->flags ^= (1 << 5); continue; // Set custom flag status
            case 'x': opt->flags ^= (1 << 6); continue; // Set exclude flag status
            case 't': opt->flags ^= (1 << 7); continue; // Set threads flag status
            case '+': opt->flags ^= (1 << 8); continue; // Set more special flag status
            default: continue;
        }
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
                case 'u': set_flags("u", opt); break;
                case 'l': set_flags("l", opt); break;
                case 'd': set_flags("d", opt); break;
                case 's': set_flags("s", opt); break;
                case '+': set_flags("+", opt); break;
                case 'c': 
                    if(argc - i <= 1) return 5;
                    opt->custom_alphabet = argv[i+1];
                    skip = 1;
                    set_flags("c", opt);
                    break;
                case 'x': 
                    if(argc - i <= 1) return 5;
                    int z = 0;
                    while(argv[i+1][z] != '\0') {
                        opt->exclude[argv[i+1][z] % 128] = 1;
                        ++z;
                    }
                    skip = 1;
                    set_flags("x", opt);
                    break;
                case 't': 
                    if(argc - i <= 1) return 5;
                    opt->threads = atoi(argv[i+1]);
                    if(opt->threads > MAX_THREADS) opt->threads = MAX_THREADS;
                    else if(opt->threads <= 0) opt->threads = 1;
                    set_flags("t", opt);
                    skip = 1;
                    break;
                case 'h': set_flags("h", opt); return 1;
                default: return 1;
            }
            ++j;
        }

        if(skip) ++i;
    }

    if(get_flags("+", opt)) set_flags("s", opt);
    if(!get_flags("ludsc+", opt)) set_flags("luds", opt);

    unsigned int len = atoi(argv[argc-1]);
    if(len <= 0) return 4;
    opt->len = (unsigned int) len;
    return 0;
}

char* build_alphabet(Options *opt) {
    char *buffer = (char *) malloc(sizeof(char) * 96);
    
    if(!buffer) error_handler(1);

    for(int i = 0; i < 96; i++) buffer[i] = '\0';

    if(get_flags("c", opt)) scat(buffer, opt->custom_alphabet, opt->exclude);
    else {
        if(get_flags("l", opt)) scat(buffer, lower, opt->exclude);
        if(get_flags("u", opt)) scat(buffer, upper, opt->exclude);
        if(get_flags("d", opt)) scat(buffer, digits, opt->exclude);
        if(get_flags("s", opt)) scat(buffer, special, opt->exclude);
        if(get_flags("+", opt)) scat(buffer, more_special, opt->exclude);
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
