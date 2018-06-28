#include <iostream>

//
// This macro is named NOMANGLE because extern "C" will ensure that declarations
// will only have C style naming (I.e. no C++ name mangling). This isn't 
// _strictly_ necessary but it helps for the client code because it doesn't
// need to go figured out some mangled name when it looks up function addresses.
//

#define NOMANGLE extern "C"

//
// On Linux, at least, this function is still visible and can be looked up via
// dlsym, but when compiling as C++ it _could_ received a mangled symbol name
// which is hard (relatively speaking) to lookup.
//

int Bar(int a, int b) {
    return a + b;
}

//
// This function is intened to be looked up via dlsym and serves to return a 
// function pointer to Bar. This isn't actually necessary since we could just
// as easily mark Bar as NOMANGLE. Still, I put this here to illustrate a
// common pattern in shared libraries - which is to have one or a small number
// of "export" functions that return a dispatch table for library functions.
//

NOMANGLE void * GetBar(void) {
    return (void *)Bar;
}