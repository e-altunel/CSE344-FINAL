#ifndef INC_MANAGER
#define INC_MANAGER

#include <CookingPersonelPool.h>
#include <DeliveryPersonelPool.h>
#include <OrderDeque.h>
#include <pthread.h>

extern int loop;

typedef struct s_Manager {
  t_CookingPersonelPool  cookingPersonelPool;
  t_DeliveryPersonelPool deliveryPersonelPool;
  t_OrderDeque           sharedDeque;
  t_CookOven             oven;
  pthread_t              managerThread;
  int                    is_exit;
  t_OrderDeque          *startDeque;
  t_OrderDeque          *finishDeque;
  int                   *loop;
  int                   *is_cancel;
  int                    p;
  int                    q;
  int                    k;
} t_Manager;

int   t_Manager_init(t_Manager *manager, int cooking_personel_count, int delivery_personel_count,
                     t_OrderDeque *startDeque, t_OrderDeque *finishDeque, int *loop, int *is_cancel, int p, int q, int k);
void  t_Manager_destroy(t_Manager *manager);
void *t_Manager_thread(void *arg);
void  t_Manager_cancel_all(t_Manager *manager);

#endif /* INC_MANAGER */
