SC_OBJ := shellcode
TST_OBJ :=stack_test
TST_SC := get_shellcode

all: $(TST_OBJ) $(TST_SC)

$(TST_OBJ): stack_test.c
	gcc -g -z execstack $^ -fno-stack-protector -o $@
	objdump -axd $@ > $@.s

$(SC_OBJ): shellcode.c
	gcc -static  $^ -o $@
	objdump -saxd $@ > $@.s

$(TST_SC): get_shellcode.c | $(SC_OBJ)
	gcc -g $^ -o $@
	objdump -saxd $@ > $@.s

clean:
	rm -rf $(TST_OBJ) $(SC_OBJ) $(TST_SC) *.s

.PHONY: clean all

# echo 0 > /proc/sys/kernel/randomize_va_space 
