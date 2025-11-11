#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>

const int CONTADORES = 5;
const int BUFFER_SIZE = sizeof(int) * CONTADORES;
const char *SHM_NAME = "/shared_mem";
const char *SEM_MESADA = "/sem_MESADA";
const char *SEM_HELADERA = "/sem_HELADERA";
const char *SEM_MUTEX = "/sem_MUTEX";

int main() {
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
    // mem[0] = platos en mostrador
    // mem[1] = postres en heladera
    // mem[2] = platos producidos por cocineros (no usado actualmente)
    // mem[3] = postres producidos por repostero (no usado actualmente)
    // mem[4] = estado bandeja (0=libre, no usado con nueva lógica)
    for (int i = 0; i < CONTADORES; i++) {
        mem[i] = 0;
    }

    // Crear semáforos
    // sem_mesada: espacios disponibles en mostrador (inicia en 27)
    // sem_heladera: espacios disponibles en heladera (inicia en 25)
    // sem_mutex: protege acceso a memoria compartida
    sem_t *sem_mesada = sem_open(SEM_MESADA, O_CREAT, 0666, 27);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, O_CREAT, 0666, 25);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);

    if (sem_mesada == SEM_FAILED || sem_heladera == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Crear procesos
    for (int i = 0; i < 3; i++) {
        if (fork() == 0) {
            execl("./cocinero", "./cocinero", NULL);
            perror("execl cocinero");
            exit(1);
        }
    }

    if (fork() == 0) {
        execl("./repostero", "./repostero", NULL);
        perror("execl repostero");
        exit(1);
    }

    for (int i = 0; i < 5; i++) {
        if (fork() == 0) {
            execl("./mozo", "./mozo", NULL);
            perror("execl mozo");
            exit(1);
        }
    }

    // Esperar a que terminen todos los procesos hijos
    int status;
    while (wait(&status) > 0);

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
