#ifndef INC_COOKOVEN
#define INC_COOKOVEN

#include <semaphore.h>

typedef struct s_CookOven {
  sem_t available_aparatus;
  sem_t available_slots;
  sem_t first_door;
  sem_t second_door;
} t_CookOven;

int  t_CookOven_init(t_CookOven *oven, unsigned int aparatus, unsigned int slots);
void t_CookOven_destroy(t_CookOven *oven);
int  t_CookOven_insert(t_CookOven *oven);
int  t_CookOven_remove(t_CookOven *oven);
int  t_CookOven_cancel(t_CookOven *oven);

#endif /* INC_COOKOVEN */
