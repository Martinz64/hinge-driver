#include "vec.h"

#ifndef ACCEL_H
#define ACCEL_H



typedef struct{
    char* path;
    float tranform_matrix[3][3];
    bool has_matrix;
    float global_scale;

    char* x_path;
    char* y_path;
    char* z_path;
} Accel;

int new_accel(Accel* acc, char* base_path);
int read_accel(Accel *a, vec3* vec);





#endif