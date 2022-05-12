#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
int fibonacci(int a);
int max_of_four_int(int a,int b,int c,int d);
// 나머지것들은 .c 위쪽에 구현
#endif /* userprog/syscall.h */
