#include "types.h"
#include "stat.h"
#include "user.h"
#include "shm.h"

int
main(int argc, char *argv[])
{
    int id1, id2, id3;
    char *addr1, *addr2;
    int i;

    // basic shmget test
    id1 = shmget(42, 4096, IPC_CREAT);
    printf(1, "shmget(42, 4096): %d\n", id1);
    
    // attach it
    addr1 = (char*)shmat(id1, 0, 0);
    printf(1, "shmat: %x\n", addr1);
    
    // write something
    addr1[0] = 'A';
    addr1[1] = 'B';
    addr1[2] = 'C';
    addr1[3] = 0;
    printf(1, "wrote: %s\n", addr1);

    // same key returns same id
    id2 = shmget(42, 4096, 0);
    printf(1, "shmget same key: %d\n", id2);

    // attach again
    addr2 = (char*)shmat(id1, 0, 0);
    printf(1, "shmat again: %x, data=%s\n", addr2, addr2);

    // detach first one
    if(shmdt(addr1) == 0)
        printf(1, "shmdt ok\n");

    // detach second
    shmdt(addr2);

    // IPC_EXCL should fail now... wait no, already exists
    id3 = shmget(42, 4096, IPC_CREAT | IPC_EXCL);
    printf(1, "IPC_EXCL on existing: %d\n", id3);

    // new segment
    int id_new = shmget(99, 8192, IPC_CREAT);
    printf(1, "new segment: %d\n", id_new);

    // IPC_PRIVATE
    int priv1 = shmget(IPC_PRIVATE, 4096, IPC_CREAT);
    int priv2 = shmget(IPC_PRIVATE, 4096, IPC_CREAT);
    printf(1, "private segs: %d, %d\n", priv1, priv2);

    // bad sizes
    printf(1, "zero size: %d\n", shmget(50, 0, IPC_CREAT));
    printf(1, "huge size: %d\n", shmget(51, 10*1024*1024, IPC_CREAT));

    // bad detach
    if(shmdt((void*)0x999999) < 0)
        printf(1, "bad detach failed correctly\n");

    // readonly attach
    int ro_id = shmget(123, 4096, IPC_CREAT);
    char *ro_addr = (char*)shmat(ro_id, 0, SHM_RDONLY);
    printf(1, "readonly attach: %x\n", ro_addr);
    shmdt(ro_addr);

    // fill up the table
    printf(1, "filling table...\n");
    for(i = 0; i < 70; i++) {
        int x = shmget(200+i, 4096, IPC_CREAT);
        if(x < 0) {
            printf(1, "table full at %d segments\n", i);
            break;
        }
    }

    printf(1, "done\n");
    exit();
}