#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

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
	shutdown_power_off();
}

void
exit (int status UNUSED)
{
}


pid_t exec (const char *file)
{
	return 0;
}

int 
wait (pid_t pid)
{
	return 0;
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
	return 0;
}

int
filesize (int fd)
{
	return 0;
}

int
read (int fd, void *buffer, unsigned length)
{
	return 0;
}

int
write (int fd, const void *buffer, unsigned length)
{
	return 0;
}

void
seek (int fd, unsigned position)
{
}

unsigned
tell (int fd)
{
	return 0;
}

void
close (int fd)
{
}
