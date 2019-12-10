#include <stdio.h>
#include <stdlib.h>
#include "intcode.h"

#define r(n) (0 <= (n) && (n) < codeLen ? code+n  : get_ref(&extra, (n)))
#define v(n) (0 <= (n) && (n) < codeLen ? code[n] : get_val(extra, (n)))
#define NO_INPUT -1

long long *get_ref(long long **extra, long long n) {
    for (int i = 0; i < **extra; ++i) {
        if ((*extra)[2*i+1] == n) return *extra + 2*i+2;
    }
    *extra = realloc(*extra, (1+2*++**extra) * sizeof **extra);
    (*extra)[2***extra-1] = n;
    (*extra)[2***extra] = 0;
    return *extra + 2***extra;
}

long long get_val(long long *extra, long long n) {
    for (int i = 0; i < *extra; ++i) {
        if (extra[2*i+1] == n) return extra[2*i+2];
    }
    return 0;
}

void intcode_run(long long *code, int codeLen, long long *input, int inputLen, int settings) {
    int ip = 0, base = 0, inputIdx = 0, ch = -2;
    long long x, y, z;
    long long *extra = malloc(sizeof *extra);
    extra[0] = 0;

    for (;;) {
        long long ins = v(ip);
        x = (ins/100)%10; y = (ins/1000)%10; z = (ins/10000)%10;
        x = x == 0 ? v(ip+1) : x == 1 ? ip+1 : x == 2 ? base+v(ip+1) : 0;
        y = y == 0 ? v(ip+2) : y == 1 ? ip+2 : y == 2 ? base+v(ip+2) : 0;
        z = z == 0 ? v(ip+3) : z == 1 ? ip+3 : z == 2 ? base+v(ip+3) : 0;

        switch (ins % 100) {
        case 1: *r(z) = v(x) + v(y); ip += 4; break;
        case 2: *r(z) = v(x) * v(y); ip += 4; break;
        case 3:
            if (input) *r(x) = inputIdx == inputLen ? NO_INPUT : input[inputIdx++];
            else if (settings & ASCII_IN) *r(x) = (ch = getchar()) == EOF ? NO_INPUT : ch;
            else {
                long long val = 0;
                int sign = 0, hasdigits = 0;
                if (ch == -2) ch = getchar();
                do {
                    if ('0' <= ch && ch <= '9') {
                        if (!sign) sign = 1;
                        hasdigits = 1;
                        val *= 10;
                        val += ch - '0';
                    }
                    else if (!sign && ch == '-') sign = -1;
                    else if (sign) { if (hasdigits) break; else sign = 0; }
                } while ((ch = getchar()) != EOF);
                *r(x) = sign ? val*sign : NO_INPUT;
            }
            ip += 2; break;
        case 4:
            if (settings & ASCII_OUT) putchar(v(x));
            else printf("%lld\n", v(x));
            ip += 2; break;
        case 5: ip = v(x) ? v(y) : ip+3; break;
        case 6: ip = v(x) ? ip+3 : v(y); break;
        case 7: *r(z) = v(x) < v(y); ip += 4; break;
        case 8: *r(z) = v(x) == v(y); ip += 4; break;
        case 9: base += v(x); ip += 2; break;
        case 99: goto quit;
        default:
             fprintf(stderr, "unknown opcode %lld\n", ins%100);
             if (!(settings & TOLERANT)) return;
             ip += 1;
        }
    }

quit:
    free(extra);
}
