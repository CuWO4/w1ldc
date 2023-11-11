TARGET := w1ld.exe
EXTERN := c
COMPILER := gcc

COMPILE_OPTION := -Wall -O2
# to generate dependent files #
COMPILE_OPTION_DES := -MMD -MP 

# store .o and .d files #
TMPDIR := tmp
# store the target file #
DEBUGDIR := .

# sources, objects and dependencies #
SRCS := $(wildcard *.$(EXTERN))
OBJS := $(patsubst %.$(EXTERN), $(TMPDIR)/%.o, $(SRCS))
DEPS := $(patsubst %.$(EXTERN), $(TMPDIR)/%.d, $(SRCS))

# link #
$(DEBUGDIR)/$(TARGET) : $(OBJS) | $(DEBUGDIR)
	@ echo linking...
	$(COMPILER) $(OBJS) -o $(DEBUGDIR)/$(TARGET)
	@ echo completed!

# compile #
$(TMPDIR)/%.o : %.$(EXTERN) | $(TMPDIR)
	@ echo compiling $<...
	$(COMPILER) $< -o $@ -c $(COMPILE_OPTION) $(COMPILE_OPTION_DES)

# create TMPDIR when it does not exist #
$(TMPDIR) :
	@ mkdir $(patsubst ./%, %, $(TMPDIR))

$(DEBUGDIR) :
	@ mkdir $(DEBUGDIR)

.gitignore :
	@ cd . > .gitignore
	@ echo $(TMPDIR) >> .gitignore
	@ echo. >> .gitignore
	@ echo *.exe >> .gitignore

README.md :
	@ cd . > README.md

main.$(EXTERN) :
	@ cd . > main.$(EXTERN)
	@ echo #include ^<stdio.h^> >> main.$(EXTERN)
	@ echo. >> main.$(EXTERN)
	@ echo int main(int argc, char **argv) { >> main.$(EXTERN)
	@ echo. >> main.$(EXTERN)
	@ echo     return 0; >> main.$(EXTERN)
	@ echo } >> main.$(EXTERN)

# files dependecies #
-include $(DEPS)

# clean command #
.PHONY : clean
clean :
	@ echo try to clean...
	rm -r $(DEBUGDIR)/$(TARGET) $(OBJS) $(DEPS)
	@ echo completed!

# initialize command #
.PHONY : init
init : | $(TMPDIR) $(DEBUGDIR) .gitignore README.md main.$(EXTERN)
	@ echo completed!

TESTFILE := test/fibonacci.c

# test command #
# notice that test command will fail if there is more than one source file #
.PHONY : test
test : $(DEBUGDIR)/$(TARGET)
	$(DEBUGDIR)/$(TARGET) $(TESTFILE)
	$(DEBUGDIR)/$(TARGET) $(SRCS) $(TESTFILE)
	$(DEBUGDIR)/$(TARGET) $(SRCS) $(SRCS) $(TESTFILE)