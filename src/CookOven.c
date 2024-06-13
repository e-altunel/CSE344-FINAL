#include <CookOven.h>
#include <stdio.h>

int t_CookOven_init(t_CookOven *oven, unsigned int aparatus, unsigned int slots) {
  if (oven == 0 || aparatus == 0 || slots == 0)
    return -1;

  if (sem_init(&oven->available_aparatus, 0, aparatus) == -1)
    return -1;

  if (sem_init(&oven->available_slots, 0, slots) == -1) {
    sem_destroy(&oven->available_aparatus);
    return -1;
  }

  if (sem_init(&oven->first_door, 0, 1) == -1) {
    sem_destroy(&oven->available_aparatus);
    sem_destroy(&oven->available_slots);
    return -1;
  }

  if (sem_init(&oven->second_door, 0, 1) == -1) {
    sem_destroy(&oven->available_aparatus);
    sem_destroy(&oven->available_slots);
    sem_destroy(&oven->first_door);
    return -1;
  }

  return 0;
}

void t_CookOven_destroy(t_CookOven *oven) {
  if (oven == 0)
    return;

  sem_destroy(&oven->available_aparatus);
  sem_destroy(&oven->available_slots);
  sem_destroy(&oven->first_door);
  sem_destroy(&oven->second_door);
}

int t_CookOven_insert(t_CookOven *oven) {
  sem_wait(&oven->available_slots);
  sem_wait(&oven->available_aparatus);
  sem_wait(&oven->first_door);
  printf("Inserting\n");
  sem_post(&oven->first_door);
  sem_post(&oven->available_aparatus);
}

int t_CookOven_remove(t_CookOven *oven) {
  sem_wait(&oven->available_aparatus);
  sem_wait(&oven->second_door);
  sem_post(&oven->second_door);
  printf("Removing\n");
  sem_post(&oven->available_aparatus);
  sem_post(&oven->available_slots);
}

int t_CookOven_cancel(t_CookOven *oven) {
  sem_wait(&oven->available_aparatus);
  sem_wait(&oven->second_door);
  sem_post(&oven->available_slots);
  sem_post(&oven->second_door);
  sem_post(&oven->available_aparatus);
}