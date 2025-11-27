#include "types.h"
#include "stat.h"
#include "user.h"
#include "shm.h"

int main(int argc, char *argv[])
{
    int shmid;
    
    if(argc != 3) {
        printf(2, "Usage: ipcrm -m <shmid>\n");
        exit();
    }
    
    if(strcmp(argv[1], "-m") != 0) {
        printf(2, "Usage: ipcrm -m <shmid>\n");
        exit();
    }
    
    shmid = atoi(argv[2]);
    
    if(shmctl(shmid, IPC_RMID, 0) < 0) {
        printf(2, "ipcrm: couldn't remove shmid %d\n", shmid);
        exit();
    }
    
    printf(1, "removed shared memory segment id: %d\n", shmid);
    exit();
}