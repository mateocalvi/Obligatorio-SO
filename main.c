#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <signal.h>

const int CONTADORES = 3;
const int BUFFER_SIZE = sizeof(int) * CONTADORES;
const char *SHM_NAME = "/shared_mem";
const char *SEM_MESADA = "/sem_MESADA";
const char *SEM_HELADERA = "/sem_HELADERA";
const char *SEM_MUTEX = "/sem_MUTEX";

pid_t pids[9];
int num_procesos = 0;

int main() {
    int pedidos_totales;
    
    printf("Ingrese la cantidad de pedidos a entregar: ");
    scanf("%d", &pedidos_totales);

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
    // mem[2] = pedidos restantes
    mem[0] = 0;
    mem[1] = 0;
    mem[2] = pedidos_totales;

    // Crear semáforos
    sem_t *sem_mesada = sem_open(SEM_MESADA, O_CREAT, 0666, 0);
    sem_t *sem_heladera = sem_open(SEM_HELADERA, O_CREAT, 0666, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);

    if (sem_mesada == SEM_FAILED || sem_heladera == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Crear procesos cocineros
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            execl("./cocinero", "./cocinero", NULL);
            perror("execl cocinero");
            exit(1);
        }
        pids[num_procesos++] = pid;
    }

    // Crear proceso repostero
    pid_t pid = fork();
    if (pid == 0) {
        execl("./repostero", "./repostero", NULL);
        perror("execl repostero");
        exit(1);
    }
    pids[num_procesos++] = pid;

    // Crear procesos mozos
    for (int i = 0; i < 5; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            execl("./mozo", "./mozo", NULL);
            perror("execl mozo");
            exit(1);
        }
        pids[num_procesos++] = pid;
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

    printf("\n¡Todos los pedidos han sido entregados!\n");
    printf("Finalizando procesos...\n");

    // Terminar todos los procesos hijos
    for (int i = 0; i < num_procesos; i++) {
        kill(pids[i], SIGTERM);
    }

    // Esperar a que terminen
    sleep(1);
    for (int i = 0; i < num_procesos; i++) {
        waitpid(pids[i], NULL, WNOHANG);
    }

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
