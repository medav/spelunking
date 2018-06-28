

default: create_shmem open_shmem
CC=clang

%: %.cc
	$(CC) $< -o $@ -lrt

clean:
	rm create_shmem open_shmem