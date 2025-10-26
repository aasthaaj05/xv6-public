//KERNBASE-HEAPLIMIT=0x80000000 - 0x70000000 = 256 MB

#define SHMMAX    (4*1024*1024)   //4 MB max segment size
#define SHMMIN    1               //1 byte min segment size
#define MAXSHM    64              //max 64 segments
#define SHMSEG    MAXSHM          //max segments per process
#define MAX_PAGES 16

// IPC flags
#define IPC_PRIVATE 0x00001111    //create private segment
#define IPC_CREAT   0x00001000    //create if doesn't exist
#define IPC_EXCL    0x00002000    //fail if exists

//states
#define SHM_FREE        0
#define SHM_ALLOCATING  1
#define SHM_READY       2

struct shmseg{
    int state;         
    int key;           // user-provided key going ti be used for lookup
    int id;            // segment id (maybe use of index can be done or else we can have global var where we increment it when allocating segment
    int size;          // size of seg , req for calculation of pages
    int nattch;        // number of processes attached will help us in detemining whther we want to free that segment or not
    char *pages[MAX_PAGES]; // pointers to physical pages which has been done throught kalloc
};

// function prototypes
void shminit(void);
int shmget(int key, int size, int shmflg);