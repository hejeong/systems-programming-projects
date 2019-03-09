
static char myblock[4096];

void trigger();

char * myFree(void * ptr, int file, int line);

void * myMalloc(int size, int file, int line);