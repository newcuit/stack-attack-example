SC_OBJ := shellcode
TST_OBJ :=stack_test
TST_SC := get_shellcode

CC ?= gcc
OBJDUMP ?= objdump
CFLAGS ?= -g -z execstack -fno-stack-protector

all: $(TST_OBJ) $(TST_SC)

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

$(TST_OBJ): $(TST_OBJ).o
	$(CC) $(CFLAGS) $^ -o $@
	$(OBJDUMP) -axd $@ > $@.s

$(SC_OBJ): $(SC_OBJ).c
	$(CC) -static  $^ -o $@
	$(OBJDUMP) -saxd $@ > $@.s

$(TST_SC): $(TST_SC).c | $(SC_OBJ)
	$(CC) $^ -o $@
	$(OBJDUMP) -saxd $@ > $@.s

clean:
	rm -rf $(TST_OBJ) $(SC_OBJ) $(TST_SC) *.s *.o

.PHONY: clean all

# echo 0 > /proc/sys/kernel/randomize_va_space 
