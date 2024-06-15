#ifndef INC_DELIVERYPERSONELPOOL
#define INC_DELIVERYPERSONELPOOL

#include <DeliveryPersonel.h>
#include <OrderDeque.h>

typedef struct s_Manager t_Manager;

typedef struct s_DeliveryPersonelPool {
  t_DeliveryPersonel *personels;
  int                 personel_count;
  t_OrderDeque       *startDeque;
  t_OrderDeque       *finishedDeque;
  t_Manager          *manager;
} t_DeliveryPersonelPool;

int  t_DeliveryPersonelPool_init(t_DeliveryPersonelPool *pool, int personel_count, int deque_size, int order_cap,
                                 t_OrderDeque *startDeque, t_OrderDeque *finishedDeque, t_Manager *manager);
void t_DeliveryPersonelPool_destroy(t_DeliveryPersonelPool *pool, int *loop, int *is_exit);

int  t_DeliveryPersonelPool_add_order(t_DeliveryPersonelPool *pool, t_Order *order, t_OrderRequestMode mode);
int  t_DeliveryPersonelPool_cancel_order(t_DeliveryPersonelPool *pool, int order_id);
void t_DeliveryPersonelPool_wait(t_DeliveryPersonelPool *pool, int *loop, int *is_exit);
int  t_DeliveryPersonelPool_remaining(t_DeliveryPersonelPool *pool);

#endif /* INC_DELIVERYPERSONELPOOL */
