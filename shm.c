#include "types.h"     
#include "defs.h"     
#include "param.h"     
#include "mmu.h"
#include "memlayout.h" 
#include "proc.h"
#include "spinlock.h"
#include "shm.h"


#define MAX_PAGES 16

/*

struct shmseg{
    int used;          // 1 if segment is in use helping in finding free slots
    int key;           // user-provided key going ti be used for lookup
    int id;            // segment id (maybe use of index can be done or else we can have global var where we increment it when allocating segment
    int size;          // size of seg , req for calculation of pages
    int nattch;        // number of processes attached will help us in detemining whther we want to free that segment or not
    char *pages[MAX_PAGES]; // pointers to physical pages which has been done throught kalloc
};

*/

// global shared memory table with lock
struct {
    struct spinlock lock;
    struct shmseg segments[MAXSHM];
} shmtable;


void
shminit(void)
{
    initlock(&shmtable.lock, "shmtable");
    
    for(int i = 0; i < MAXSHM; i++){
        shmtable.segments[i].used = 0;
        shmtable.segments[i].key = 0;
        shmtable.segments[i].id = i;
        shmtable.segments[i].size = 0;
        shmtable.segments[i].nattch = 0;
        for(int j = 0; j < MAX_PAGES; j++){
            shmtable.segments[i].pages[j] = 0;
        }
    }
}

/*
sys_shmget:
1. acquire lock
2. check if segment with given key exists
3. if exists, return its id
4. error if IPC_CREAT or IPC_EXCL flags are set and segment exists
5. if size requested is less than SHMMIN or greater than SHMMAX, return error
6. IPC_CREAT flag set, find a free slot in shmtable
7. for non-existing segment, initialize its fields
8. allocate physical pages for the segment. store pointers in pages array
9. initilialize shared memory metadata
10. release lock
11. return segment id or error code
*/

int
shmget(int key, int size, int shmflg)
{
     int i;

    if(size < SHMMIN || size > SHMMAX)
        return -1;
    
    acquire(&shmtable.lock);

    //check for existing segment
    if(key != IPC_PRIVATE) {
        for(i = 0; i < MAXSHM; i++) {
            if(shmtable.segments[i].used && shmtable.segments[i].key == key) {
                if((shmflg & IPC_CREAT) && (shmflg & IPC_EXCL)) {
                    release(&shmtable.lock);
                    return -1; 
                }
                int id = shmtable.segments[i].id;
                release(&shmtable.lock);
                return id;
            }
        }
        
        //segment not found, check if IPC_CREAT is set
        if(!(shmflg & IPC_CREAT)) {
            release(&shmtable.lock);
            return -1;  
        }
    }

    //find free slot for new segment
    for(i = 0; i < MAXSHM; i++) {
        if(!shmtable.segments[i].used) {
            shmtable.segments[i].used = 1;
            shmtable.segments[i].key = key;
            shmtable.segments[i].id = i;
            shmtable.segments[i].size = size;
            shmtable.segments[i].nattch = 0;

            release(&shmtable.lock);  //release BEFORE slow allocation

            //allocate physical pages
            if(allocshm(size, shmtable.segments[i].pages) < 0) {
                acquire(&shmtable.lock);
                shmtable.segments[i].used = 0;
                release(&shmtable.lock);
                return -1;  
            }

            return i;
        }
    }

    release(&shmtable.lock);
    return -1;  
}