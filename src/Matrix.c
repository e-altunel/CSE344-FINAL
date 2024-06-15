#include <CookingPersonel.h>
#include <Matrix.h>
#include <stdlib.h>
#include <sys/time.h>

void transpose(double src[ROWS][COLS], double dst[COLS][ROWS], t_CookingPersonel *personel, int mode) {
  for (int i = 0; i < ROWS; ++i) {
    if (personel) {
      pthread_mutex_lock(&personel->job_lock);
      sem_wait(&personel->oven_ref->available_aparatus);
    }
    for (int j = 0; j < COLS / mode; ++j) {
      dst[j][i] = src[i][j];
    }
    if (personel) {
      sem_post(&personel->oven_ref->available_aparatus);
      pthread_mutex_unlock(&personel->job_lock);
    }
  }
}

void multiply(double A[COLS][ROWS], double B[ROWS][COLS], double C[COLS][COLS], t_CookingPersonel *personel, int mode) {
  for (int i = 0; i < COLS / mode; ++i) {
    for (int j = 0; j < COLS / mode; ++j) {
      if (personel) {
        pthread_mutex_lock(&personel->job_lock);
        sem_wait(&personel->oven_ref->available_aparatus);
      }
      C[i][j] = 0;
      for (int k = 0; k < ROWS; ++k) {
        C[i][j] += A[i][k] * B[k][j];
      }
      if (personel) {
        sem_post(&personel->oven_ref->available_aparatus);
        pthread_mutex_unlock(&personel->job_lock);
      }
    }
  }
}

int inverse(double src[COLS][COLS], double dst[COLS][COLS], t_CookingPersonel *personel, int mode) {
  if (personel) {
    pthread_mutex_lock(&personel->job_lock);
    sem_wait(&personel->oven_ref->available_aparatus);
  }
  double temp[COLS][COLS];
  for (int i = 0; i < COLS / mode; ++i) {
    for (int j = 0; j < COLS / mode; ++j) {
      if (i == j)
        temp[i][j] = 1;
      else
        temp[i][j] = 0;
    }
  }
  if (personel) {
    sem_post(&personel->oven_ref->available_aparatus);
    pthread_mutex_unlock(&personel->job_lock);

    pthread_mutex_lock(&personel->job_lock);
    sem_wait(&personel->oven_ref->available_aparatus);
  }
  for (int i = 0; i < COLS / mode; ++i) {
    double t = src[i][i];
    if (t == 0) {
      return -1;
    }
    for (int j = 0; j < COLS / mode; ++j) {
      src[i][j] /= t;
      temp[i][j] /= t;
    }
    for (int j = 0; j < COLS / mode; ++j) {
      if (i != j) {
        t = src[j][i];
        for (int k = 0; k < COLS / mode; ++k) {
          src[j][k] -= src[i][k] * t;
          temp[j][k] -= temp[i][k] * t;
        }
      }
    }
  }

  for (int i = 0; i < COLS / mode; ++i) {
    for (int j = 0; j < COLS / mode; ++j) {
      dst[i][j] = temp[i][j];
    }
  }
  if (personel) {
    sem_post(&personel->oven_ref->available_aparatus);
    pthread_mutex_unlock(&personel->job_lock);
  }

  return 0;
}

void pseudo_inverse(double A[ROWS][COLS], double A_pinv[COLS][ROWS], t_CookingPersonel *personel, int mode) {
  double At[COLS][ROWS];
  double AtA[COLS][COLS];
  double AtA_inv[COLS][COLS];
  double A_copy[ROWS][COLS];

  for (int i = 0; i < ROWS; ++i) {
    for (int j = 0; j < COLS / mode; ++j) {
      A_copy[i][j] = A[i][j];
    }
  }

  transpose(A, At, personel, mode);
  multiply(At, A, AtA, personel, mode);

  while (inverse(AtA, AtA_inv, personel, mode) == -1) {
    initialize_random_matrix(A_copy);
    transpose(A_copy, At, personel, mode);
    multiply(At, A_copy, AtA, personel, mode);
  }

  for (int i = 0; i < COLS / mode; ++i) {
    for (int j = 0; j < ROWS; ++j) {
      if (personel) {
        pthread_mutex_lock(&personel->job_lock);
        sem_wait(&personel->oven_ref->available_aparatus);
      }
      A_pinv[i][j] = 0;
      for (int k = 0; k < COLS / mode; ++k) {
        A_pinv[i][j] += AtA_inv[i][k] * At[k][j];
      }
      if (personel) {
        sem_post(&personel->oven_ref->available_aparatus);
        pthread_mutex_unlock(&personel->job_lock);
      }
    }
  }
}

void initialize_random_matrix(double A[ROWS][COLS]) {
  srand(time(0));
  for (int i = 0; i < ROWS; ++i) {
    for (int j = 0; j < COLS; ++j) {
      A[i][j] = (double)(rand() % 100) + 1.0;
    }
  }
}

void calculate_prepare_time(t_CookingPersonel *personel, int mode) {
  double A[ROWS][COLS];
  double A_pinv[COLS][ROWS];

  initialize_random_matrix(A);

  pseudo_inverse(A, A_pinv, personel, mode);
}