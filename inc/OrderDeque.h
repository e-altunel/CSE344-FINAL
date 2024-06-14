#ifndef INC_ORDERDEQUE
#define INC_ORDERDEQUE

#include <Order.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct s_OrderDeque {
  t_Order        *orders;
  int             head;
  int             tail;
  int             size;
  pthread_mutex_t mutex;
  sem_t           empty_slots;
  sem_t           full_slots;
} t_OrderDeque;

typedef enum e_OrderDequeStatus {
  ORDER_DEQUE_OK,
  ORDER_DEQUE_ERROR
} t_OrderDequeStatus;

typedef enum e_OrderRequestMode {
  ORDER_REQUEST_MODE_BLOCKING,
  ORDER_REQUEST_MODE_NON_BLOCKING
} t_OrderRequestMode;

int  t_OrderDeque_init(t_OrderDeque *deque, unsigned int size);
void t_OrderDeque_destroy(t_OrderDeque *deque);
int  t_OrderDeque_enqueue(t_OrderDeque *deque, t_Order *order, t_OrderRequestMode mode);
int  t_OrderDeque_dequeue(t_OrderDeque *deque, t_Order **order, t_OrderRequestMode mode);
int  t_OrderDeque_size(t_OrderDeque *deque);
int  t_OrderDeque_cancel(t_OrderDeque *deque, int order_id);
int  t_OrderDeque_clear_without_lock(t_OrderDeque *deque);

#endif /* INC_ORDERDEQUE */
