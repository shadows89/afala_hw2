echo Copying...

cp kernel/kernel/sched.c /usr/src/linux-2.4.18-14custom/kernel
echo -n "1 "

cp kernel/kernel/fork.c /usr/src/linux-2.4.18-14custom/kernel
echo -n "2 "

cp kernel/kernel/exit.c /usr/src/linux-2.4.18-14custom/kernel
echo -n "3 "

cp kernel/kernel/hw2_syscalls.c /usr/src/linux-2.4.18-14custom/kernel
echo -n "4 "

cp kernel/kernel/Makefile /usr/src/linux-2.4.18-14custom/kernel
echo -n "5 "

cp kernel/include/linux/sched.h /usr/src/linux-2.4.18-14custom/include/linux
echo -n "6 "

cp kernel/include/asm-i386/unistd.h /usr/src/linux-2.4.18-14custom/include/asm-i386
echo -n "7 "

cp kernel/arch/i386/kernel/entry.S /usr/src/linux-2.4.18-14custom/arch/i386/kernel
echo "8 "
echo done