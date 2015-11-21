#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/block.h"
#include "lib/kernel/list.h"

// Only Single Indirect
// 124 * 124(126) * 512 is similar to 8MB, enough to pass tests
#define INDIRECT_SIZE 124
/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    off_t length;                       /* File size in bytes. */
    unsigned magic;                     /* Magic number. */
	int iindex; 						/* Indirect Block Index */
	int is_dir; 						/* Is directory, 바이트 수 맞추기 위해 bool 대신 int 사용 */
    block_sector_t iblocks[INDIRECT_SIZE]; 	/* Indirect Blocks */
  };

struct indirect
{
	int index; 								/* Block Index */
	block_sector_t blocks[INDIRECT_SIZE]; 	/* Blocks */
	uint32_t unused[2]; 					/* Not used */
};

/* In-memory inode. */
struct inode
  {
    struct list_elem elem;              /* Element in inode list. */
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    struct inode_disk data;             /* Inode content. */
  };

struct bitmap;

void inode_init (void);
bool inode_create (block_sector_t, off_t, int);
struct inode *inode_open (block_sector_t);
struct inode *inode_reopen (struct inode *);
block_sector_t inode_get_inumber (const struct inode *);
void inode_close (struct inode *);
void inode_remove (struct inode *);
off_t inode_read_at (struct inode *, void *, off_t size, off_t offset);
off_t inode_write_at (struct inode *, const void *, off_t size, off_t offset);
void inode_deny_write (struct inode *);
void inode_allow_write (struct inode *);
off_t inode_length (const struct inode *);

// Project 4
void allocate_indirect(int, struct inode_disk *);
int allocate_more(struct inode *, int);
void free_indirect(struct inode_disk *);


#endif /* filesys/inode.h */
