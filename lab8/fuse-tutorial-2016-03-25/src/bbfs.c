/*
  Big Brother File System
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.
  A copy of that code is included in the file fuse.h
  
  The point of this FUSE filesystem is to provide an introduction to
  FUSE.  It was my first FUSE filesystem as I got to know the
  software; hopefully, the comments in this code will help people who
  follow later to get a gentler introduction.

  This might be called a no-op filesystem:  it doesn't impose
  filesystem semantics on top of any other existing structure.  It
  simply reports the requests that come in, and passes them to an
  underlying filesystem.  The information is saved in a logfile named
  bbfs.log, in the directory from which you run bbfs.
*/

#include "params.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"

#define BLOCK_SZ 4096
#define SIZE_HASH 41

void free_inode(struct file_node *fi);
struct file_node * getfilenode(struct fuse_file_info * fi);
int update_inode(const char* path, struct file_node *inode);
char * get_hash(const char *buf);
void decr_ref_count(const char *hash);
void incr_ref_count(const char *hash, const char *buf);

void free_inode(struct file_node *fi){
	for (int i=0; i<fi->n_blocks; ++i){
		if (fi->block[i] != NULL){
			free(fi->block[i]);
		}
	}
	if (fi->block != NULL) free(fi->block);

	free(fi);
}

struct file_node * getfilenode(struct fuse_file_info * fi){
	int status;

	struct file_node * ret = malloc(sizeof(struct file_node));
	ret->size = 0;
	ret->n_blocks = 0;
	ret->block = NULL;

	status = pread(fi->fh, &ret, sizeof(struct file_cnt), 0);
	if (status == -1){
		ret->size = 0;
		ret->n_blocks = 0;
		free_inode(ret);
		return NULL;
	}
	ret->block = malloc(sizeof(char*)*ret->size);
	for (int i=0; i<ret->size; ++i) ret->block[i] = NULL;

	for (int i=0; i<ret->size; ++i){
		status = pread(fi->fh, ret->block[i], SIZE_HASH*sizeof(char), sizeof(struct file_cnt) + i*SIZE_HASH*sizeof(char));
		if (status == -1){
			free_inode(ret);
			return NULL;
		}
	}

	return ret;
}

int update_inode(const char* path, struct file_node *inode){
	if (path == NULL || inode == NULL) return -1;
	int ret = 0;

	char realpath[400];
	sprintf(realpath, "%s/%s", BB_DATA->rootdir, path);

	int fd = open(realpath, O_WRONLY);
	if (fd == -1){
		log_error("update_inode: error opening file");
		return -1;
	}

	int status = pwrite(fd, inode, sizeof(struct file_cnt), 0);
	if(status != sizeof(struct file_cnt)){
		log_error("update_inode, can't write file_cnt\n");
		close(fd);
		return -1;
	}

	for (int i=0; i<inode->size; ++i){
		status = pwrite(fd, inode->block[i], SIZE_HASH*sizeof(char), sizeof(struct file_cnt) + i*SIZE_HASH*sizeof(char));
		if (status == -1){
			log_error("update_inode, can't write block\n");
			close(fd);
			return -1;
		}
	}

	close(fd);
	return 0;
}

// increment ref count 
// check if hash.meta file exists
// if not
//			create two files 
//					h.data . Write into it actual content
//					h.meta . Write into it ref count
// if present
//			update the ref count of h.meta 
void incr_ref_count(const char *hash, const char *buf){

	char meta_file_path[200], data_file_path[200];
	sprintf(meta_file_path, "%s/.META/%s.meta", BB_DATA->rootdir, hash);
	sprintf(data_file_path, "%s/.META/%s.data", BB_DATA->rootdir, hash);

	// check if both file exists
	int n; // always to check count of read or written
	int count;
	int d_fd = open(data_file_path, O_RDWR);

	if(d_fd < 0){
		// create a file and write buf to it
		d_fd = open(data_file_path, O_RDWR | O_CREAT);
		
		n = pwrite(d_fd, (void *)buf, sizeof(buf), 0 );
		
		if(n < 0){
			log_msg("incr_ref_count::cannot write to .data .\n");
			close(d_fd);
			return;
		}
	}

	int m_fd = open(meta_file_path, O_RDWR);

	if(m_fd < 0){
		// create the file and write to it 1
		m_fd = open(meta_file_path, O_RDWR | O_CREAT);
		count = 1;
		n = pwrite(d_fd, (void *)count, sizeof(int), 0 );
		if (n < 0 ){
			log_msg("incr_ref_count::cannot write to .meta .\n");
			close(m_fd);
			return;
		}
	}
	else{

		n = pread(m_fd, (void *)count, sizeof(int), 0);

		if( n < 0){
			log_msg("incr_ref_count::Unsuccessful read count\n");
			return;		
		}

		count++;

		n = pwrite(m_fd, (void *)count, sizeof(int), 0 );

		if(n < 0){
			log_msg("incr_ref_count::Unsuccessful write count \n");
			return;
		}

	}

	return;

}



// decrement ref count
// open h.meta . Decrease the ref count by one 

void decr_ref_count(const char *hash){

	//open file corresponding to hash
	char meta_file_path[200];
	sprintf(meta_file_path, "%s/.META/%s.meta", BB_DATA->rootdir, hash);

	int count;
	//read from block and close
	int fd = open(meta_file_path, O_RDWR);

	if (fd < 0 ){
		log_msg("decr_ref_count::file does not exists .\n");
		return;
	}

	int n = pread(fd, (void *)count, sizeof(int), 0);

	if( n < 0){
		log_msg("decr_ref_count::Unsuccessful read\n");
		return;		
	}

	count--;

	n = pwrite(fd, (void *)count, sizeof(int), 0 );

	if(n < 0){
		log_msg("decr_ref_count::Unsuccessful write\n");
		return;
	}

	//close fd
	close(fd);


}


//  All the paths I see are relative to the root of the mounted
//  filesystem.  In order to get to the underlying filesystem, I need to
//  have the mountpoint.  I'll save it away early on in main(), and then
//  whenever I need a path for something I'll call this to construct
//  it.
static void bb_fullpath(char fpath[PATH_MAX], const char *path){
	strcpy(fpath, BB_DATA->rootdir);
	strncat(fpath, path, PATH_MAX); // ridiculously long paths will
					// break here

	log_msg("    bb_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
		BB_DATA->rootdir, path, fpath);
}

///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come from /usr/include/fuse.h
//
/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int bb_getattr(const char *path, struct stat *statbuf){
	int retstat;
	char fpath[PATH_MAX];
	
	log_msg("\nbb_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);
	bb_fullpath(fpath, path);

	retstat = log_syscall("lstat", lstat(fpath, statbuf), 0);
	
	log_stat(statbuf);
	
	return retstat;
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// Note the system readlink() will truncate and lose the terminating
// null.  So, the size passed to to the system readlink() must be one
// less than the size passed to bb_readlink()
// bb_readlink() code by Bernardo F Costa (thanks!)
int bb_readlink(const char *path, char *link, size_t size){
	int retstat;
	char fpath[PATH_MAX];
	
	log_msg("bb_readlink(path=\"%s\", link=\"%s\", size=%d)\n",
	  path, link, size);
	bb_fullpath(fpath, path);

	retstat = log_syscall("fpath", readlink(fpath, link, size - 1), 0);
	if (retstat >= 0) {
	link[retstat] = '\0';
	retstat = 0;
	}
	
	return retstat;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
// shouldn't that comment be "if" there is no.... ?
int bb_mknod(const char *path, mode_t mode, dev_t dev){
	int retstat;
	char fpath[PATH_MAX];
	
	log_msg("\nbb_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n",
	  path, mode, dev);
	bb_fullpath(fpath, path);
	
	// On Linux this could just be 'mknod(path, mode, dev)' but this
	// tries to be be more portable by honoring the quote in the Linux
	// mknod man page stating the only portable use of mknod() is to
	// make a fifo, but saying it should never actually be used for
	// that.
	if (S_ISREG(mode)) {
		retstat = log_syscall("open", open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode), 0);
		if (retstat >= 0){
			retstat = log_syscall("close", close(retstat), 0);
			struct stat st;
			stat(fpath, &st);
			int size = st.st_size;
			if (size == 0){
				log_msg("mknod, initializing file\n");
				struct file_cnt * temp = malloc(sizeof(struct file_cnt));
				temp->n_blocks = 0;
				temp->size = 0;
				FILE *file= fopen(fpath, "wb");
				fwrite(temp, sizeof(struct file_cnt), 1, file);
				fclose(file);
			}
		}
	} 
	else{
		if (S_ISFIFO(mode))
			retstat = log_syscall("mkfifo", mkfifo(fpath, mode), 0);
		else
			retstat = log_syscall("mknod", mknod(fpath, mode, dev), 0);
	}

	return retstat;
}

/** Create a directory */
int bb_mkdir(const char *path, mode_t mode){
	char fpath[PATH_MAX];
	
	log_msg("\nbb_mkdir(path=\"%s\", mode=0%3o)\n",
		path, mode);
	bb_fullpath(fpath, path);

	return log_syscall("mkdir", mkdir(fpath, mode), 0);
}

/** Remove a file */
int bb_unlink(const char *path){
	char fpath[PATH_MAX];
	
	log_msg("bb_unlink(path=\"%s\")\n",
		path);
	bb_fullpath(fpath, path);

	return log_syscall("unlink", unlink(fpath), 0);
}

/** Remove a directory */
int bb_rmdir(const char *path){
	char fpath[PATH_MAX];
	
	log_msg("bb_rmdir(path=\"%s\")\n",
		path);
	bb_fullpath(fpath, path);

	return log_syscall("rmdir", rmdir(fpath), 0);
}

/** Create a symbolic link */
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
int bb_symlink(const char *path, const char *link){
	char flink[PATH_MAX];
	
	log_msg("\nbb_symlink(path=\"%s\", link=\"%s\")\n",
		path, link);
	bb_fullpath(flink, link);

	return log_syscall("symlink", symlink(path, flink), 0);
}

/** Rename a file */
// both path and newpath are fs-relative
int bb_rename(const char *path, const char *newpath){
	char fpath[PATH_MAX];
	char fnewpath[PATH_MAX];
	
	log_msg("\nbb_rename(fpath=\"%s\", newpath=\"%s\")\n",
		path, newpath);
	bb_fullpath(fpath, path);
	bb_fullpath(fnewpath, newpath);

	return log_syscall("rename", rename(fpath, fnewpath), 0);
}

/** Create a hard link to a file */
int bb_link(const char *path, const char *newpath){
	char fpath[PATH_MAX], fnewpath[PATH_MAX];
	
	log_msg("\nbb_link(path=\"%s\", newpath=\"%s\")\n",
		path, newpath);
	bb_fullpath(fpath, path);
	bb_fullpath(fnewpath, newpath);

	return log_syscall("link", link(fpath, fnewpath), 0);
}

/** Change the permission bits of a file */
int bb_chmod(const char *path, mode_t mode){
	char fpath[PATH_MAX];
	
	log_msg("\nbb_chmod(fpath=\"%s\", mode=0%03o)\n",
		path, mode);
	bb_fullpath(fpath, path);

	return log_syscall("chmod", chmod(fpath, mode), 0);
}

/** Change the owner and group of a file */
int bb_chown(const char *path, uid_t uid, gid_t gid){
	char fpath[PATH_MAX];
	
	log_msg("\nbb_chown(path=\"%s\", uid=%d, gid=%d)\n",
		path, uid, gid);
	bb_fullpath(fpath, path);

	return log_syscall("chown", chown(fpath, uid, gid), 0);
}

/** Change the size of a file */
int bb_truncate(const char *path, off_t newsize){
	char fpath[PATH_MAX];
	
	log_msg("\nbb_truncate(path=\"%s\", newsize=%lld)\n",
		path, newsize);
	bb_fullpath(fpath, path);

	return log_syscall("truncate", truncate(fpath, newsize), 0);
}

/** Change the access and/or modification times of a file */
/* note -- I'll want to change this as soon as 2.6 is in debian testing */
int bb_utime(const char *path, struct utimbuf *ubuf){
	char fpath[PATH_MAX];
	
	log_msg("\nbb_utime(path=\"%s\", ubuf=0x%08x)\n",
		path, ubuf);
	bb_fullpath(fpath, path);

	return log_syscall("utime", utime(fpath, ubuf), 0);
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int bb_open(const char *path, struct fuse_file_info *fi){
	int retstat = 0;
	int fd;
	char fpath[PATH_MAX];
	
	log_msg("\nbb_open(path\"%s\", fi=0x%08x)\n",
		path, fi);
	bb_fullpath(fpath, path);

	// if the open call succeeds, my retstat is the file descriptor,
	// else it's -errno.  I'm making sure that in that case the saved
	// file descriptor is exactly -1.
	fd = log_syscall("open", open(fpath, fi->flags), 0);
	if (fd < 0)
	retstat = log_error("open");
	
	fi->fh = fd;

	log_fi(fi);
	
	return retstat;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
// I don't fully understand the documentation above -- it doesn't
// match the documentation for the read() system call which says it
// can return with anything up to the amount of data requested. nor
// with the fusexmp code which returns the amount of data also
// returned by read.
int bb_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	int retstat = 0;
	
	// take fuse_file_info as pointer and returns file_node structure (refer params.h)
	struct file_node *inode = getfilenode(fi);
	if (inode == NULL){
		log_msg("bb_read file not found\n");
		return -1;
	}

	int block_n = offset/BLOCK_SZ;


	if (inode->size < offset || block_n >= inode->n_blocks){
		log_msg("bb_read out of bounds read\n");
		return -1;
	}

	//open file corresponding to hash
	char block_file_path[200];
	sprintf(block_file_path, "%s/.META/%s.data", BB_DATA->rootdir, inode->block[block_n]);
	FILE *block_file = fopen(block_file_path, "r");

	//read from block and close
	int st = fread((void *)buf, sizeof(char), BLOCK_SZ, block_file);
	fclose(block_file);
	free_inode(inode);

	if (st != BLOCK_SZ){

		log_msg("bb_read Unsuccessful read\n");
	}
	else {
		log_msg("\nbb_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
			path, buf, size, offset, fi);
		// no need to get fpath on this one, since I work from fi->fh not the path
		log_fi(fi);

		return 0;
	}
	
	// return log_syscall("pread", pread(fi->fh, buf, size, offset), 0);
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
// As  with read(), the documentation above is inconsistent with the
// documentation for the write() system call.
int bb_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	// assume size = BLOCK_SZ and offset is a multiple of BLOCK_SZ

	int retstat = 0;

	struct file_node *inode = getfilenode(fi);
	if (inode == NULL){
		log_msg("bb_read file not found\n");
		return -1;
	}

	int block_n = offset/BLOCK_SZ;

	// if (inode->size < offset || block_n >= inode->n_blocks){
	// 	log_msg("bb_read out of bounds read\n");
	// 	return -1;
	// }

	char *hash_str = get_hash(buf);

	if (hash_str == NULL){
		retstat = -1;
	}
	else if (block_n < inode->n_blocks){
		decr_ref_count(inode->block[block_n]);
		incr_ref_count(hash_str, buf);

		strcpy(inode->block[block_n], hash_str);

		update_inode(path, inode);
	}
	else if (block_n == inode->size){
		incr_ref_count(hash_str, buf);
		inode->size += BLOCK_SZ;
		inode->n_blocks++;

		strcpy(inode->block[block_n], hash_str);

		update_inode(path, inode);
	}
	else{
		retstat = -1;
	}

	free_inode(inode);

	return retstat;


	//calculate hash of buffer

	//check if hash exists
	//if so appropriately change block change block 

	//else create block and add block entry

	log_msg("\nbb_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
		path, buf, size, offset, fi
		);
	// no need to get fpath on this one, since I work from fi->fh not the path
	log_fi(fi);

	return log_syscall("pwrite", pwrite(fi->fh, buf, size, offset), 0);
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 * Replaced 'struct statfs' parameter with 'struct statvfs' in
 * version 2.5
 */
int bb_statfs(const char *path, struct statvfs *statv){
	int retstat = 0;
	char fpath[PATH_MAX];
	
	log_msg("\nbb_statfs(path=\"%s\", statv=0x%08x)\n",
		path, statv);
	bb_fullpath(fpath, path);
	
	// get stats for underlying filesystem
	retstat = log_syscall("statvfs", statvfs(fpath, statv), 0);
	
	log_statvfs(statv);
	
	return retstat;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
// this is a no-op in BBFS.  It just logs the call and returns success
int bb_flush(const char *path, struct fuse_file_info *fi){
	log_msg("\nbb_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
	// no need to get fpath on this one, since I work from fi->fh not the path
	log_fi(fi);
	
	return 0;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int bb_release(const char *path, struct fuse_file_info *fi){
	log_msg("\nbb_release(path=\"%s\", fi=0x%08x)\n",
		path, fi);
	log_fi(fi);

	// We need to close the file.  Had we allocated any resources
	// (buffers etc) we'd need to free them here as well.
	return log_syscall("close", close(fi->fh), 0);
}

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 * Changed in version 2.2
 */
int bb_fsync(const char *path, int datasync, struct fuse_file_info *fi){
	log_msg("\nbb_fsync(path=\"%s\", datasync=%d, fi=0x%08x)\n",
		path, datasync, fi);
	log_fi(fi);
	
	// some unix-like systems (notably freebsd) don't have a datasync call
	#ifdef HAVE_FDATASYNC
		if (datasync)
		return log_syscall("fdatasync", fdatasync(fi->fh), 0);
		else
	#endif
	return log_syscall("fsync", fsync(fi->fh), 0);
}

#ifdef HAVE_SYS_XATTR_H
/** Set extended attributes */
int bb_setxattr(const char *path, const char *name, const char *value, size_t size, int flags){
	char fpath[PATH_MAX];
	
	log_msg("\nbb_setxattr(path=\"%s\", name=\"%s\", value=\"%s\", size=%d, flags=0x%08x)\n",
		path, name, value, size, flags);
	bb_fullpath(fpath, path);

	return log_syscall("lsetxattr", lsetxattr(fpath, name, value, size, flags), 0);
}

/** Get extended attributes */
int bb_getxattr(const char *path, const char *name, char *value, size_t size){
	int retstat = 0;
	char fpath[PATH_MAX];
	
	log_msg("\nbb_getxattr(path = \"%s\", name = \"%s\", value = 0x%08x, size = %d)\n",
		path, name, value, size);
	bb_fullpath(fpath, path);

	retstat = log_syscall("lgetxattr", lgetxattr(fpath, name, value, size), 0);
	if (retstat >= 0)
	log_msg("    value = \"%s\"\n", value);
	
	return retstat;
}

/** List extended attributes */
int bb_listxattr(const char *path, char *list, size_t size){
	int retstat = 0;
	char fpath[PATH_MAX];
	char *ptr;
	
	log_msg("bb_listxattr(path=\"%s\", list=0x%08x, size=%d)\n",
		path, list, size
		);
	bb_fullpath(fpath, path);

	retstat = log_syscall("llistxattr", llistxattr(fpath, list, size), 0);
	if (retstat >= 0) {
	log_msg("    returned attributes (length %d):\n", retstat);
	for (ptr = list; ptr < list + retstat; ptr += strlen(ptr)+1)
		log_msg("    \"%s\"\n", ptr);
	}
	
	return retstat;
}

/** Remove extended attributes */
int bb_removexattr(const char *path, const char *name){
	char fpath[PATH_MAX];
	
	log_msg("\nbb_removexattr(path=\"%s\", name=\"%s\")\n",
		path, name);
	bb_fullpath(fpath, path);

	return log_syscall("lremovexattr", lremovexattr(fpath, name), 0);
}
#endif

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int bb_opendir(const char *path, struct fuse_file_info *fi){
	DIR *dp;
	int retstat = 0;
	char fpath[PATH_MAX];
	
	log_msg("\nbb_opendir(path=\"%s\", fi=0x%08x)\n",
	  path, fi);
	bb_fullpath(fpath, path);

	// since opendir returns a pointer, takes some custom handling of
	// return status.
	dp = opendir(fpath);
	log_msg("    opendir returned 0x%p\n", dp);
	if (dp == NULL)
	retstat = log_error("bb_opendir opendir");
	
	fi->fh = (intptr_t) dp;
	
	log_fi(fi);
	
	return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */

int bb_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	int retstat = 0;
	DIR *dp;
	struct dirent *de;
	
	log_msg("\nbb_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",
		path, buf, filler, offset, fi);
	// once again, no need for fullpath -- but note that I need to cast fi->fh
	dp = (DIR *) (uintptr_t) fi->fh;

	// Every directory contains at least two entries: . and ..  If my
	// first call to the system readdir() returns NULL I've got an
	// error; near as I can tell, that's the only condition under
	// which I can get an error from readdir()
	de = readdir(dp);
	log_msg("    readdir returned 0x%p\n", de);
	if (de == 0) {
	retstat = log_error("bb_readdir readdir");
	return retstat;
	}

	// This will copy the entire directory into the buffer.  The loop exits
	// when either the system readdir() returns NULL, or filler()
	// returns something non-zero.  The first case just means I've
	// read the whole directory; the second means the buffer is full.
	do {
	log_msg("calling filler with name %s\n", de->d_name);
	if (filler(buf, de->d_name, NULL, 0) != 0) {
		log_msg("    ERROR bb_readdir filler:  buffer full");
		return -ENOMEM;
	}
	} while ((de = readdir(dp)) != NULL);
	
	log_fi(fi);
	
	return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int bb_releasedir(const char *path, struct fuse_file_info *fi){
	int retstat = 0;
	
	log_msg("\nbb_releasedir(path=\"%s\", fi=0x%08x)\n",
		path, fi);
	log_fi(fi);
	
	closedir((DIR *) (uintptr_t) fi->fh);
	
	return retstat;
}

/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 * Introduced in version 2.3
 */
// when exactly is this called?  when a user calls fsync and it
// happens to be a directory? ??? >>> I need to implement this...
int bb_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi){
	int retstat = 0;
	
	log_msg("\nbb_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x)\n",
		path, datasync, fi);
	log_fi(fi);
	
	return retstat;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
// Undocumented but extraordinarily useful fact:  the fuse_context is
// set up before this function is called, and
// fuse_get_context()->private_data returns the user_data passed to
// fuse_main().  Really seems like either it should be a third
// parameter coming in here, or else the fact should be documented
// (and this might as well return void, as it did in older versions of
// FUSE).
void *bb_init(struct fuse_conn_info *conn){
	log_msg("\nbb_init()\n");

	char cmd[200];
	sprintf(cmd, "mkdir -p %s/.META", BB_DATA->rootdir);
	int status = system(cmd);
	log_msg("Initialized .META dir..\n");
	
	log_conn(conn);
	log_fuse_context(fuse_get_context());
	
	return BB_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void bb_destroy(void *userdata){
	log_msg("\nbb_destroy(userdata=0x%08x)\n", userdata);
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
int bb_access(const char *path, int mask){
	int retstat = 0;
	char fpath[PATH_MAX];
   
	log_msg("\nbb_access(path=\"%s\", mask=0%o)\n",
		path, mask);
	bb_fullpath(fpath, path);
	
	retstat = access(fpath, mask);
	
	if (retstat < 0)
	retstat = log_error("bb_access access");
	
	return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
// Not implemented.  I had a version that used creat() to create and
// open the file, which it turned out opened the file write-only.

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the truncate() method will be
 * called instead.
 *
 * Introduced in version 2.5
 */
int bb_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi){
	int retstat = 0;
	
	log_msg("\nbb_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x)\n",
		path, offset, fi);
	log_fi(fi);
	
	retstat = ftruncate(fi->fh, offset);
	if (retstat < 0)
	retstat = log_error("bb_ftruncate ftruncate");
	
	return retstat;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int bb_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi){
	int retstat = 0;
	
	log_msg("\nbb_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n",
		path, statbuf, fi);
	log_fi(fi);

	// On FreeBSD, trying to do anything with the mountpoint ends up
	// opening it, and then using the FD for an fgetattr.  So in the
	// special case of a path of "/", I need to do a getattr on the
	// underlying root directory instead of doing the fgetattr().
	if (!strcmp(path, "/"))
	return bb_getattr(path, statbuf);
	
	retstat = fstat(fi->fh, statbuf);
	if (retstat < 0)
	retstat = log_error("bb_fgetattr fstat");
	
	log_stat(statbuf);
	
	return retstat;
}

struct fuse_operations bb_oper = {
	.getattr = bb_getattr,
	.readlink = bb_readlink,
	// no .getdir -- that's deprecated
	.getdir = NULL,
	.mknod = bb_mknod,
	.mkdir = bb_mkdir,
	.unlink = bb_unlink,
	.rmdir = bb_rmdir,
	.symlink = bb_symlink,
	.rename = bb_rename,
	.link = bb_link,
	.chmod = bb_chmod,
	.chown = bb_chown,
	.truncate = bb_truncate,
	.utime = bb_utime,
	.open = bb_open,
	.read = bb_read,
	.write = bb_write,
	/** Just a placeholder, don't set */ // huh???
	.statfs = bb_statfs,
	.flush = bb_flush,
	.release = bb_release,
	.fsync = bb_fsync,

	#ifdef HAVE_SYS_XATTR_H
	.setxattr = bb_setxattr,
	.getxattr = bb_getxattr,
	.listxattr = bb_listxattr,
	.removexattr = bb_removexattr,
	#endif

	.opendir = bb_opendir,
	.readdir = bb_readdir,
	.releasedir = bb_releasedir,
	.fsyncdir = bb_fsyncdir,
	.init = bb_init,
	.destroy = bb_destroy,
	.access = bb_access,
	.ftruncate = bb_ftruncate,
	.fgetattr = bb_fgetattr
};

void bb_usage(){
	fprintf(stderr, "usage:  bbfs [FUSE and mount options] rootDir mountPoint\n");
	abort();
}

// // get number of directories in directory
// int dircount(char *path){
// 	int count = 0;
// 	DIR *dir;
// 	struct dirent *entry;

// 	dir = opendir(path);
// 	if (dir == NULL) return -1;

// 	while ((entry = readdir(dir)) != NULL) {
// 		if (entry->d_type == DT_DIR){ /* If the entry is a directory*/
// 			count++;
// 		}
// 	}

// 	closedir(dir);
// }

// // get number of directories in directory
// int filecount(char *path){
// 	int count = 0;
// 	DIR *dir;
// 	struct dirent *entry;

// 	dir = opendir(path);
// 	if (dir == NULL) return -1;

// 	while ((entry = readdir(dir)) != NULL) {
// 		if (entry->d_type == DT_REG){ /* If the entry is a regular file */
// 			count++;
// 		}
// 	}

// 	closedir(dir);
// }


int main(int argc, char *argv[]){
	int fuse_stat;
	struct bb_state *bb_data;

	// bbfs doesn't do any access checking on its own (the comment
	// blocks in fuse.h mention some of the functions that need
	// accesses checked -- but note there are other functions, like
	// chown(), that also need checking!).  Since running bbfs as root
	// will therefore open Metrodome-sized holes in the system
	// security, we'll check if root is trying to mount the filesystem
	// and refuse if it is.  The somewhat smaller hole of an ordinary
	// user doing it with the allow_other flag is still there because
	// I don't want to parse the options string.
	if ((getuid() == 0) || (geteuid() == 0)) {
		fprintf(stderr, "Running BBFS as root opens unnacceptable security holes\n");
		return 1;
	}

	// See which version of fuse we're running
	fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
	
	// Perform some sanity checking on the command line:  make sure
	// there are enough arguments, and that neither of the last two
	// start with a hyphen (this will break if you actually have a
	// rootpoint or mountpoint whose name starts with a hyphen, but so
	// will a zillion other programs)
	if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	bb_usage();
	
	char *rootdir = realpath(argv[argc-2], NULL);
	fprintf(stderr, "Cleaning %s...\n", rootdir);
	char cmd[100];
	sprintf(cmd, "rm -rf %s/*", rootdir);
	int status = system(cmd);
	fprintf(stderr, "\t Done with exit code %d\n", status);
	// fprintf(stderr, "Number of files:%d\nNumber of directories:%d\n", filecount(rootdir), dircount(rootdir));

	bb_data = malloc(sizeof(struct bb_state));
	if (bb_data == NULL) {
		perror("main calloc");
		abort();
	}

	// Pull the rootdir out of the argument list and save it in my
	// internal data
	bb_data->rootdir = realpath(argv[argc-2], NULL);
	argv[argc-2] = argv[argc-1];
	argv[argc-1] = NULL;
	argc--;
	
	bb_data->logfile = log_open();
	
	// turn over control to fuse
	fprintf(stderr, "about to call fuse_main\n");
	fuse_stat = fuse_main(argc, argv, &bb_oper, bb_data);
	fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
	
	return fuse_stat;
}


#ifndef SHA1_H
#define SHA1_H

/*
   SHA-1 in C
   By Steve Reid <steve@edmweb.com>
   100% Public Domain
 */

typedef struct
{
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(
    uint32_t state[5],
    const unsigned char buffer[64]
    );

void SHA1Init(
    SHA1_CTX * context
    );

void SHA1Update(
    SHA1_CTX * context,
    const unsigned char *data,
    uint32_t len
    );

void SHA1Final(
    unsigned char digest[20],
    SHA1_CTX * context
    );

void SHA1(
    char *hash_out,
    const char *str,
    int len);

#endif /* SHA1_H */

/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

/* #define LITTLE_ENDIAN * This should be #define'd already, if true. */
/* #define SHA1HANDSOFF * Copies data before messing with it. */

#define SHA1HANDSOFF

#include <stdio.h>
#include <string.h>

/* for uint32_t */
#include <stdint.h>



#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#if BYTE_ORDER == LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#elif BYTE_ORDER == BIG_ENDIAN
#define blk0(i) block->l[i]
#else
#error "Endianness not defined!"
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


/* Hash a single 512-bit block. This is the core of the algorithm. */

void SHA1Transform(
    uint32_t state[5],
    const unsigned char buffer[64]
)
{
    uint32_t a, b, c, d, e;

    typedef union
    {
        unsigned char c[64];
        uint32_t l[16];
    } CHAR64LONG16;

#ifdef SHA1HANDSOFF
    CHAR64LONG16 block[1];      /* use array to appear as a pointer */

    memcpy(block, buffer, 64);
#else
    /* The following had better never be used because it causes the
     * pointer-to-const buffer to be cast into a pointer to non-const.
     * And the result is written through.  I threw a "const" in, hoping
     * this will cause a diagnostic.
     */
    CHAR64LONG16 *block = (const CHAR64LONG16 *) buffer;
#endif
    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);
    R0(b, c, d, e, a, 4);
    R0(a, b, c, d, e, 5);
    R0(e, a, b, c, d, 6);
    R0(d, e, a, b, c, 7);
    R0(c, d, e, a, b, 8);
    R0(b, c, d, e, a, 9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
#ifdef SHA1HANDSOFF
    memset(block, '\0', sizeof(block));
#endif
}


/* SHA1Init - Initialize new context */

void SHA1Init(
    SHA1_CTX * context
)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void SHA1Update(
    SHA1_CTX * context,
    const unsigned char *data,
    uint32_t len
)
{
    uint32_t i;

    uint32_t j;

    j = context->count[0];
    if ((context->count[0] += len << 3) < j)
        context->count[1]++;
    context->count[1] += (len >> 29);
    j = (j >> 3) & 63;
    if ((j + len) > 63)
    {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1Transform(context->state, context->buffer);
        for (; i + 63 < len; i += 64)
        {
            SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else
        i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void SHA1Final(
    unsigned char digest[20],
    SHA1_CTX * context
)
{
    unsigned i;

    unsigned char finalcount[8];

    unsigned char c;

#if 0    /* untested "improvement" by DHR */
    /* Convert context->count to a sequence of bytes
     * in finalcount.  Second element first, but
     * big-endian order within element.
     * But we do it all backwards.
     */
    unsigned char *fcp = &finalcount[8];
    for (i = 0; i < 2; i++)
    {
        uint32_t t = context->count[i];
        int j;
        for (j = 0; j < 4; t >>= 8, j++)
            *--fcp = (unsigned char) t}
#else
    for (i = 0; i < 8; i++)
    {
        finalcount[i] = (unsigned char) ((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);      /* Endian independent */
    }
#endif
    c = 0200;
    SHA1Update(context, &c, 1);
    while ((context->count[0] & 504) != 448)
    {
        c = 0000;
        SHA1Update(context, &c, 1);
    }
    SHA1Update(context, finalcount, 8); /* Should cause a SHA1Transform() */
    for (i = 0; i < 20; i++)
    {
        digest[i] = (unsigned char)
            ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
    /* Wipe variables */
    memset(context, '\0', sizeof(*context));
    memset(&finalcount, '\0', sizeof(finalcount));
}

void SHA1(
    char *hash_out,
    const char *str,
    int len)
{
    SHA1_CTX ctx;
    unsigned int ii;

    SHA1Init(&ctx);
    for (ii=0; ii<len; ii+=1)
        SHA1Update(&ctx, (const unsigned char*)str + ii, 1);
    SHA1Final((unsigned char *)hash_out, &ctx);
    hash_out[20] = '\0';
}

char * get_hash(const char *buf){
	char result[21];
	char *hexresult = malloc(SIZE_HASH*sizeof(char));
	size_t offset;

	/* calculate hash */
	SHA1( result, buf, BLOCK_SZ);

	/* format the hash for comparison */
	for( offset = 0; offset < 20; offset++) {
		sprintf( ( hexresult + (2*offset)), "%02x", result[offset]&0xff);
	}
	hexresult[41] = '\0';

	return hexresult;
}