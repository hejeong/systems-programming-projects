
#define malloc( x ) mymalloc( x, __FILE__, __LINE__ );
#define free( x ) myfree( x, __FILE__, __LINE__ );

static char myblock[4096];

char * myFree(void * ptr, int file, int line);

void * myMalloc(int size, int file, int line);