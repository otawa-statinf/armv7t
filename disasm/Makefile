
CFLAGS=-I../include -I../src -g -O3
LIBADD += $(shell bash ../src/arm-config --libs)
EXEC=arm-disasm$(EXE_SUFFIX)

all: $(EXEC)

$(EXEC): arm-disasm.o ../src/libarm.a
	$(CC) $(CFLAGS) -o $@ $< $(LIBADD)

clean:
	rm -rf arm-disasm.o

distclean: clean
	rm -rf $(EXEC)
