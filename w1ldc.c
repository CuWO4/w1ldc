//***************************************************************************
//                                                                          *
// file name    :   w1ldc.c                                                 *
// description  :   c (subset) interpreter                                  *
// author       :   CuWO4                                                   *
// tutorial     :   https://github.com/lotabout/write-a-C-interpreter       *
//                  https://lotabout.me/2015/write-a-C-interpreter-0/       *
// source       :   c4 https://github.com/rswier/c4                         *
//                                                                          *
//***************************************************************************/

//************************ Not Supported Behavior ***************************
//  1. macro                                                                *
//  2. standard library function except exit(), open(), close(), read()     *
//      printf(), malloc(), memset(), memcpy()                              *
//  3. declare variable in the middle of function                           *
//  4. initialize variable                                                  *
//  5. stuct                                                                *
//  6. for-style loop                                                       *
//  7. octal and hexadecimal numbers                                        *
//  9. special escapes except '\n'                                          *
//  8. typedef                                                              *
//  9. /*...*/ style comment                                                *
//***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <io.h>         // includes open(), close() */

int debug; // debug switch */

//************************ INTERPRETER **************************
int     token;              // current token                    *
char   *src,                // source file string               *
       *src_old;
int     line;               // current line                     *
int     poolsize;           // default buffer size              *

int    *idmain;             // main function                    *

//********************** VIRTUAL MACHINE ************************
// memory */
int    *text,               // text segment                     *
       *text_old;
int    *stack;              // stack memory                     *
char   *data;               // data segment, only for string    *

// register */
int    *pc;                 // program counter                  *
int    *bp;                 // base address pointer             *
int    *sp;                 // stack top pointer                *
int     ax;                 // general purpose register         *
int     cycle;
// the virtual machine will use heap memory of interpreter */

// instruction */
// sorted by the number of operands, to generate debug  */
// information easier                                  */
enum Instruction { 
    LEA, IMM, JMP, CALL, JEZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, 
    PUSH, OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB,
    MUL, DIV, MOD, OPEN, READ, CLOS, PRTF,MALC, MSET, MCMP, EXIT,
};

//*************************** Lexer *****************************/
// token */
// operators are sorted by precedence order */
enum Token {
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, 
    Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

int     token_val;          // current token value              *
int    *current_id,         // current parser id                *
       *symbols;            // symbol table                     *

// offsets of symbol struct, since struct is not supported      *
enum Symbol_Offset {
    Token,  // token type, Id or reserved words                 *
    Hash,   // hash value of token, for fast comparing          *
    Name,   // literal of token                                 *
    Type,   // type of token (int / double / pointer...)        *
    Class,  // global / local                                   *
    Value,  // value (for variable) or address (for function)   *   // ? */
    BType, BClass, BValue, // save the same-name global token   *
    IdSize,
};

enum Type { CHAR, INT, PTR, };

//************************** Parser *****************************


//***************************************************************/

//***************************************************************
// @brief   to get next token from src
// @note    relavant results are restored in global variables
//***************************************************************/
void next() { 
    char *last_pos;
    int hash;

    while ((token = *src++)) {
        if (token == ' ' || token == '\t');
        else if (token == '\n') {
            line++;
        }
        // skip macros, since we do not support them */
        else if (token == '#') {
            while (*src != 0 && *src != '\n') {
                src++;
            }
        }
        // identifier */
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || token == '_') {
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') 
                    || (*src >= '0' && *src <= '9') || *src == '_') {
                hash = hash * 143 + *src;
                src++;
            }

            // look for existing identifier */
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash 
                        && !memcmp((char*)current_id[Name], last_pos, src - last_pos)) { 
                    // short circuit if hash differs */
                    // found */
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }

            // new identifier */
            current_id[Name] = (int) last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        // num */
        else if (token >= '0' && token <= '9') {
            token_val = token - '0';
            while (*src >= '0' && *src <= '9') {
                token_val = token_val * 10 + *src - '0';
                src++;
            }
            token = Num;
            return;
        }
        // string and character */
        // Note : No error will be reported when the character length is greater than 1 */
        else if (token == '"' || token == '\'') {
            last_pos = data;
            while (*src != 0 && *src != token) {
                token_val = *src++;
                if (token_val == '\\') { // escape character */
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                }

                if (token == '"') {
                    *data++ = token_val;
                }
            }
            src++;

            if (token == '"') { 
                // '"' itself is the token of string */
                // maybe we should define a new token type String, and let token = String? */
                token_val = (int)last_pos;
            } else {
                token = Num;
            }
            return;
        }
        // coment and divide operator
        else if (token == '/') {
            if (*src++ == '/') {
                while (*src != '\n' && *src != 0) {
                    src++;
                }
                return;
            }
            else {
                token = Div;
                return;
            }
        }
        // '==' and '='
        else if (token == '=') {
            if (*src == '=') { src ++; token = Eq; } 
            else { token = Assign; }
            return;
        }
        // '+' and '++'
        else if (token == '+') {
            if (*src == '+') { src ++; token = Inc; } 
            else { token = Add; }
            return;
        }
        // '-' and '--'
        else if (token == '-') {
            if (*src == '-') { src ++; token = Dec;} 
            else { token = Sub; }
            return;
        }
        // '!='
        else if (token == '!') {
            if (*src == '=') { src++; token = Ne;}
            return;
        }
        // '<=', '<<' and '<'
        else if (token == '<') {
            if (*src == '=') { src ++; token = Le; } 
            else if (*src == '<') { src ++; token = Shl; } 
            else { token = Lt; }
            return;
        }
        // '>=', '>>' and '>'
        else if (token == '>') {
            if (*src == '=') { src ++; token = Ge; } 
            else if (*src == '>') { src ++; token = Shr; } 
            else { token = Gt; }
            return;
        }
        // '|' and '||'
        else if (token == '|') {
            if (*src == '|') { src ++; token = Lor; } 
            else { token = Or; }
            return;
        }
        // '&' and '&&'
        else if (token == '&') {
            if (*src == '&') { src ++; token = Lan; } 
            else { token = And; }
            return;
        }
        else if (token == '^') { token = Xor; return; }
        else if (token == '%') { token = Mod; return; }
        else if (token == '*') { token = Mul; return; }
        else if (token == '[') { token = Brak; return; }
        else if (token == '?') { token = Cond; return; }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' 
                || token == ')' || token == ']' || token == ',' || token == ':') {
            // directly return the character as token;
            return;
        }

        else {
            printf("Line %d: Unknown token %c(%d)!\n", line, token, token);
            exit(-1);
        }
    }
}

//***************************************************************
// @brief   to evaluate the expression
//***************************************************************/
void expression(int level) {

}

//********************** Context Free Grammar ***************************
// program ::= {global_declaration}+
// global_declaration ::= enum_decl | variable_decl | function_decl
// enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'
// variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
// function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'
// parameter_decl ::= type {'*'} id {',' type {'*'} id}
// body_decl ::= {variable_decl}, {statement}
// statement ::= non_empty_statement | empty_statement
// non_empty_statement ::= if_statement | while_statement | '{' statement '}'
//                      | 'return' expression | expression ';'
// if_statement ::= 'if' '(' expression ')' statement ['else' non_empty_statement]
// while_statement ::= 'while' '(' expression ')' non_empty_statement
//***********************************************************************/

//***************************************************************
// @brief   the entrance of parser
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
// @brief   to run the virtual machine
// @return  non-zero value if fails; otherwise 0
// @note    UNSAFE! when the stack of virtual machine grows bigger than poolsize, the
//          program will crash
//***************************************************************/
int eval() {
    int op;
    int *tmp;
    while (1) {
        op = *pc++;
        //    Instruction   |                        Description                            *
        // IMM <num>        | load immediate number to ax                                   *
        if (op == IMM)          { ax = *pc++; }
        // LC               | load character to ax, address in ax                           *
        else if (op == LC)      { ax = *(char*) ax; }
        // LI               | load interger to ax, address in ax                            *
        else if (op == LI)      { ax = *(int*)  ax; }
        // SC               | save character, value in ax, address in the top of stack      *
        else if (op == SC)      { ax = *(char*) sp = ax; }  // why not simply *(char*) sp = ax ? */
        // SI               | save interger, value in ax, address in the top of stack       *
        else if (op == SI)      { *(int*)  sp = ax; }
        // PUSH             | push the value of ax onto the stack                           *
        else if (op == PUSH)    { *--sp = ax; }
        // JMP <adr>        | jump to the address                                           *
        else if (op == JMP)     { pc = (int*) *pc; }
        // JEZ <adr>        | jump to the address if ax equals to zero                      *
        else if (op == JEZ)     { pc = ax == 0 ? (int*) *pc : pc + 1; }
        // JNZ <adr>        | jump to the address if ax does not equal to zero              *
        else if (op == JNZ)     { pc = ax != 0 ? (int*) *pc : pc + 1; }
        // CALL <adr>       | call subroutine, address in <adr>                             *
        else if (op == CALL)    { *--sp = (int) (pc + 1); pc = (int*) *pc; }
        // ENT <size>       | create a new stack frame                                      *
        else if (op == ENT)     { *--sp = (int) bp; bp = sp; sp = sp - *pc++; } // -= is not supported */
        // ADJ <size>       | remove arguments from stack frame                             *
        else if (op == ADJ)     { sp = sp + *pc++; }
        // LEV              | restore pc, stack frame, then return                          *
        else if (op == LEV)     { sp = bp; bp = (int*) *sp++; pc = (int*) *sp++; }
        // LEA <offset>     | load argument                                                 *
        else if (op == LEA)     { ax = (int) (bp + *pc++); }

        // <operation>      | operands in ax and the top of stack, save result in ax        *
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

        // <function>       | build-in function                                             *
        else if (op == EXIT)    { printf("exit(%d)\n", *sp); return *sp;}
        else if (op == OPEN)    { ax = open((char*) sp[1], sp[0]); }
        else if (op == CLOS)    { ax = close(*sp);}
        else if (op == READ)    { ax = read(sp[2], (char*) sp[1], *sp); }
        else if (op == PRTF)    { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], 
                                    tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
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
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // choosed randomly */
    line = 1;

    // alloc memory for virtual machine */
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

    if (!(symbols = (int*) malloc(poolsize))) {
        printf("Failed to alloc memory for symbol table!\n");
        return -1;
    }
    memset(symbols, 0, poolsize);

    // add keywords to symbol table */
    src = "char else enum if int return sizeof while "  // in the same order as token enum */
          "open read close printf malloc memset memcmp exit void main";

    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // add library functions to symbol table */
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next(); current_id[Token] = Char;   // void type            */
    next(); idmain = current_id;        // keep track of main   */
    
    // initialize the virtual machine */
    // todo: pc = *main* */
    bp = sp = stack + poolsize;

    // open(), read() and close() are Linux functions                          */
    // We use them since they work better with virtual machine                 */
    // and we do not use fprintf(stderr, ...),                                 */
    // instead, we use printf() since it works better with virtual machine     */
    if ((fd = open(*argv, 0)) < 0) {    // #define F_OK 0 */
        printf("Failed to open the file %s!\n", *argv);
        return -1;
    }

    if (!(src = src_old = (char*) malloc(poolsize))) {
        printf("Failed to alloc memory for source code buffer!\n");
        return -1;
    }

    if((i = read(fd, src, poolsize - 1)) <= 0) {
        printf("Failed when executing read(), read() returned %d!\n", i);
        return -1;
    }

    src[i] = 0; // '\0' */
    close(fd);

    program();

    return eval();
}