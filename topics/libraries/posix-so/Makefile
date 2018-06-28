

default: libfoo.so bar
CC=clang++

bar: bar.o
	$(CC) $< -o $@ -ldl

%.so: %.o
	$(CC) -shared $< -o $@

%.o: %.cc
	$(CC) -fPIC -c $< -o $@

clean:
	rm *.o *.so bar