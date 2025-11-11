#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

const int BUFFER_SIZE = sizeof(int)*2;
const char* SHM_NAME = "/shared_mem";
const char* SEM_MESADA = "/sem_MESADA";
const char* SEM_HELADERA = "/sem_HELADERA";

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_fd == -1) { perror("shm_open mozo"); exit(1); }
    int *stock = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (stock == MAP_FAILED) { perror("mmap mozo"); exit(1); }

    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);
    if (sem_mesada == SEM_FAILED || sem_heladera == SEM_FAILED) {
        perror("sem_open mozo");
        exit(1);
    }

    while (1) {
        // Simular retirar platos o postres (alternando)
        if (stock[0] > 0) {
            stock[0]--;
            printf("Mozo: retiró un plato. Platos en mostrador: %d\n", stock[0]);
            sem_post(sem_mesada); // libera lugar para que cocinero produzca
        } else if (stock[1] > 0) {
            stock[1]--;
            printf("Mozo: retiró un postre. Postres en heladera: %d\n", stock[1]);
            sem_post(sem_heladera); // libera lugar para repostero
        } else {
            // No hay platos ni postres, espera un poco
            sleep(1);
            continue;
        }
        sleep(rand() % 3 + 1);
    }

    munmap(stock, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    sem_close(sem_heladera);
    return 0;
}
