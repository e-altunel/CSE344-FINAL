#ifndef INC_MATRIX
#define INC_MATRIX

#include <CookingPersonel.h>

#define ROWS 30
#define COLS 40

void transpose(double src[ROWS][COLS], double dst[COLS][ROWS], t_CookingPersonel *personel, int mode);
void multiply(double A[COLS][ROWS], double B[ROWS][COLS], double C[COLS][COLS], t_CookingPersonel *personel, int mode);
int  inverse(double src[COLS][COLS], double dst[COLS][COLS], t_CookingPersonel *personel, int mode);
void pseudo_inverse(double A[ROWS][COLS], double A_pinv[COLS][ROWS], t_CookingPersonel *personel, int mode);
void initialize_random_matrix(double A[ROWS][COLS]);
void calculate_prepare_time(t_CookingPersonel *personel, int mode);

#endif /* INC_MATRIX */
