#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

const int BUFFER_SIZE = sizeof(int)*2;
const char* SHM_NAME = "/shared_mem";
const char* SEM_MESADA = "/sem_MESADA";

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
    p.tipo = PLATO;  // Para cocinero, siempre platos
    p.cantidad = (rand() % max_cant) + 1;
    return p;
}

int main() {
    srand(time(NULL)); // ^ getpid()

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_fd == -1) { perror("shm_open cocinero"); exit(1); }
    int *stock = mmap(0, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (stock == MAP_FAILED) { perror("mmap cocinero"); exit(1); }

    sem_t *sem_mesada = sem_open(SEM_MESADA, 0);
    if (sem_mesada == SEM_FAILED) { perror("sem_open cocinero"); exit(1); }

    while(1) {
        Pedido p = generar_pedido_random(27);

        sem_wait(sem_mesada);

        // Verificación simple de cantidad
        if (p.cantidad >=1 && p.cantidad <=27) {
            stock[0] += p.cantidad;
            printf("Cocinero: generó pedido de %d platos. Platos en mostrador: %d\n", p.cantidad, stock[0]);
        } else {
            printf("Cocinero: pedido inválido\n");
        }
        sleep(rand() % 3 + 1);
    }
    munmap(stock, BUFFER_SIZE);
    close(shm_fd);
    sem_close(sem_mesada);
    return 0;
}
