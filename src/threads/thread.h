#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING,       /* About to be destroyed. */
	//THREAD_ZOMBIE,		/* For User Program, Parent waiting */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */
	
	// prj1 replacing busy-waiting
	int wait_flag;						/* prj1 : 1 when sleeping */
	int wait_start;						/* prj1 : wait start tick */
	int wait_length;					/* prj1 : wait length tick */

	// prj1 priority donation
	int original_priority;				/* prj1 : priority before donation */
	struct lock *waiting_lock;			/* prj1 : lock that this thread is waiting */
	struct list donator;				/* prj1 : list of thread that donated this priority */
	struct list_elem donator_elem;		/* prj1 : list elem of donator */

	// prj2 user program
	struct list file_list;				/* prj2 : files opened in this process */
	int current_max_fd;					/* prj2 : increment when new file opened */

	struct list child_list;				/* prj2 : child process list(struct child) - To store child process */
	struct child *myself;				/* prj2 : this goes to parent's child_list */
	
	struct file *executing_file;		/* prj2 : the file that this process are executing
										   		  must not be written when executing */
	// prj4 file system
	struct dir *directory; 				/* prj4 : current working directory */


#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

struct child
{
	struct list_elem child_elem;
	tid_t tid;
	struct thread *self;			/* this process */
	struct thread *parent;			/* parent process */
	int exit_status;				/* exit status, -1 when error */
	bool parent_is_waiting;			/* if this flag is set, wake up parent */
	bool is_zombie;					/* child process is dead - true when terminated */
	int load_status;				/* filesys_open test is not enough,
									   so we will not return exec until this is confirmed 
												  0 is loading
												  1 is success
												  -1 is fail*/
};

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
struct child *get_child (tid_t tid);	// prj2 New function
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);


/////////////////////////
//prj1 New function
/////////////////////////
//
void thread_sleep(int64_t ticks);
bool sleep_time_less(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);
bool priority_more(const struct list_elem *a, const struct list_elem *b, void *aus UNUSED);

void thread_check_ready(void);
void donate_priority(struct thread *t);
void clear_waiting(struct thread *t, struct lock *waiting_lock);
void restore_priority(struct thread *t);

////////////////////////

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

#endif /* threads/thread.h */
