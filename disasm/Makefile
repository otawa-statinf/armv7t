
CFLAGS=-I../include -I../src -g -O3
LIBADD += $(shell bash ../src/arm-config --libs)
EXEC=arm-disasm$(EXE_SUFFIX)

all: fix $(EXEC)

fix: 
	cp arm-disasm.c.save arm-disasm.c

$(EXEC): arm-disasm.o ../src/libarm.a
	$(CC) $(CFLAGS) -o $@ $< $(LIBADD)

clean:
	rm -rf arm-disasm.o

distclean: clean
	rm -rf $(EXEC)
