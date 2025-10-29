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

// shmat flags
#define SHM_RDONLY  0x00004000		//attach readonly
#define SHM_REMAP   0x00008000		//attach remapping

// shmctl commands
#define IPC_STAT    0x00010000    //get segment info
#define IPC_SET     0x00020000    //set segment info
#define IPC_RMID    0x00040000    //remove segment
#define IPC_INFO    0x00080000    //get system limits

//for IPC_STAT
struct shmid_ds {
    int shm_segsz;      // size of segment in bytes
    int shm_nattch;     // number of current attaches
    int shm_lpid;       // pid of last shmat/shmdt
    int shm_cpid;       // pid of creator
};

//for IPC_INFO
struct shminfo {
    int shmmax;         // max segment size
    int shmmin;         // min segment size
    int shmmni;         // max number of segments
    int shmseg;         // max segments per process
};

//states
#define SHM_FREE        0
#define SHM_ALLOCATING  1
#define SHM_READY       2
#define SHM_DELETED     3

int shmget(int key, int size, int shmflg);
extern struct shm_table shmtable;
void shminit(void);
int shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
int shmctl(int shmid, int cmd, struct shmid_ds *buf);


struct shmseg{
    int state;         
    int key;           // user-provided key going ti be used for lookup
    int id;            // segment id (maybe use of index can be done or else we can have global var where we increment it when allocating segment
    int size;          // size of seg , req for calculation of pages
    int nattch;       // number of processes attached will help us in detemining whther we want to free that segment or not
    int lpid;         // last process that attached
    int cpid;           // creator process id
    char *pages[MAX_PAGES]; // pointers to physical pages which has been done throught kalloc
};

