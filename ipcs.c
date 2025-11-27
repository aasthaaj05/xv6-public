#include "types.h"
#include "stat.h"
#include "user.h"
#include "shm.h"

int main(int argc, char *argv[])
{
    struct shmid_ds buf;
    int i;
    
    printf(1, "%-10s %-10s %-10s %-10s %-10s\n", "shmid", "key", "size", "nattch", "cpid");
    
    // Query each possible segment ID
    for(i = 0; i < 64; i++) {  // MAXSHM = 64
        if(shmctl(i, IPC_STAT, &buf) == 0) {
            printf(1, "%-10d %-10d %-10d %-10d %-10d\n",
                   i, i, buf.shm_segsz, buf.shm_nattch, buf.shm_cpid);
        }
    }
    
    // Display system limits
    if(shmctl(0, IPC_INFO, &buf) == 0) {
        printf(1, "max seg size: %d bytes\n", buf.shm_segsz);
        printf(1, "min seg size: %d bytes\n", buf.shm_nattch);
        printf(1, "max segments: %d\n", buf.shm_lpid);
    }
    
    exit();
}