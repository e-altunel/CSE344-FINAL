#ifndef INC_COOKINGPERSONEL
#define INC_COOKINGPERSONEL

#include <CookOven.h>
#include <Order.h>
#include <OrderDeque.h>
#include <pthread.h>

typedef struct s_CookingPersonel {
  int             id;
  t_OrderDeque   *deque_ref;
  t_OrderDeque   *finishedDeque_ref;
  t_CookOven     *oven_ref;
  t_Order        *active_order;
  t_Order        *pending_order;
  pthread_t       prepare_thread;
  pthread_t       cook_thread;
  pthread_mutex_t job_lock;
  pthread_mutex_t variable_lock;
  int             is_exit;
  int             is_cancelled;
  int             cooked_count;
} t_CookingPersonel;

int  t_CookingPersonel_init(t_CookingPersonel *personel, int id, t_CookOven *oven, t_OrderDeque *deque,
                            t_OrderDeque *finishedDeque);
void t_CookingPersonel_destroy(t_CookingPersonel *personel);
void t_CookingPersonel_set_exit(t_CookingPersonel *personel);

void *t_CookingPersonel_prepare_thread(void *arg);
void *t_CookingPersonel_cook_thread(void *arg);

int t_CookingPersonel_prepare(t_CookingPersonel *personel);
int t_CookingPersonel_insert(t_CookingPersonel *personel);
int t_CookingPersonel_cook(t_CookingPersonel *personel);
int t_CookingPersonel_remove(t_CookingPersonel *personel);

int t_CookingPersonel_check_prepare(const t_CookingPersonel *personel, pthread_mutex_t *mutex);
int t_CookingPersonel_check_insert(const t_CookingPersonel *personel, pthread_mutex_t *mutex);
int t_CookingPersonel_check_cook(const t_CookingPersonel *personel, pthread_mutex_t *mutex);
int t_CookingPersonel_check_remove(const t_CookingPersonel *personel, pthread_mutex_t *mutex);

int t_CookingPersonel_is_exit(t_CookingPersonel *personel);
int t_CookingPersonel_has_active_order(t_CookingPersonel *personel);
int t_CookingPersonel_has_pending_order(t_CookingPersonel *personel);

#endif /* INC_COOKINGPERSONEL */