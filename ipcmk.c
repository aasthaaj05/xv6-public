#include "types.h"
#include "stat.h"
#include "user.h"
#include "shm.h"

int main(int argc, char *argv[])
{
    int size = 4096; 
    int shmid;
    int key;
    
    if(argc < 2) {
        printf(2, "Usage: ipcmk -M <size>\n");
        exit();
    }
    
    if(strcmp(argv[1], "-M") == 0) {
        if(argc == 3) {
            size = atoi(argv[2]);
        }
    } else {
        printf(2, "Usage: ipcmk -M <size>\n");
        exit();
    }
    
    // Generate a somewhat random key using PID and uptime
    key = getpid() * 100 + uptime();
    
    shmid = shmget(key, size, IPC_CREAT | IPC_EXCL);
    
    if(shmid < 0) {
        printf(2, "ipcmk failed\n");
        exit();
    }
    
    printf(1, "shared memory id: %d\n", shmid);
    printf(1, "key: %d, size: %d bytes\n", key, size);
    
    exit();
}