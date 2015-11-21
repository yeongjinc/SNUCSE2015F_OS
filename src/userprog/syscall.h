#include <stdbool.h>
#include "threads/synch.h"
#include "lib/user/syscall.h"

#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

/* Process identifier. */
typedef int pid_t;
#define PID_ERROR ((pid_t) -1)

struct lock fl;

void syscall_init (void);
void halt (void);
void exit (int);
pid_t exec (const char *);
bool openTest (const char *);
int wait (pid_t);
bool create (const char *, unsigned);
bool remove (const char *);
int open (const char *);
int filesize (int);
int read (int, void *, unsigned);
int write (int, const void *, unsigned);
void seek (int, unsigned);
unsigned tell (int);
void close (int);
void close_all (void);

bool chdir(const char *);
bool mkdir(const char *);
bool readdir(int, char[READDIR_MAX_LEN]);
bool isdir(int);
int inumber(int);

#endif /* userprog/syscall.h */
