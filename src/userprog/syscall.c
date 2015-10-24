#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "process.h"

static void syscall_handler (struct intr_frame *);

void is_valid_pointer(void *ptr);
void *convert_userp(void *ptr);
void get_argument(struct intr_frame *f, int *argument, int n);
struct file *get_file(int fd);

struct custom_file
{
	struct list_elem file_elem;
	int fd;
	struct file *f;
};

struct custom_file *get_custom_file(int fd);
struct custom_file *get_custom_file(int fd)
{
	struct list *l = &thread_current()->file_list;
	struct list_elem *e;

	for(e = list_begin(l); e != list_end(l); e = list_next(e))
	{
		struct custom_file *cf = list_entry(e, struct custom_file, file_elem);
		if(fd == cf->fd)
			return cf;
	}
	return NULL;
}

struct file *get_file(int fd)
{
	struct custom_file *cf = get_custom_file(fd);
	if(cf == NULL)
		return NULL;
	return cf->f;
}

void
syscall_init (void) 
{
  lock_init(&fl);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
	int nsyscall, ret = 0, args[10];
	int *esp = (int *)f->esp;

	is_valid_pointer(esp);

	nsyscall = *(esp++);
	//printf("syscall [%d]\n", nsyscall);	
	switch(nsyscall)
	{
    	case SYS_HALT:                   /* Halt the operating system. */
			halt();
			break;
    	case SYS_EXIT:                   /* Terminate this process. */
			get_argument(f, args, 1);
			exit(args[0]);
			break;
    	case SYS_EXEC:                   /* Start another process. */
			get_argument(f, args, 1);
			args[0] = (int)convert_userp((void *)args[0]);
			ret = exec((char *)args[0]);
			break;
    	case SYS_WAIT:                   /* Wait for a child process to die. */
			get_argument(f, args, 1);
			ret = wait(args[0]);
			break;
    	case SYS_CREATE:                 /* Create a file. */
			get_argument(f, args, 2);
			args[0] = (int)convert_userp((void *)args[0]);
			ret = create((char *)args[0], (unsigned)args[1]);
			break;
    	case SYS_REMOVE:                 /* Delete a file. */
			get_argument(f, args, 1);
			args[0] = (int)convert_userp((void *)args[0]);
			ret = remove((char *)args[0]);
			break;
    	case SYS_OPEN:                   /* Open a file. */
			get_argument(f, args, 1);
			args[0] = (int)convert_userp((void *)args[0]);
			ret = open((char *)args[0]);
			break;
    	case SYS_FILESIZE:               /* Obtain a file's size. */
			get_argument(f, args, 1);
			ret = filesize(args[0]);
			break;
    	case SYS_READ:                   /* Read from a file. */
			get_argument(f, args, 3);
			args[1] = (int)convert_userp((void *)args[1]);
			ret = read(args[0], (void *)args[1], (unsigned)args[2]);
			break;
    	case SYS_WRITE:                  /* Write to a file. */
    		get_argument(f, args, 3);
			args[1] = (int)convert_userp((void *)args[1]);
			ret = write(args[0], (const void *)args[1], (unsigned)args[2]);
			break;
		case SYS_SEEK:                   /* Change position in a file. */
			get_argument(f, args, 2);
			seek(args[0], (unsigned)args[1]);
			break;
    	case SYS_TELL:                   /* Report current position in a file. */
			get_argument(f, args, 1);
			ret = tell(args[0]);
			break;
    	case SYS_CLOSE:                  /* Close a file. */
			get_argument(f, args, 1);
			close(args[0]);
			break;
		default:
			thread_exit();	
	}

	f->eax = ret;
}

void is_valid_pointer(void *ptr)
{
	if(ptr == NULL)
		exit(-1);

	if( ! is_user_vaddr(ptr))
		exit(-1);

	if(ptr > PHYS_BASE)
		exit(-1);

	if(ptr < 0x08048000)
		exit(-1);
	// 매핑된 페이지가 없는 경우
	void *page = pagedir_get_page(thread_current()->pagedir, ptr);
	if(page == NULL)
		exit(-1);
}

void *convert_userp(void *ptr)
{
	is_valid_pointer(ptr);
	return pagedir_get_page(thread_current()->pagedir, ptr);
}

void get_argument(struct intr_frame *f, int *argument, int n)
{
	int i;
	for(i=0; i<n; i++)
	{
		int *p = (int *)f->esp + 1 + i;
		is_valid_pointer((void *)p);
		*(argument+i) = *p;
	}
}

void
halt (void)
{
	shutdown_power_off();
}

void
exit (int status)
{
	struct thread *t = thread_current();
	t->myself->exit_status = status;
	printf("%s: exit(%d)\n", t->name, status);
	thread_exit();
}


pid_t exec (const char *file) // with parameters
{
	pid_t ret = process_execute(file);
	if(ret == -1)	// openTest 실패 시 바로 리턴
		return ret;

	struct child *c = get_child(ret);	
	// memory 때문에 exec 파일 로딩에 실패할 때
	// filesys_open (openTest) 까지는 되지만, process.c의 load()가 실패함
	// 따라서 이것까지 기다렸다가 exec의 결과를 리턴할 수밖에 없음
	while(c->load_status == 0)	 
	{
		thread_yield();
	}

	if(c->load_status == -1)
	{
		return -1;
	}

	return ret;
}

bool openTest(const char *file)
{
	lock_acquire(&fl);
	struct file *f = filesys_open(file);
	if(f == NULL)
	{
		lock_release(&fl);
		return false;
	}
	file_close(f);
	lock_release(&fl);
	return true;
}

int 
wait (pid_t pid)
{
	return process_wait(pid);
}

bool
create (const char *filename, unsigned initial_size)
{
	if(filename == NULL)
		return false;

	lock_acquire(&fl);
	bool ret = filesys_create(filename, initial_size);
	lock_release(&fl);
	return ret;
}

bool
remove (const char *filename)
{
    if(filename == NULL)
		return false;
	lock_acquire(&fl);
	bool ret = filesys_remove(filename);
	lock_release(&fl);
	return ret;
}

int
open (const char *filename)
{
	if(filename == NULL)
		return -1;

	lock_acquire(&fl);
	struct file *f = filesys_open(filename);
	if(f == NULL)
	{
		lock_release(&fl);
		return -1;
	}

	//file_deny_write(f);
	// -> start_process로 옮김 

	int new_fd = thread_current()->current_max_fd + 1;
	thread_current()->current_max_fd += 1;

	struct custom_file *cf = malloc(sizeof(struct custom_file));
	cf->f = f;
	cf->fd = new_fd;
	list_push_back(&thread_current()->file_list, &cf->file_elem);
	
	lock_release(&fl);
	return new_fd;
}

int
filesize (int fd)
{
	struct custom_file *cf = get_custom_file(fd);
	if(cf != NULL)
	{
		lock_acquire(&fl);
		off_t length = file_length(cf->f);
		lock_release(&fl);
		return length;
	}
	return 0;
}

int
read (int fd, void *buffer, unsigned length)
{
	if(fd == 0) // stdin
	{
		unsigned i;
		char *buf = (char *)buffer;
		for(i=0; i<length; i++)
			buf[i] = input_getc();
		return length;
	}
	else
	{
		struct file *f = get_file(fd);
		if(f == NULL)
			return -1;
		
		lock_acquire(&fl);
		int real_length = file_read(f, buffer, length);
		lock_release(&fl);
		return real_length;
	}
}

int
write (int fd, const void *buffer, unsigned length)
{
	if(fd == 1)	// stdout
	{
		unsigned i;
		// 100 chars per putbuf
		for(i=0; i<=length/100; i++)
		{
			int _len = (length - i*100) >= 100? 100 : (length - i*100);

			putbuf(&((const char *)buffer)[100*i], _len);
		}
		return length;
	}
	else
	{
		struct file *f = get_file(fd);
		if(f == NULL)
			return -1;

		lock_acquire(&fl);
		int real_length = file_write(f, buffer, length);
		lock_release(&fl);
		return real_length;
	}
}

void
seek (int fd, unsigned position)
{
	struct custom_file *cf = get_custom_file(fd);
	if(cf != NULL)
	{
		lock_acquire(&fl);
		file_seek(cf->f, position);
		lock_release(&fl);
	}
}

unsigned
tell (int fd)
{
	struct custom_file *cf = get_custom_file(fd);
	if(cf != NULL)
	{
		lock_acquire(&fl);
		off_t t = file_tell(cf->f);
		lock_release(&fl);
		return t;
	}
	return 0;
}

void close_internal(struct custom_file *cf);
void close_internal(struct custom_file *cf)
{
	//file_allow_write(cf->f);	file_close 내에서 이미 호출함 
	
	lock_acquire(&fl);
	file_close(cf->f);
	lock_release(&fl);
	list_remove(&cf->file_elem);
	free(cf);
}

void close (int fd)
{
	struct custom_file *cf = get_custom_file(fd);
	if(cf != NULL)
		close_internal(cf);
}

void close_all()
{
	struct thread *t = thread_current();
	struct list_elem *e = list_begin(&t->file_list);

	// 이 프로세스에서 오픈한 파일들 
	while(e != list_end(&t->file_list))
	{
		struct list_elem *next = list_next(e); // close_internal에서 free하기 때문에 저장해놓아야 함 
		struct custom_file *cf = list_entry(e, struct custom_file, file_elem);
		close_internal(cf);
		e = next;	
	}

	// 유저 프로그램 실행 프로세스였다면, 그 프로그램 파일
	if(t->executing_file != NULL)
	{
		// 이 안에서 allow_write 하게 됨
		lock_acquire(&fl);
		file_close(t->executing_file);
		lock_release(&fl);
		t->executing_file = NULL;
	}
}
