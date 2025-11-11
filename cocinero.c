#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

const int CONTADORES = 5;
const int BUFFER_SIZE = sizeof(int) * CONTADORES;
const char *SHM_NAME = "/sharedmem";
const char *SEM_MESADA = "/semMESADA";

const int MAX_PROD_PLATOS = 4;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    int *mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);

    while (1) {
        sem_wait(sem_mesada); // Esperar espacio disponible
        while (mem[2] >= MAX_PROD_PLATOS) {
            sleep(1);
        }
        mem[2]++;       // prodplatos
        mem[0]++;       // platos
        mem[4] = 1;     // estado_bandeja = platos
        printf("Cocinero: generó un plato. Platos en mostrador: %d\n", mem[0]);
        sleep(rand() % 3 + 1);
        sem_post(sem_mesada); // Señalar producción
    }

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    return 0;
}
