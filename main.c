#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include "constantes.h"

int main() {
    int pedidos_totales;
    
    printf(NEGRITA CYAN "\n==============================================\n" RESET);
    printf(NEGRITA CYAN "SISTEMA DE GESTIÓN DE \"LA ALBONDIGA EMBRUJADA\"\n" RESET);
    printf(NEGRITA CYAN "==============================================\n\n" RESET);
    printf(AMARILLO "Ingrese la cantidad de pedidos a entregar: " RESET);
    scanf("%d", &pedidos_totales);
    printf("\n");

    // Limpiar recursos previos
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_MESADA);
    sem_unlink(SEM_HELADERA);
    sem_unlink(SEM_MUTEX);

    // Crear memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, BUFFER_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    int *mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Inicializar memoria compartida
    mem[0] = 0;
    mem[1] = 0;
    mem[2] = pedidos_totales;
    mem[3] = 0;

    // Crear semáforos
    sem_t *sem_mesada = sem_open(SEM_MESADA, O_CREAT, 0666, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, O_CREAT, 0666, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);

    if (sem_mesada == SEM_FAILED || sem_heladera == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    printf(VERDE "✓ Iniciando procesos...\n\n" RESET);

    // Crear procesos cocineros
    for (int i = 0; i < NUM_COCINEROS; i++) {
        if (fork() == 0) {
            execl("./cocinero", "./cocinero", NULL);
            perror("execl cocinero");
            exit(1);
        }
    }

    // Crear proceso repostero
    if (fork() == 0) {
        execl("./repostero", "./repostero", NULL);
        perror("execl repostero");
        exit(1);
    }

    // Crear procesos mozos
    for (int i = 0; i < NUM_MOZOS; i++) {
        if (fork() == 0) {
            execl("./mozo", "./mozo", NULL);
            perror("execl mozo");
            exit(1);
        }
    }

    // Esperar a que se completen todos los pedidos
    while (1) {
        sem_wait(sem_mutex);
        int restantes = mem[2];
        sem_post(sem_mutex);
        
        if (restantes <= 0) {
            break;
        }
        sleep(1);
    }

    printf(NEGRITA VERDE "\n========================================\n" RESET);
    printf(NEGRITA VERDE "  ¡Todos los pedidos han sido entregados!\n" RESET);
    printf(NEGRITA VERDE "========================================\n" RESET);
    printf(AMARILLO "Esperando que los procesos terminen...\n" RESET);
    
    // Activar bandera de terminación
    sem_wait(sem_mutex);
    mem[3] = 1;
    sem_post(sem_mutex);

    // Esperar a que terminen todos los procesos hijos
    for (int i = 0; i < (NUM_COCINEROS + 1 + NUM_MOZOS); i++) {
        wait(NULL);
    }

    printf(VERDE "✓ Todos los procesos finalizados.\n\n" RESET);

    // Limpiar recursos
    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    sem_close(sem_mesada);
    sem_close(sem_heladera);
    sem_close(sem_mutex);
    sem_unlink(SEM_MESADA);
    sem_unlink(SEM_HELADERA);
    sem_unlink(SEM_MUTEX);

    return 0;
}
