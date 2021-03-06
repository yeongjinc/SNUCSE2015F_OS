#ifndef FILESYS_FILESYS_H
#define FILESYS_FILESYS_H

#include <stdbool.h>
#include "filesys/off_t.h"

/* Sectors of system file inodes. */
#define FREE_MAP_SECTOR 0       /* Free map file inode sector. */
#define ROOT_DIR_SECTOR 1       /* Root directory file inode sector. */

struct inode;

/* Block device that contains the file system. */
struct block *fs_device;

void filesys_init (bool format);
void filesys_done (void);
bool filesys_create (const char *name, off_t initial_size);
struct file *filesys_open (const char *name);
bool filesys_remove (const char *name);

/* Project 4 */
bool filesys_chdir(const char *);
bool filesys_mkdir(const char *);
struct dir *filesys_opendir(struct inode *inode);

struct dir *dir_from_path(const char *path);
char *filename_from_path(const char *path);

#endif /* filesys/filesys.h */
