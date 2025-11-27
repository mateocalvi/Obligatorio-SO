#ifndef CONSTANTES_H
#define CONSTANTES_H

// Constantes para nombres de semáforos
#define SEM_MESADA     "/sem_mesada"
#define SEM_HELADERA   "/sem_heladera"
#define SEM_MUTEX      "/sem_mutex"
#define SEM_BARRERA    "/sem_barrera"

#define NUM_COCINEROS     2
#define NUM_MOZOS         3
#define NUM_REPOSTEROS    1

// Total procesos hijos: cocineros + mozos + reposteros
#define TOTAL_PROCESOS (NUM_COCINEROS + NUM_MOZOS + NUM_REPOSTEROS)

// Máximo platos que puede tener el mostrador (mesada)
#define MAX_PLATOS_MOSTRADOR 5

// Máximo postres que puede tener la heladera
#define MAX_POSTRES_HELADERA 5

#endif // CONSTANTES_H
