#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

const int CONTADORES = 5;
const int BUFFER_SIZE = sizeof(int) * CONTADORES;
const char *SHM_NAME = "/shared_mem";
const char *SEM_MESADA = "/sem_MESADA";
const char *SEM_HELADERA = "/sem_HELADERA";

const int BANDEJA_PLATOS = 1;
const int BANDEJA_POSTRES = 2;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    int *mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);

    int cantidad = 0;
    srand(getpid());

    while (1) {
        if (mem[4] == 0) {
            // Bandeja libre, decidir qué retirar según disponibilidad
            if (mem[0] > 0) mem[4] = BANDEJA_PLATOS;
            else if (mem[1] > 0) mem[4] = BANDEJA_POSTRES;
            else {
                sleep(rand() % 3 + 1);
                continue;
            }
        }

        if (mem[4] == BANDEJA_PLATOS) {
            sem_wait(sem_mesada);
            if (mem[0] > 0) {
                mem[0]--;
                printf("Mozo retiró un plato. Platos en mostrador: %d\n", mem[0]);
            }
            mem[4] = 0;
            sem_post(sem_mesada);
        } else if (mem[4] == BANDEJA_POSTRES) {
            sem_wait(sem_heladera);
            if (mem[1] > 0) {
                mem[1]--;
                printf("Mozo retiró un postre. Postres en heladera: %d\n", mem[1]);
            }
            mem[4] = 0;
            sem_post(sem_heladera);
        }
        sleep(rand() % 3 + 1);
    }

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    sem_close(sem_heladera);
    return 0;
}
