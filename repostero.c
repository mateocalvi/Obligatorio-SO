#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

const int BUFFER_SIZE = sizeof(int)*2;
const char* SHM_NAME = "/shared_mem";
const char* SEM_HELADERA = "/sem_HELADERA";

typedef enum {
    PLATO = 1,
    POSTRE = 2
} TipoPedido;

typedef struct {
    TipoPedido tipo;
    int cantidad;
} Pedido;

Pedido generar_pedido_random(int max_cant) {
    Pedido p;
    p.tipo = POSTRE;  // Para repostero, siempre postres
    p.cantidad = (rand() % max_cant) + 1;
    return p;
}

int main() {
    srand(time(NULL)); // ^ getpid()

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_fd == -1) { perror("shm_open repostero"); exit(1); }
    int *stock = mmap(0, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (stock == MAP_FAILED) { perror("mmap repostero"); exit(1); }

    sem_t *sem_heladera = sem_open(SEM_HELADERA, 0);
    if (sem_heladera == SEM_FAILED) { perror("sem_open repostero"); exit(1); }

    while(1) {
        Pedido p = generar_pedido_random(25);

        sem_wait(sem_heladera);

        if (p.cantidad >=1 && p.cantidad <= 25) {
            stock[1] += p.cantidad;
            printf("Repostero: generó pedido de %d postres. Postres en heladera: %d\n", p.cantidad, stock[1]);
        } else {
            printf("Repostero: pedido inválido\n");
        }
        sleep(rand() % 3 + 1);
    }
    munmap(stock, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_heladera);
    return 0;
}
