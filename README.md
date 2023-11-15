# w1ldc : C语言解释器

---

## 教程来源

最初启发者: https://github.com/rswier/c4

二次教程: https://github.com/lotabout/write-a-C-interpreter

中文版博客: https://lotabout.me/2015/write-a-C-interpreter-0/

## BUILD

> `make`

## USAGE

> `./w1ldc [source file] [args]`

`source file` : 源文件.

`args` : 命令行参数

## SAMPLE

> `./w1ldc test/fibonacci.c`

> `./w1ldc w1ld.c test/fibonacci.c`

> `./w1ldc w1ld.c w1ld.c test/fibonacci.c`

或使用

> `make test`

以依次执行上述三条命令．

## 说明

按照教程, 支持的语言应该是C的子集. 不清楚排除了哪些语法.