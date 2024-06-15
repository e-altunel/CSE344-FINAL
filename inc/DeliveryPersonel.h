#ifndef INC_DELIVERYPERSONEL
#define INC_DELIVERYPERSONEL

#include <OrderDeque.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define DELIVERY_PERSONEL_MAX_WAIT_TIME 2

typedef struct s_Manager t_Manager;

typedef struct s_DeliveryPersonel {
  int             id;
  t_OrderDeque   *deque_ref;
  t_OrderDeque   *finishedDeque_ref;
  t_Order        *active_orders;
  int             order_cap;
  int             active_order_count;
  int             is_exit;
  pthread_t       deliver_thread;
  pthread_mutex_t variable_mutex;
  time_t          first_failed_time;
  int             does_failed;
  int             delivered_order_count;
  int             is_cancel;
  t_Manager      *manager;
} t_DeliveryPersonel;

int  t_DeliveryPersonel_init(t_DeliveryPersonel *personel, int id, t_OrderDeque *deque, t_OrderDeque *finishedDeque,
                             int order_cap, t_Manager *manager);
void t_DeliveryPersonel_destroy(t_DeliveryPersonel *personel);
void t_DeliveryPersonel_set_exit(t_DeliveryPersonel *personel);

void *t_DeliveryPersonel_deliver_thread(void *arg);

void t_DeliveryPersonel_deliver(t_DeliveryPersonel *personel);
int  t_DeliveryPersonel_cancel(t_DeliveryPersonel *personel, int order_id);
int  t_DeliveryPersonel_get_active_order_count(t_DeliveryPersonel *personel);
int  t_DeliveryPersonel_get_order(t_DeliveryPersonel *personel);

#endif /* INC_DELIVERYPERSONEL */
