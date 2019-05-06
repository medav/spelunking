
default: all

all: microbench

microbench: microbench.c
	gcc -g -O0 -I. microbench.c -o microbench -ldl -lrt -pthread

microbench_m5:
	gcc -g -O0 -I. microbench.c -o microbench_m5 -DGEM5MODE -ldl -lrt -pthread

clean:
	rm -f microbench
