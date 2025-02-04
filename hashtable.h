#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>

typedef unsigned int int32;
typedef unsigned short int int16;
typedef unsigned char int8;

#define hash(x, y) hash_naive(x, y)

struct s_hash
{
    unsigned int hash : 21;
};

typedef struct s_hash Hash;

void zero(int8 *, int16);
Hash hash_naive(int8 *, int16);
int main(int, char **);
int16 Example_genhashes(int8 *, int8 *);