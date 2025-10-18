#include "types.h"     
#include "defs.h"     
#include "param.h"     
#include "memlayout.h" 
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

// global shared memory table
struct shmseg shmtable[MAXSHM];


void shminit(void){
    for(int i = 0; i < MAXSHM; i++){
        shmtable[i].used = 0;
        shmtable[i].key = 0;
        shmtable[i].id = 0;
        shmtable[i].size = 0;
        shmtable[i].nattch = 0;
        for(int j = 0; j < MAX_PAGES; j++){
        
            shmtable[i].pages[j] = 0;
            }
    }
}

int shmget(int key, int size){
    int i;

     /* first we wil check for segment and if its there then we return that */
    for (i = 0; i < MAXSHM; i++) {
        if(shmtable[i].used && shmtable[i].key == key){
           
            return shmtable[i].id;
          }
       }

    //if not then we just allocate with the arguments provided
    for (i = 0; i < MAXSHM; i++){
        if (!shmtable[i].used) {
            
        shmtable[i].used = 1;
        shmtable[i].key = key;
        shmtable[i].id = i;     // could use a global counter for unique but for not it local 
        shmtable[i].size = size;
        shmtable[i].nattch = 0;

            //have to figure out pages part
            
             cprintf("key=%d, index=%d, id=%d\n", key, i, shmtable[i].id);
            return shmtable[i].id;
        }
    }

    //and if full then -1
    return -1;
}

