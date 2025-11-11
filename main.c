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

int main() {
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

    for (int i = 0; i < CONTADORES; i++) mem[i] = 0;

    sem_t *sem_mesada = sem_open(SEM_MESADA, O_CREAT, 0666, 27);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, O_CREAT, 0666, 25);

    if (sem_mesada == SEM_FAILED || sem_heladera == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

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

    int status;
    while (wait(&status) > 0);

    munmap(mem, BUFFER_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    sem_close(sem_mesada);
    sem_close(sem_heladera);
    sem_unlink(SEM_MESADA);
    sem_unlink(SEM_HELADERA);

    return 0;
}
