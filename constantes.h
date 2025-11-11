#ifndef CONSTANTES_H
#define CONSTANTES_H

// Configuración de memoria compartida
const int CONTADORES = 4;
const int BUFFER_SIZE = sizeof(int) * CONTADORES;

// Nombres de recursos IPC
const char *SHM_NAME = "/shared_mem";
const char *SEM_MESADA = "/sem_MESADA";
const char *SEM_HELADERA = "/sem_HELADERA";
const char *SEM_MUTEX = "/sem_MUTEX";
const char *SEM_BARRERA = "/sem_BARRERA";  // NUEVA LÍNEA

// Capacidades máximas
const int MAX_PLATOS_MOSTRADOR = 27;
const int MAX_POSTRES_HELADERA = 25;

// Cantidad de procesos
const int NUM_COCINEROS = 3;
const int NUM_MOZOS = 5;
const int TOTAL_PROCESOS = 9;  // 3 cocineros + 1 repostero + 5 mozos

// Colores para prints de consola
#define RESET   "\e[0m"
#define ROJO    "\e[31m"
#define VERDE   "\e[32m"
#define AMARILLO "\e[33m"
#define AZUL    "\e[34m"
#define MAGENTA "\e[35m"
#define CYAN    "\e[36m"
#define NEGRITA "\e[1m"

#endif // CONSTANTES_H
