#include <OrderDeque.h>

int t_OrderDeque_init(t_OrderDeque *deque, unsigned int size) {
  if (deque == 0)
    return -1;

  if (size == 0)
    return -1;

  deque->orders = (t_Order *)malloc(size * sizeof(t_Order));
  if (deque->orders == 0)
    return -1;

  deque->head = 0;
  deque->tail = 0;
  deque->size = size;

  if (pthread_mutex_init(&deque->mutex, 0) != 0) {
    free(deque->orders);
    return -1;
  }

  if (sem_init(&deque->empty_slots, 0, size) != 0) {
    pthread_mutex_destroy(&deque->mutex);
    free(deque->orders);
    return -1;
  }

  if (sem_init(&deque->full_slots, 0, 0) != 0) {
    sem_destroy(&deque->empty_slots);
    pthread_mutex_destroy(&deque->mutex);
    free(deque->orders);
    return -1;
  }

  return 0;
}

void t_OrderDeque_destroy(t_OrderDeque *deque) {
  if (deque == 0)
    return;

  if (deque->orders != 0)
    free(deque->orders);
}

int t_OrderDeque_enqueue(t_OrderDeque *deque, t_Order *order) {
  if (deque == 0 || order == 0)
    return -1;

  sem_wait(&deque->empty_slots);
  pthread_mutex_lock(&deque->mutex);

  deque->orders[deque->tail] = *order;
  deque->tail                = (deque->tail + 1) % deque->size;

  pthread_mutex_unlock(&deque->mutex);
  sem_post(&deque->full_slots);

  return 0;
}

int t_OrderDeque_dequeue(t_OrderDeque *deque, t_Order *order) {
  if (deque == 0 || order == 0)
    return -1;

  sem_wait(&deque->full_slots);
  pthread_mutex_lock(&deque->mutex);

  *order      = deque->orders[deque->head];
  deque->head = (deque->head + 1) % deque->size;

  pthread_mutex_unlock(&deque->mutex);
  sem_post(&deque->empty_slots);

  return 0;
}
