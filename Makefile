
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
HDRS := $(wildcard *.h)

CFLAGS := -g `pkg-config fuse --cflags` 
LDLIBS := `pkg-config fuse --libs`

nufs: $(OBJS)
	gcc $(CLFAGS) -o $@ nufs.o pages.o bitmap.o inode.o directory.o hash.o libmkfs.o $(LDLIBS)

mkfs: $(OBJS)
	gcc $(CLFAGS) -o mkfs mkfs.o mfs.o pages.o bitmap.o inode.o directory.o hash.o libmkfs.o $(LDLIBS)
	
read_root: $(OBJS)
	gcc $(CLFAGS) -o read_root read_root.o mfs.o pages.o directory.o bitmap.o inode.o hash.o libmkfs.o $(LDLIBS)
	
lookup: $(OBJS)
	gcc $(CLFAGS) -o lookup lookup.o pages.o directory.o bitmap.o inode.o hash.o libmkfs.o $(LDLIBS)

%.o: %.c $(HDRS)
	gcc $(CFLAGS) -c -o $@ $<

clean: unmount
	rm -f nufs *.o test.log data.nufs mkfs read_root lookup
	rmdir mnt || true

mount: nufs
	mkdir -p mnt || true
	./nufs -s -f mnt data.nufs

unmount:
	fusermount -u mnt || true

test: nufs
	perl test.pl

gdb: nufs
	mkdir -p mnt || true
	gdb --args ./nufs -s -f mnt data.nufs

open:
	gedit *.c *.h

.PHONY: clean mount unmount gdb
