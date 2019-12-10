#ifndef __INTCODE_H__
#define __INTCODE_H__

#define ASCII_IN  (1<<0)
#define ASCII_OUT (1<<1)
#define TOLERANT  (1<<2)

void intcode_run(long long *code, int codeLen, long long *input, int inputLen, int settings);

#endif
