#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}

void
halt (void)
{
}

void
exit (int status)
{
}

pid_t exec (const char *file)
{
}

int 
wait (pid_t pid)
{
}

bool
create (const char *file, unsigned initial_size)
{
	if(file != NULL)
		return filesys_create(file, initial_size);
	else
		return false;
}

bool
remove (const char *file)
{
    if(file != NULL)
        return filesys_remove(file);
    else
        return false;
}

int
open (const char *file)
{

}

int
filesize (int fd)
{
}

int
read (int fd, void *buffer, unsigned length)
{
}

int
write (int fd, const void *buffer, unsigned length)
{
}

void
seek (int fd, unsigned position)
{
}

unsigned
tell (int fd)
{
}

void
close (int fd)
{
}
