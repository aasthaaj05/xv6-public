#include "types.h"
#include "stat.h"
#include "user.h"
#include "shm.h"

int main(int argc, char *argv[]){
    int shmid1 = shmget(42, 4096);
    int shmid2 = shmget(42, 4096);
    int shmid3 = shmget(43, 8192);

    printf(1, "shmid1 = %d\n", shmid1);
    printf(1, "shmid2 = %d\n", shmid2);
    printf(1, "shmid3 = %d\n", shmid3);

    exit();
}

