CC=arm-none-eabi-gcc
ifdef OTAWA_HOME
	ODISASM=$(OTAWA_HOME)/bin/odisasm
else
	ODISASM=odisasm
endif
CPU=cortex-m4
ASFLAGS=-x assembler-with-cpp 
CFLAGS=-nostartfiles -mthumb -mcpu=$(CPU) -g
BIN=test

all: $(BIN) $(BIN).odis $(BIN).cmp 

$(BIN):	$(BIN).s
	$(CC) $(CFLAGS) $(ASFLAGS) $< -o $@

.PHONY: $(BIN).odis
$(BIN).odis: $(BIN)
	$(ODISASM) $(@:.odis=) > $@

%.cmp: %.odis %.odis.valid
	diff $(@:%.cmp=%.odis) $(@:%.cmp=%.odis.valid)

.PHONY: clean
clean:
	rm -f $(BIN) $(BIN).odis $(BIN).dis
