#include "types.h"
#include "stat.h"
#include "user.h"
#include "shm.h"


#define SHMMIN 1
int
main(int argc, char *argv[])
{
    int id1, id2, id3, id4, id5, id6;
    int i;


    // Basic creation
    id1 = shmget(10, 4096, IPC_CREAT);
    printf(1, "Test 1: shmget(10, 4096, IPC_CREAT):id=%d\n", id1);

    //Same key should return same id (no new allocation)
    id2 = shmget(10, 4096, IPC_CREAT);
    printf(1, "Test 2: shmget(10, 4096, IPC_CREAT) again:id=%d\n", id2);

    //Existing key + IPC_EXCL should fail
    id3 = shmget(10, 4096, IPC_CREAT | IPC_EXCL);
    printf(1, "Test 3: shmget(10, 4096, IPC_CREAT|IPC_EXCL):id=%d\n", id3);

    //Create new key
    id4 = shmget(20, 8192, IPC_CREAT);
    printf(1, "Test 4: shmget(20, 8192, IPC_CREAT):id=%d\n", id4);

    //IPC_PRIVATE (unique segment)
    id5 = shmget(IPC_PRIVATE, 4096, IPC_CREAT);
    printf(1, "Test 5: shmget(IPC_PRIVATE, 4096, IPC_CREAT):id=%d\n", id5);

    //Invalid size tests
    id6 = shmget(30, 0, IPC_CREAT);
    printf(1, "Test 6a: shmget(30, 0, IPC_CREAT):id=%d\n", id6);
    id6 = shmget(31, SHMMAX * 2, IPC_CREAT);
    printf(1, "Test 6b: shmget(31, SHMMAX*2, IPC_CREAT):id=%d\n", id6);
    
    int overflow = shmget(999, 4096, IPC_CREAT);
    printf(1, "shmget(key=999) after table full :id=%d\n", overflow);

    //Fill table completely
    printf(1, "\nTest 7: Filling shm table completely...\n");
    for (i = 0; i < 65; i++) {
        int ret = shmget(100 + i, 4096, IPC_CREAT);
        printf(1, "  shmget(100+%d):id=%d\n", i, ret);
    }

    //Try one more beyond limit
    int overflow1 = shmget(200, 4096, IPC_CREAT);
    printf(1, "Test 8: shmget(200, 4096, IPC_CREAT) after full table:id=%d\n", overflow1);

    exit();
}

