#ifndef INC_COOKINGPERSONELPOOL
#define INC_COOKINGPERSONELPOOL

#include <CookOven.h>
#include <CookingPersonel.h>
#include <OrderDeque.h>

typedef struct s_CookingPersonelPool {
  t_CookingPersonel *personels;
  int                personel_count;
  t_CookOven        *oven;
  t_OrderDeque       startDeque;
  t_OrderDeque      *finishedDeque;
} t_CookingPersonelPool;

int  t_CookingPersonelPool_init(t_CookingPersonelPool *pool, int personel_count, t_CookOven *oven, int deque_size,
                                t_OrderDeque *finishedDeque);
void t_CookingPersonelPool_destroy(t_CookingPersonelPool *pool);

int  t_CookingPersonelPool_add_order(t_CookingPersonelPool *pool, t_Order *order, t_OrderRequestMode mode);
int  t_CookingPersonelPool_cancel_order(t_CookingPersonelPool *pool, int order_id);
void t_CookingPersonelPool_wait(t_CookingPersonelPool *pool);
int  t_CookingPersonelPool_remaining(t_CookingPersonelPool *pool);
#endif /* INC_COOKINGPERSONELPOOL */
