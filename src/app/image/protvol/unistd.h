// Dummy unistd.h file

#ifndef unistd_h
#define unistd_h

#include <io.h>

#define lseek _lseek
#define read _read
#define write _write

#endif // ndef unistd_h
