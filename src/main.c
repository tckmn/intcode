#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intcode.h"

#define FAIL do { help(argv[0]); return 1; } while (0)
void help(char *name) {
    printf("usage: %s [FLAG...] [FILE]\n"
           "flags:\n"
           "  -r    read input as ints (default)\n"
           "  -R    read input as ASCII\n"
           "  -w    write output as ints (default)\n"
           "  -W    write output as ASCII\n"
           "  -e    provide code as string\n"
           "  -i    provide input as string\n"
           "  -t    tolerant mode (do not error on unknown opcodes)\n", name);
}

long long *parse_string(char *data, int *len) {
    int bufsize = 256;
    *len = 0;
    long long *buf = malloc(bufsize * sizeof *buf);

    long long cur = 0;
    int sign = 0;

    for (;; ++data) {
        if ('0' <= *data && *data <= '9') {
            if (sign) cur *= 10, cur += *data - '0';
            else sign = 1, cur = *data - '0';
        } else {
            if (sign) {
                if (*len == bufsize) buf = realloc(buf, (bufsize*=2) * sizeof *buf);
                buf[(*len)++] = cur*sign;
                sign = 0;
            } else if (*data == '-' && ('0' <= data[1] && data[1] <= '9')) {
                cur = 0;
                sign = -1;
            }
        }

        if (!*data) break;
    }

    return buf;
}

long long *parse_file(char *fname, int *len) {
    long long *res;
    int bufsize = 512, nr;
    char *buf = malloc(bufsize * sizeof *buf);
    FILE *f = fopen(fname, "r");

    if ((nr = fread(buf, 1, bufsize, f)) != bufsize) {
        buf[nr] = 0;
        res = parse_string(buf, len);
        free(buf);
        fclose(f);
        return res;
    }

    do {
        buf = realloc(buf, (bufsize*=2) * sizeof *buf);
    } while ((nr = fread(buf+bufsize/2, 1, bufsize/2, f)) == bufsize/2);
    buf[bufsize/2+nr] = 0;

    res = parse_string(buf, len);
    free(buf);
    fclose(f);
    return res;
}

int main(int argc, char **argv) {
    long long *code = 0, *input = 0;
    int codeLen, inputLen;
    int settings = 0;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            char *flags = argv[i]+1, *flagData;
            for (;;) {
                switch (*flags) {
                case 0: goto endloop;
                case 'r': settings &= ~ASCII_IN; ++flags; break;
                case 'R': settings |= ASCII_IN; ++flags; break;
                case 'w': settings &= ~ASCII_OUT; ++flags; break;
                case 'W': settings |= ASCII_OUT; ++flags; break;
                case 'e': case 'i':
                    flagData = flags[1] ? flags+1 : i+1 < argc ? argv[++i] : 0;
                    if (!flagData) FAIL;
                    if (*flags == 'e') {
                        if (code) FAIL;
                        code = parse_string(flagData, &codeLen);
                    } else {
                        if (input) FAIL;
                        if (settings & ASCII_IN) {
                            inputLen = strlen(flagData);
                            input = malloc(inputLen * sizeof *input);
                            for (int j = 0; j < inputLen; ++j) input[j] = flagData[j];
                        } else input = parse_string(flagData, &inputLen);
                    }
                    goto endloop;
                case 't': settings |= TOLERANT; ++flags; break;
                default: FAIL;
                }
            }
            endloop: continue;
        }

        if (code) FAIL;
        code = parse_file(argv[i], &codeLen);
    }

    if (!code) FAIL;
    intcode_run(code, codeLen, input, inputLen, settings);
    return 0;
}
