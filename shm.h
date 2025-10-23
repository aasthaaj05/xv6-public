//KERNBASE-HEAPLIMIT=0x80000000 - 0x70000000 = 256 MB

#define SHMMAX    (4*1024*1024)   //4 MB max segment size
#define SHMMIN    1               //1 byte min segment size
#define MAXSHM    64              //max 64 segments
#define SHMSEG    MAXSHM          //max segments per process
#define MAX_PAGES 16

struct shmseg{
    int used;
    int key;
    int id;
    int size;
    int nattch;
    char *pages[MAX_PAGES];
};

extern struct shmseg shmtable[MAXSHM];
void shminit(void);
int shmget(int key, int size);


