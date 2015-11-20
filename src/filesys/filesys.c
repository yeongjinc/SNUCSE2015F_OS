#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/thread.h"
#include "threads/malloc.h"

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format)
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  inode_init ();
  free_map_init ();

  if (format)
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void)
{
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *path, off_t initial_size)
{
  block_sector_t inode_sector = 0;
  struct dir *dir = dir_from_path(path);
  char *file_name = filename_from_path(path);
  bool success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size, 0)
                  && dir_add (dir, file_name, inode_sector));
  if (!success && inode_sector != 0)
    free_map_release (inode_sector, 1);
  dir_close (dir);
  free(file_name);

  return success;
}

bool
filesys_mkdir(const char *path)
{
	block_sector_t inode_sector = 0;
	struct dir *dir = dir_from_path(path);
	char *dir_name = filename_from_path(path);
	struct inode *dummy; 		 // 같은 이름 파일 존재하는지 확인용

	bool success = (dir != NULL
					&& ! dir_lookup(dir, dir_name, &dummy)
					&& free_map_allocate(1, &inode_sector)
					&& dir_create(inode_sector, 16, dir)
					&& dir_add(dir, dir_name, inode_sector));

	if( ! success && inode_sector != 0)
		free_map_release(inode_sector, 1);
	dir_close(dir);
	free(dir_name);

	return success;
}

bool
filesys_chdir(const char *path)
{
	char *include_dir = malloc(strlen(path) + 3);
	char *cur = "/.";
	memcpy(include_dir, path, strlen(path));
	memcpy(include_dir+strlen(path), cur, 3);
	struct dir *dir = dir_from_path(include_dir);
	if(dir == NULL)
	{
		free(include_dir);
		return false;
	}

	dir_close(thread_current()->directory);
	thread_current()->directory = dir;

	free(include_dir);
	return true;
}

// just for wrapper (do not access directory.c from syscall.c)
struct dir *
filesys_opendir(struct inode *inode)
{
	return dir_open(inode);
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *path)
{
  if(path == NULL || strlen(path) == 0)
	  return NULL;
  struct dir *dir = dir_from_path(path);
  if(dir == NULL)
	  return NULL;
  struct inode *inode = NULL;
  char *file_name = filename_from_path(path);

  if(file_name == NULL || strlen(file_name) == 0)
	  inode = dir_get_inode(dir);
  else
	  dir_lookup (dir, file_name, &inode);

  dir_close (dir);
  free(file_name);

  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *path)
{
  struct dir *dir = dir_from_path(path);
  char *file_name = filename_from_path(path);
  bool success = dir != NULL && dir_remove (dir, file_name);
  dir_close (dir);
  free(file_name);

  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16, NULL))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}



struct dir *dir_from_path(const char *path)
{
	//TODO handle '//'

	char buf[strlen(path) + 1];
	struct dir *dir;

	memcpy(buf, path, strlen(path)+1);

	if(thread_current()->directory == NULL || path[0] == '/')
		dir = dir_open_root();
	else
		dir = dir_reopen(thread_current()->directory);

	char *tok, *ptr, *prev; //마지막 것 파싱하지 않으려고 prev 만듦
	struct inode *inode;
	prev = strtok_r(buf, "/", &ptr);
	tok = strtok_r(NULL, "/", &ptr);
	while(tok != NULL)
	{
		//printf("dir tok : %s\n", tok);
		if(strcmp(prev, ".") == 0)
		{
		}
		else // ".."은 dir_entry에 삽입하여 처리
		{
			if( ! dir_lookup(dir, prev, &inode))
				return NULL;

			if(inode->data.is_dir == 1)
			{
				dir_close(dir);
				dir = dir_open(inode);
			}
			else
			{
				inode_close(inode);
			}
		}
		prev = tok;
		tok = strtok_r(NULL, "/", &ptr);
	}

	return dir;
}

char *
filename_from_path(const char *path)
{
	//TODO handle '//'

	//printf("path [%s]\n", path);
	char buf[strlen(path) + 1];
	memcpy(buf, path, strlen(path) + 1);

	char *tok, *ptr, *before = "";
	tok = strtok_r(buf, "/", &ptr);
	while(tok != NULL)
	{
		//printf("filename tok : %s\n", tok);
		before = tok;
		tok = strtok_r(NULL, "/", &ptr);
	}

	char *filename = malloc(strlen(before) + 1); // must be freed
	memcpy(filename, before, strlen(before) + 1);
	return filename;
}












