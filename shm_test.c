#include "types.h"
#include "stat.h"
#include "user.h"
#include "shm.h"

int
main(int argc, char *argv[])
{
    int shmid;
    char *addr1, *addr2, *addr3;

    // create a new segment
    shmid = shmget(100, 4096, IPC_CREAT);
    printf(1, "shmget(100, 4096) = %d\n", shmid);

    // attach first mapping
    addr1 = (char*)shmat(shmid, 0, 0);
    printf(1, "first attach: %x\n", addr1);

    // attach second mapping
    addr2 = (char*)shmat(shmid, 0, 0);
    printf(1, "second attach: %x\n", addr2);

    // attach third mapping
    addr3 = (char*)shmat(shmid, 0, 0);
    printf(1, "third attach: %x\n", addr3);

    // write through first mapping
    addr1[0] = 'X';
    addr1[1] = 'Y';
    addr1[2] = 'Z';
    addr1[3] = 0;

    // read through second and third mapping
    printf(1, "second mapping reads: %s\n", addr2);
    printf(1, "third mapping reads: %s\n", addr3);

    // detach all
    shmdt(addr1);
    shmdt(addr2);
    shmdt(addr3);

    exit();
}

