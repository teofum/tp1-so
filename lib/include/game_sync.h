#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <semaphore.h>
#include <stdint.h>

typedef struct {
  sem_t view_should_update; // El máster le indica a la vista que hay cambios por imprimir 
  sem_t view_did_update;    // La vista le indica al máster que terminó de imprimir 

  sem_t master_write_mutex; // Mutex para evitar inanición del máster al acceder al estado
  sem_t game_state_mutex;   // Mutex para el estado del juego
  sem_t read_count_mutex;   // Mutex para la siguiente variable 
  uint32_t read_count;      // Cantidad de jugadores leyendo el estado 

  sem_t player_may_move[9]; // Le indican a cada jugador que puede enviar 1 movimiento 
} game_sync_t;

#endif