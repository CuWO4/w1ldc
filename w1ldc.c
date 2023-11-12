//***************************************************************************
//                                                                          *
// file name    :   w1ldc.c                                                 *
// description  :   c (subset) interpreter                                  *
// author       :   CuWO4                                                   *
// tutorial     :   https://github.com/lotabout/write-a-C-interpreter       *
//                  https://lotabout.me/2015/write-a-C-interpreter-0/       *
//                                                                          *
//***************************************************************************/

// /*...*/ style comment is not supported

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <io.h>         // includes open(), close() */

//************************ INTERPRETER **************************/
int     token;              // current token                    */
char   *src,                // source file string               */
       *src_old;
int     line;               // current line                     */
int     poolsize;           // default buffer size              */

//********************** VIRTUAL MACHINE ************************/
// memory */
int    *text,               // text segment                     */
       *text_old;
int    *stack;              // stack memory                     */
char   *data;               // data segment, only for string    */

// register */
int    *pc;                 // program counter                  */
int    *bp;                 // base address pointer             */
int    *sp;                 // stack top pointer                */
int     ax;                 // general purpose register         */
int     cycle;
// the virtual machine will use heap memory of interpreter. */

// instruction */
// sorted by the number of operands, to generate debug  */
// information easier.                                  */
enum Instruction { 
    LEA, IMM, JMP, CALL, JEZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, 
    PUSH, OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB,
    MUL, DIV, MOD, OPEN, READ, CLOS, PRTF,MALC, MSET, MCMP, EXIT,
};

//***************************************************************/

//***************************************************************
// @brief   to get next token from src.
//***************************************************************/
void next() { 
    token = *src++;
}

//***************************************************************
// @brief   to evaluate the expression.
//***************************************************************/
void expression(int level) {

}

//***************************************************************
// @brief   the entrance of the grammar syntax.
//***************************************************************/
void program() {

}

//******** Structure of Stack Frame *********
//  sub_function(arg1, arg2, arg3);
//   |    ....        | high address
//   +----------------+
//   | arg: 1         |    new_bp + 4
//   +----------------+
//   | arg: 2         |    new_bp + 3
//   +----------------+
//   | arg: 3         |    new_bp + 2
//   +----------------+
//   | return address |    new_bp + 1
//   +----------------+
//   | old bp         | <- new bp
//   +----------------+
//   | local var 1    |    new_bp - 1
//   +----------------+
//   | local var 2    |    new_bp - 2
//   +----------------+
//   |    ....        |  low address
//*******************************************/

//***************************************************************
// @brief   to run the virtual machine.
// @return  non-zero value if fails; otherwise 0.
// @note    UNSAFE! when the stack of virtual machine grows bigger than poolsize, the
//          program will crash.
//***************************************************************/
int eval() {
    int op;
    int *tmp;
    while (1) {
        op = *pc++;
        //    Instruction   |                        Description                            */
        // IMM <num>        | load immediate number to ax                                   */
        if (op == IMM)          { ax = *pc++; }
        // LC               | load character to ax, address in ax                           */
        else if (op == LC)      { ax = *(char*) ax; }
        // LI               | load interger to ax, address in ax                            */
        else if (op == LI)      { ax = *(int*)  ax; }
        // SC               | save character, value in ax, address in the top of stack      */
        else if (op == SC)      { ax = *(char*) sp = ax; }  // why not simply *(char*) sp = ax ? */
        // SI               | save interger, value in ax, address in the top of stack       */
        else if (op == SI)      { *(int*)  sp = ax; }
        // PUSH             | push the value of ax onto the stack                           */
        else if (op == PUSH)    { *--sp = ax; }
        // JMP <adr>        | jump to the address                                           */
        else if (op == JMP)     { pc = (int*) *pc; }
        // JEZ <adr>        | jump to the address if ax equals to zero                      */
        else if (op == JEZ)     { pc = ax == 0 ? (int*) *pc : pc + 1; }
        // JNZ <adr>        | jump to the address if ax does not equal to zero              */
        else if (op == JNZ)     { pc = ax != 0 ? (int*) *pc : pc + 1; }
        // CALL <adr>       | call subroutine, address in <adr>                             */
        else if (op == CALL)    { *--sp = (int) (pc + 1); pc = (int*) *pc; }
        // ENT <size>       | create a new stack frame                                      */
        else if (op == ENT)     { *--sp = (int) bp; bp = sp; sp = sp - *pc++; } // -= is not supported */
        // ADJ <size>       | remove arguments from stack frame                             */
        else if (op == ADJ)     { sp = sp + *pc++; }
        // LEV              | restore pc, stack frame, then return                          */
        else if (op == LEV)     { sp = bp; bp = (int*) *sp++; pc = (int*) *sp++; }
        // LEA <offset>     | load argument                                                 */
        else if (op == LEA)     { ax = (int) (bp + *pc++); }

        // <operation>      | operands in ax and the top of stack, save result in ax        */
        else if (op == OR)      ax = *sp++ | ax;
        else if (op == XOR)     ax = *sp++ ^ ax;
        else if (op == AND)     ax = *sp++ & ax;
        else if (op == EQ)      ax = *sp++ == ax;
        else if (op == NE)      ax = *sp++ != ax;
        else if (op == LT)      ax = *sp++ < ax;
        else if (op == LE)      ax = *sp++ <= ax;
        else if (op == GT)      ax = *sp++ >  ax;
        else if (op == GE)      ax = *sp++ >= ax;
        else if (op == SHL)     ax = *sp++ << ax;
        else if (op == SHR)     ax = *sp++ >> ax;
        else if (op == ADD)     ax = *sp++ + ax;
        else if (op == SUB)     ax = *sp++ - ax;
        else if (op == MUL)     ax = *sp++ * ax;
        else if (op == DIV)     ax = *sp++ / ax;
        else if (op == MOD)     ax = *sp++ % ax;

        // <function>       | build-in function                                             */
        else if (op == EXIT)    { printf("exit(%d)\n", *sp); return *sp;}
        else if (op == OPEN)    { ax = open((char*) sp[1], sp[0]); }
        else if (op == CLOS)    { ax = close(*sp);}
        else if (op == READ)    { ax = read(sp[2], (char*) sp[1], *sp); }
        else if (op == PRTF)    { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
        else if (op == MALC)    { ax = (int) malloc(*sp);}
        else if (op == MSET)    { ax = (int) memset((char *)sp[2], sp[1], *sp);}
        else if (op == MCMP)    { ax = memcmp((char *) sp[2], (char*) sp[1], *sp);}

        else {
            printf("Unknown instruction %d!\n", op);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {

    int tmp, file_handle;

    argc--;
    argv++;

    poolsize = 256 * 1024; // choosed randomly */
    line = 1;

    // alloc memory for virtual machine. */
    if (!(text = text_old = (int*) malloc(poolsize))) {
        printf("Failed to alloc memory for text segment!\n");
        return -1;
    }
    memset(text, 0, poolsize);

    if (!(stack = (int*)malloc(poolsize))) {
        printf("Failed to alloc memory for stack memory!\n");
        return -1;
    }
    memset(stack, 0, poolsize);

    if (!(data = (char*) malloc(poolsize))) {
        printf("Failed to alloc memory for data segment!\n");
        return -1;
    }
    memset(data, 0, poolsize);
    
    // initialize the virtual machine */
    // todo: pc = *main* */
    bp = sp = stack + poolsize;

    // open(), read() and close() are Linux functions.                          */
    // We use them since they work better with virtual machine.                 */
    // and we do not use fprintf(stderr, ...),                                  */
    // instead, we use printf() since it works better with virtual machine.     */
    if ((file_handle = open(*argv, 0)) < 0) {    // #define F_OK 0 */
        printf("Failed to open the file %s!\n", *argv);
        return -1;
    }

    if (!(src = src_old = (char*) malloc(poolsize))) {
        printf("Failed to alloc memory for source code buffer!\n");
        return -1;
    }

    if((tmp = read(file_handle, src, poolsize - 1)) <= 0) {
        printf("Failed when executing read(), read() returned %d!\n", tmp);
        return -1;
    }

    src[tmp] = 0; // '\0' */
    close(file_handle);

    program();

    return eval();
}