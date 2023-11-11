#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <io.h>

int token;              /* current token */
char *src, *src_old;    /* source file string */
int line;               /* current line */
int buffer_size;        /* default buffer size */

/**
 * @brief to get next token from src.
**/
void next(void) { 
    token = *src++;
}

/**
 * @brief to evaluate the expression.
**/
void expression(int level) {

}

/**
 * @brief the entrance of the grammar syntax.
**/
void program(void) {
    do {
        next();
        printf("%c", token);
    } while(token > 0);
}

/**
 * @brief to run the virtual machine.
 * @return non-zero value if fails; otherwise 0.
**/
int eval(void) {
    return 0;
}

int main(int argc, char **argv) {
    int tmp, file_handle;

    argc--;
    argv++;

    buffer_size = 256 * 1024; /* choosed randomly */
    line = 1;

    /* open(), read() and close() are Linux functions.                          */
    /* We use them since they work better with virtual machine.                 */
    /* and we do not use fprintf(stderr, ...),                                  */
    /* instead, we use printf() since it works better with virtual machine.     */
    if ((file_handle = open(*argv, F_OK)) < 0) {
        printf("Failed to open the file %s!\n", *argv);
        return -1;
    }

    if (!(src = src_old = (char*)malloc(buffer_size))) {
        printf("Failed to alloc memory!\n");
        return -1;
    }

    if((tmp = read(file_handle, src, buffer_size - 1)) <= 0) {
        printf("Failed when executing read(), read() returned %d!\n", tmp);
        return -1;
    }

    src[tmp] = '\0';
    close(file_handle);

    program();
    return eval();
}