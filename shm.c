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

struct shm_table shmtable;


// global shared memory table with lock
struct shm_table {
  struct spinlock lock;
  struct shmseg segments[MAXSHM];
};




void
shminit(void)
{
    initlock(&shmtable.lock, "shmtable");
    
    for(int i = 0; i < MAXSHM; i++){
        shmtable.segments[i].state = SHM_FREE;
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
1. Validate size (must be between SHMMIN and SHMMAX)
2. Acquire lock and check if segment with given key already exists
3. If exists:
   - Return error if both IPC_CREAT and IPC_EXCL flags are set
   - Otherwise return existing segment id
4. If not exists and IPC_CREAT not set, return error
5. Find free slot in shmtable and mark as SHM_ALLOCATING
6. Release lock before allocating physical pages (slow operation)
7. Call allocshm() to allocate physical pages
8. On allocation failure, rollback by marking segment as SHM_FREE
9. On success, copy page pointers to segment and mark as SHM_READY
10. Return segment id
*/


int
shmget(int key, int size, int shmflg)
{
    int i;
    char *temp_pages[MAX_PAGES];
    

    if(size < SHMMIN || size > SHMMAX)
        return -1;
    
    acquire(&shmtable.lock);

    


    // Check for existing segment

    if(key != IPC_PRIVATE) {
        for(i = 0; i < MAXSHM; i++) {
            if(shmtable.segments[i].state == SHM_READY && 
               shmtable.segments[i].key == key) {
                if((shmflg & IPC_CREAT) && (shmflg & IPC_EXCL)) {
                    release(&shmtable.lock);
                    return -1; 
                }
                int id = shmtable.segments[i].id;
                release(&shmtable.lock);
                return id;
            }
        }

        if(!(shmflg & IPC_CREAT)) {
            release(&shmtable.lock);
            return -1;  
        }
    }

    for(i = 0; i < MAXSHM; i++) {
        if(shmtable.segments[i].state == SHM_FREE) {
            shmtable.segments[i].state = SHM_ALLOCATING;
            shmtable.segments[i].key = key;
            shmtable.segments[i].id = i;
            shmtable.segments[i].size = size;
            shmtable.segments[i].nattch = 0;

            
            release(&shmtable.lock);
            
            // Allocate pages without lock
            if(allocshm(size, temp_pages) < 0) {

            release(&shmtable.lock);

            
                acquire(&shmtable.lock);
                shmtable.segments[i].state = SHM_FREE;
                release(&shmtable.lock);
                return -1;
            }
            
            // Copy pages and mark READY
            acquire(&shmtable.lock);
            for(int j = 0; j < MAX_PAGES; j++) {
                shmtable.segments[i].pages[j] = temp_pages[j];
            }
            shmtable.segments[i].state = SHM_READY;
            release(&shmtable.lock);
            
            return i;
        }
    }
    
    release(&shmtable.lock);

    return -1;
}



