Deduplication supporting filesystem
===================================

Built on top of [BBFS](https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/).


File System Plan:
=================

The rootdir contains a directory ".META". Say file system currenltly stores
n distinct blocks. Consider a single block with SHA1 hash `h`. Corresponding
to this block one has two files in ".META":

	`h.data` is a file spanning a block and stores the actual block data
	`h.meta` is a file storing auxillary data like number of references to this block

Say we have a file in filesystem named `myfile.txt`
Actual contents of `myfile.txt` on disk will be:
	N_BLOCKS
	SIZE
	BLOCK1 (each of these values is nothing but hash value)
	BLOCK2
	.
	.
	.
	.
This structure of file will be read up by our filesystem and handled appropriately.
Directories are stored in usual way

* We clean the rootdir upon initialization. to make stuff easier.

TESTING:
========

We create a random file of size 10 M `head -c 10M </dev/urandom >file`
Then make 100 copies of the file in the mounted directory(takes some time).
In our case a `du -sh` of the root dir shows that only 6.6MB was used up.
