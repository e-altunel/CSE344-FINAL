#ifndef INC_COOKINGPERSONEL
#define INC_COOKINGPERSONEL

#include <CookOven.h>
#include <Order.h>
#include <OrderDeque.h>
#include <pthread.h>

typedef struct s_CookingPersonel {
  t_CookOven     *oven_ref;
  t_Order        *active_order;
  t_Order        *pending_order;
  pthread_t       prepare_thread;
  pthread_t       cook_thread;
  pthread_mutex_t mutex;
  int             is_exit;
} t_CookingPersonel;

int  t_CookingPersonel_init(t_CookingPersonel *personel, t_CookOven *oven);
void t_CookingPersonel_destroy(t_CookingPersonel *personel);

void t_CookingPersonel_take_order(t_CookingPersonel *personel, t_OrderDeque *deque);
void t_CookingPersonel_prepare_order(t_CookingPersonel *personel);
void t_CookingPersonel_cook_order(t_CookingPersonel *personel);

#endif /* INC_COOKINGPERSONEL */
