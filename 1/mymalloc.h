
static char myblock[4096];

void trigger();

char * myFree(void * ptr, char * file, int line);

void * myMalloc(int size, char * file, int line);