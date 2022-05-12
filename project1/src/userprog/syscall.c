#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "lib/kernel/console.h"

static void syscall_handler (struct intr_frame *);
void exit(int status);
tid_t exec(const char* file);
int wait(tid_t pid);
int read(int fd,void* buffer, unsigned size);
int write(int fd,const void* buffer,unsigned size);
void valid_check(const void* addr);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //f->esp는 void 포인터임
  switch(*(uint32_t*)(f->esp)){
	case SYS_HALT:
		// printf("halt called\n");
		shutdown_power_off();
		break;
	case SYS_EXIT:
		// printf("exit\n");
		valid_check((f->esp)+4);
		exit(*(int*)((f->esp)+4));
		break;
	case SYS_EXEC:
		// printf("exec\n");
		valid_check((f->esp)+4);
		(f->eax)=exec((const char*)*(uint32_t*)((f->esp)+4));
		break;
	case SYS_WAIT:
		// printf("wait\n");
		valid_check((f->esp)+4);
		(f->eax)=wait((tid_t)*(uint32_t*)((f->esp)+4));
		break;
	case SYS_CREATE:
		break;
	case SYS_REMOVE:
		break;
	case SYS_OPEN:
		break;
	case SYS_FILESIZE:
		break;
	case SYS_READ:
		// printf("read\n");
		valid_check((f->esp)+4);
		valid_check((f->esp)+8);
		valid_check((f->esp)+12);
		(f->eax)=read(*(int*)((f->esp)+4),(void*)(*(uint32_t*)((f->esp)+8)),*(unsigned int*)((f->esp)+12));
		break;
	case SYS_WRITE:
		// printf("write\n");
		valid_check((f->esp)+4);
                valid_check((f->esp)+8);
                valid_check((f->esp)+12);
                (f->eax)=write(*(int*)((f->esp)+4),(void*)(*(uint32_t*)((f->esp)+8)),*(unsigned int*)((f->esp)+12));
		break;
	case SYS_SEEK:
		break;
	case SYS_TELL:
		break;
	case SYS_CLOSE:
		break;
	case SYS_FIBONACCI:
		valid_check((f->esp)+4);
		(f->eax)=fibonacci(*(int*)((f->esp)+4));
		break;
	case SYS_MAX_OF_FOUR_INT:
		valid_check((f->esp)+4);
                valid_check((f->esp)+8);
                valid_check((f->esp)+12);
		valid_check((f->esp)+16);
		(f->eax)=max_of_four_int(*(int*)((f->esp)+4),*(int*)((f->esp)+8),*(int*)((f->esp)+12),*(int*)((f->esp)+16));
		break;	
  }
		
	
}


void exit(int status)
{
	thread_current()->exit_status=status;
	printf("%s: exit(%d)\n",thread_name(),status);
	thread_exit();
}

tid_t exec(const char* file)
{
	tid_t sol=process_execute(file);
	return sol;
}

int wait(tid_t pid)
{
	return process_wait(pid);
}

int read(int fd,void* buffer, unsigned size)
{
	unsigned int i;
	if(fd!=0)
		return -1;
	for(i=0;i<size;i++)
	{
		((char*)buffer)[i]=input_getc();
		if(((char*)buffer)[i]==0)
			break;	
	}
	if(i==size)
		return i;
	else
		return -1;
}

int write(int fd,const void* buffer,unsigned size)
{
	if(fd!=1)
		return -1;
	putbuf(buffer,size);
	return size;
}

int fibonacci(int a)
{
	int p3,count=2;
	int p1=1;
	int p2=1;
	if(a==1 || a==2)
		return 1;
	else
	{
		while(count<a){	
		count++;
		p3=p1+p2;
		p1=p2;
		p2=p3;	
		}
		return p3;
	}
}

int max_of_four_int(int a,int b,int c,int d)
{
	int max=a;
	if(b>max)
		max=b;
	if(c>max)
		max=c;
	if(d>max)
		max=d;
	return max;
}

void valid_check(const void* addr)
{
	if(is_user_vaddr(addr)==false)
		exit(-1);
}


