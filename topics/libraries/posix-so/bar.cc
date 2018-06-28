#include <iostream>
#include <dlfcn.h>
#include <stdint.h>

typedef int(*BarFunc)(int, int);
typedef void*(*GetBarFunc)(void);

int main() {

    //
    // Get a handle referring to the shared library we want to open. Note the 
    // './' prefix is there because otherwise dlopen won't search the current
    // directory but instead will search pre-configured paths and 
    // LD_LIBRARY_PATH for it and fail.
    //

    void * handle = dlopen("./libfoo.so", RTLD_LAZY);

    if (!handle) {
        printf("handle = 0x%lx\n", (uint64_t)handle);
        perror("dlopen");
        return 1;
    }

    //
    // Lookup the "Get" function. Note that this returns the actual address of
    // the symbol we search for and can be cast as a function pointer so it can
    // be called.
    //

    GetBarFunc GetBar = (GetBarFunc)dlsym(handle, "GetBarFunc");

    if (!GetBar) {
        perror("dlsym");
        return 1;
    }

    //
    // Now use the getter to get an address to Bar() and call it. All of these
    // functions (after dlopen) are in this process's address space so all the
    // work we've been doing is just getting addresses to them so they can be
    // called.
    //

    BarFunc Bar = (BarFunc)GetBar();
    std::cout << "Bar(2, 3) = " << Bar(2, 3) << std::endl;

    //
    // Release the handle we have to the shared library.
    //

    dlclose(handle);

    return 0;
}