#include "types.h"     
#include "defs.h"     
#include "param.h"     
#include "mmu.h"
#include "memlayout.h" 
#include "proc.h"
#include "spinlock.h"
#include "shm.h"

// global shared memory table with lock
struct shm_table {
  struct spinlock lock;
  struct shmseg segments[MAXSHM];
};


struct shm_table shmtable;

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
        shmtable.segments[i].lpid = 0;
        shmtable.segments[i].cpid = 0; 
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

    if(key != IPC_PRIVATE){
        for(i = 0; i < MAXSHM; i++)   {
            if(shmtable.segments[i].state == SHM_READY && 
               shmtable.segments[i].key == key)  {
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
            shmtable.segments[i].cpid = myproc()->pid;
            
            release(&shmtable.lock);
       
            if(allocshm(size, temp_pages) < 0) {
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

int
shmat(int shmid, const void *shmaddr, int shmflag)
{
    struct shmseg *seg;
    struct proc *curproc = myproc();
    char *attach_addr = 0;

    if (shmid < 0 || shmid >= MAXSHM) {
        return -1;
	}
   
    acquire(&shmtable.lock); //lock for accessing table
    seg = &shmtable.segments[shmid];

  
    if (seg->state != SHM_READY) {
        release(&shmtable.lock);
        return -1;
    }
	//int remap;
	int perm;
            
	if (shmflag & SHM_RDONLY) {
    perm = PTE_U;
	} else {
    perm = PTE_W | PTE_U;
	}


	/*if (shmflag & SHM_REMAP) {
	    remap = 1;
	} else {
  	  remap = 0;
	}*/
      
    release(&shmtable.lock);

    if (shmaddr == 0) {
        attach_addr = findfree_vm_region(curproc->pgdir, seg->size);
        if (attach_addr == 0)
            return -1; 
    } else {
        attach_addr = (char *)PGROUNDDOWN((uint)shmaddr);
    }

   
    if (mapshm(curproc->pgdir, seg->size, seg->pages, attach_addr, perm) < 0) {
        return -1;
        }

  
    acquire(&shmtable.lock);
    seg->nattch++;
    seg->lpid = curproc->pid;
    release(&shmtable.lock);

   
    return (int)attach_addr;
}

/*
1. Validate that shmaddr is page-aligned
2. Use check_shmaddr to verify the address is mapped
3. Find which segment this address belongs to by checking all segments
4. Unmap the pages from the process's address space
5. Decrement nattch counter
6. Update lpid to current process
*/
int
shmdt(const void *shmaddr)
{
    struct proc *curproc = myproc();
    char *addr = (char *)shmaddr;
    struct shmseg *seg = 0;
    int i;
    
    //if address is page-aligned
    if((uint)addr % PGSIZE != 0) 
        return -1;
    
    //address is actually mapped
    if(check_shmaddr(curproc->pgdir, addr) == 0) 
        return -1;
    
    //find segment 
    acquire(&shmtable.lock);
    
    for(i = 0; i < MAXSHM; i++) {
        if(shmtable.segments[i].state == SHM_READY || shmtable.segments[i].state == SHM_DELETED) {
            seg = &shmtable.segments[i];
            
            pte_t *pte = walkpgdir(curproc->pgdir, addr, 0);
            if(pte && (*pte & PTE_P)) {
                char *pa = (char*)P2V(PTE_ADDR(*pte));
                if(pa == seg->pages[0]) {
                    break;
                }
            }
        }
        seg = 0;
    }
    
    if(seg == 0) {
        release(&shmtable.lock);
        return -1;
    }
    
    //segment info before unmapping
    uint seg_size = seg->size;
    
    //--attachment count
    if(seg->nattch > 0) {
        seg->nattch--;
    }
    seg->lpid = curproc->pid;

    if(seg->state == SHM_DELETED && seg->nattch == 0) {
        deallocshm(seg->pages, seg->size);
        seg->state = SHM_FREE;
        seg->key = 0;
        seg->size = 0;
    }
    
    release(&shmtable.lock);
    
    //unmap the shm region
    char *end = addr + PGROUNDUP(seg_size);
    if(unmapshm(curproc->pgdir, addr, end) < 0) return -1;
    
    return 0;
}

/*
  IPC_STAT  - Copy segment info into buf
  IPC_RMID  - Mark segment for deletion (actual deletion when nattch=0)
  IPC_INFO  - Get system limits
*/
int
shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
    struct shmseg *seg;
    
    if(shmid < 0 || shmid >= MAXSHM) return -1;
    
    acquire(&shmtable.lock);
    seg = &shmtable.segments[shmid];
    
    if(seg->state != SHM_READY && seg->state != SHM_DELETED) {
        release(&shmtable.lock);
        return -1;
    }
    
    switch(cmd) {
        case IPC_STAT:
            if(buf == 0) {
                release(&shmtable.lock);
                return -1;
            }
            buf->shm_segsz = seg->size;
            buf->shm_nattch = seg->nattch;
            buf->shm_lpid = seg->lpid;
            buf->shm_cpid = seg->cpid;
            release(&shmtable.lock);
            return 0;
            
        case IPC_RMID:
            //mark segment for removal
            //if no processes attached, free immediately
            //else, free when last process detaches
            if(seg->nattch == 0) {
                deallocshm(seg->pages, seg->size);
                seg->state = SHM_FREE;
                seg->key = 0;
                seg->size = 0;
                release(&shmtable.lock);
            } else {
                seg->state = SHM_DELETED;
                release(&shmtable.lock);
            }
            return 0;
            
        case IPC_INFO:
            release(&shmtable.lock);
            if(buf == 0) return -1;
            buf->shm_segsz = SHMMAX;
            buf->shm_nattch = SHMMIN;
            buf->shm_lpid = MAXSHM;
            buf->shm_cpid = SHMSEG;
            return 0;
            
        default:
            release(&shmtable.lock);
            return -1;
    }
}