#ifndef INC_ORDERDEQUE
#define INC_ORDERDEQUE

#include <Order.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct s_OrderDeque {
  t_Order        *orders;
  unsigned int    head;
  unsigned int    tail;
  unsigned int    size;
  pthread_mutex_t mutex;
  sem_t           empty_slots;
  sem_t           full_slots;
} t_OrderDeque;

int  t_OrderDeque_init(t_OrderDeque *deque, unsigned int size);
void t_OrderDeque_destroy(t_OrderDeque *deque);
int  t_OrderDeque_enqueue(t_OrderDeque *deque, t_Order *order);
int  t_OrderDeque_dequeue(t_OrderDeque *deque, t_Order *order);

#endif /* INC_ORDERDEQUE */
