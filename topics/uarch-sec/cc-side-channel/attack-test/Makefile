
default: all

all: shlib.so spy trojan

shlib.so: shlib.c common.h
	gcc -shared -fPIC shlib.c -o shlib.so

spy: spy.c common.h
	gcc -g -O0 -I. spy.c -o spy -ldl -lrt -pthread

trojan: trojan.c common.h
	gcc -g -O0 -I. trojan.c -o trojan -ldl -lrt -pthread

clean:
	rm -f shlib.so
	rm -f spy
	rm -f trojan
