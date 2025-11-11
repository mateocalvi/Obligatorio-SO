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
const char *SEM_MUTEX = "/sem_MUTEX";

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    int *mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

    srand(getpid());

    while (1) {
        // Decidir qué retirar (alternar entre platos y postres)
        int retirar_plato = rand() % 2;

        if (retirar_plato) {
            // Intentar retirar plato
            sem_wait(sem_mutex);
            if (mem[0] > 0) {
                mem[0]--;
                printf("Mozo %d: retiró un plato. Platos en mostrador: %d\n", getpid(), mem[0]);
                sem_post(sem_mutex);
                
                // Señalar espacio disponible en mostrador
                sem_post(sem_mesada);
                
                // Simular tiempo de entrega
                sleep(rand() % 3 + 1);
            } else {
                sem_post(sem_mutex);
                usleep(100000); // Esperar 0.1 segundos antes de reintentar
            }
        } else {
            // Intentar retirar postre
            sem_wait(sem_mutex);
            if (mem[1] > 0) {
                mem[1]--;
                printf("Mozo %d: retiró un postre. Postres en heladera: %d\n", getpid(), mem[1]);
                sem_post(sem_mutex);
                
                // Señalar espacio disponible en heladera
                sem_post(sem_heladera);
                
                // Simular tiempo de entrega
                sleep(rand() % 3 + 1);
            } else {
                sem_post(sem_mutex);
                usleep(100000); // Esperar 0.1 segundos antes de reintentar
            }
        }
    }

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    sem_close(sem_heladera);
    sem_close(sem_mutex);
    return 0;
}
