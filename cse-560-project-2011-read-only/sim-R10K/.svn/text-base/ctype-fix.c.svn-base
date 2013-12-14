#include <ctype.h>

/*
 This file is a fix for the undefined `__ctype_b' reference in libexo.a. The
 problem is that the symbol `__ctype_b' has been deprecated in newer versions of
 glibc, but libexo hasn't been recompiled to reflect this. So we need to
 generate a reference to `__ctype_b' against which libexo can link. 

 We provide that with this file, and by invoking gcc with the flag
 `-Wl,--wrap,__ctype_b'. This causes the linker to rewrite all references to
 `__ctype_b' to refer to `__wrap__ctype_b' instead, which references the code
 below. Then we can invoke `__ctype_b_loc', the modern glibc replacement for
 `__ctype_b'.
*/

__const unsigned short int **__wrap___ctype_b (void) {
  return __ctype_b_loc();
}
