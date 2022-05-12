#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "lib/kernel/console.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);
void exit(int status);
tid_t exec(const char* file);
int wait(tid_t pid);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read(int fd,void* buffer, unsigned size);
int write(int fd,const void* buffer,unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
void valid_check(const void* addr);

// struct lock lock1; 
struct semaphore rd;
struct semaphore wt;
int count;

void
syscall_init (void) 
{
  // lock_init(&lock1);
  count=0;
  sema_init(&rd,1);
  sema_init(&wt,1);
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
		valid_check((f->esp)+4);
                valid_check((f->esp)+8);
		(f->eax)=create((const char*)*(uint32_t*)((f->esp)+4),*(unsigned int*)((f->esp)+8));
		break;
	case SYS_REMOVE:
		valid_check((f->esp)+4);
		(f->eax)=remove((const char*)*(uint32_t*)((f->esp)+4));
		break;
	case SYS_OPEN:
		valid_check((f->esp)+4);
                (f->eax)=open((const char*)*(uint32_t*)((f->esp)+4));
		break;
	case SYS_FILESIZE:
		valid_check((f->esp)+4);
		(f->eax)=filesize(*(int*)((f->esp)+4));
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
		valid_check((f->esp)+4);
                valid_check((f->esp)+8);
		seek(*(int*)((f->esp)+4),*(unsigned int*)((f->esp)+8));
		break;
	case SYS_TELL:
		 valid_check((f->esp)+4);
                (f->eax)=tell(*(int*)((f->esp)+4));
		break;
	case SYS_CLOSE:
		 valid_check((f->esp)+4);
                close(*(int*)((f->esp)+4));
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
	// close file
	for(int i=3;i<128;i++)
	{
		struct thread*temp=thread_current();
		if(temp->fd[i]==NULL)
			continue;
		else
			close(i);
	}
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

bool create (const char *file, unsigned initial_size)
{
	valid_check(file);
	if(file==NULL)
		exit(-1);
	return filesys_create(file,initial_size);
}

bool remove (const char *file)
{
	valid_check(file);
	if(file==NULL)
                exit(-1);
	return filesys_remove(file);
}

int open (const char *file)
{
	valid_check(file);
	if(file==NULL)
                exit(-1);
	//lock_acquire(&lock1);
	sema_down(&rd);
	count+=2;
	if(count==2)
		sema_down(&wt);
	sema_up(&rd);
	int start=3,result=-1;
	struct file*fp=filesys_open(file);
	if(fp==NULL)
	{
		result=-1;;
	}
	else{
	while(1){
	struct thread*temp=thread_current();
	if(temp->fd[start]==NULL)
	{
		if(!strcmp(temp->name,file))
			file_deny_write(fp);
		temp->fd[start]=fp;
		result=start;
		break;
	}
	start++;
	if(start==128)
		break;
	}
	}
	sema_down(&rd);
	count-=2;
	if(count==0)
		sema_up(&wt);
	sema_up(&rd);
	//lock_release(&lock1);
	return result;
}

int filesize (int fd)
{
	struct thread*temp=thread_current();
	if(temp->fd[fd]==NULL)
		exit(-1);
	return file_length(temp->fd[fd]);
}

int read(int fd,void* buffer, unsigned size)
{
	valid_check(buffer);
	// lock_acquire(&lock1);
	unsigned int i;
	int result=-1,flag=0;
	sema_down(&rd);
	count+=2;
	if(count==2)
		sema_down(&wt);
	sema_up(&rd);
	if(fd>2)
	{
		struct thread*temp=thread_current();
		if(temp->fd[fd]==NULL){
			//lock_release(&lock1);
               		exit(-1);}
		flag=1;
		// return file_read(thread_current()->fd[fd],buffer,size);
	}
	else if(fd==1 || fd==2)
		flag=0;
	else{
	for(i=0;i<size;i++)
	{
		((char*)buffer)[i]=input_getc();
		if(((char*)buffer)[i]==0)
			break;	
	}
	if(i==size)
	{
		result=i;
	}
	}
	if(flag==1)
		result=file_read(thread_current()->fd[fd],buffer,size);
	sema_down(&rd);
	count-=2;
	if(count==0)
		sema_up(&wt);
	sema_up(&rd);
	// lock_release(&lock1);
	return result;
}

int write(int fd,const void* buffer,unsigned size)
{
	valid_check(buffer);
	// lock_acquire(&lock1);
	int flag=0;
	sema_down(&wt);
	int result=size;	
	if(fd>2)
	{
		struct thread*temp=thread_current();
		if(temp->fd[fd]==NULL){
			// lock_release(&lock1);
                        exit(-1);}
		flag=1;
		// return file_write(thread_current()->fd[fd],buffer,size);
	}
	else if(fd==2 || fd==0)
		flag=0;
	else
	{
		putbuf(buffer,size);
	}
	if(flag==1)
		result=file_write(thread_current()->fd[fd],buffer,size);
	//lock_release(&lock1);
	sema_up(&wt);
	return result;
}

void seek (int fd, unsigned position)
{
	struct thread*temp=thread_current();
	if(temp->fd[fd]==NULL)
		exit(-1);
	file_seek(temp->fd[fd],position);
}

unsigned tell (int fd)
{
	struct thread*temp=thread_current();
	if(temp->fd[fd]==NULL)
		exit(-1);
	return file_tell(temp->fd[fd]);
}

void close (int fd)
{
	struct thread*temp=thread_current();
	if(temp->fd[fd]==NULL)
		exit(-1);
	struct file*temp2=temp->fd[fd];
	temp->fd[fd]=NULL;
	file_close(temp2);
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


