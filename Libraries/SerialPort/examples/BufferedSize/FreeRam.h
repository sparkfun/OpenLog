#include <stdlib.h>
static inline int FreeRam() {
  extern char *__brkval;
  char top;
#if defined(CORE_TEENSY)
  return &top - __brkval;
#else  // malloc type
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // malloc type
}
