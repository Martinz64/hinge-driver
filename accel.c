#include "accel.h"
#include "vec.h"
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int read_matrix(float mat[3][3], char* path){
    FILE* f = fopen(path,"r");
    if(f == NULL) {
        printf("open file error\n");
        return 1;
    }
    fscanf(f,
        "%f,%f,%f;%f,%f,%f;%f,%f,%f",
        &mat[0][0],
        &mat[0][1],
        &mat[0][2],
        &mat[1][0],
        &mat[1][1],
        &mat[1][2],
        &mat[2][0],
        &mat[2][1],
        &mat[2][2]
    );
    fclose(f);
    return 0;
}

int read_float(float *fl, char* path){
    FILE* f = fopen(path,"r");
    if(f == NULL) {
        printf("open file error\n");
        return 1;
    }
    fscanf(f, "%f", fl);
    fclose(f);
    return 0;
}


int checkdir(char* path){
    DIR* dir = opendir(path);
    if(dir){
        closedir(dir);
        return 0;
    } else if(errno == ENOENT){
        return 1;
    }
    return 1;
}

int checkfile(char* path){
    FILE* f = fopen(path,"r");
    if(f == NULL) return 1;
    fclose(f);
    return 0;
}

#define X_SUFFIX "/in_accel_x_raw"
#define Y_SUFFIX "/in_accel_y_raw"
#define Z_SUFFIX "/in_accel_z_raw"

int new_accel(Accel* acc, char* base_path){
    Accel a = {0};

    if(checkdir(base_path)){
        fprintf(stderr, "Could not open accelerometer, path: %s\n", base_path);
        return 1;
    }

    a.path = malloc(strlen(base_path)+1);
    strcpy(a.path,base_path);


    char matrix_path[128];
    strcpy(matrix_path,a.path);
    strcat(matrix_path, "/in_mount_matrix");
    a.has_matrix = !read_matrix(a.tranform_matrix, matrix_path);

    char global_scale_path[128];
    strcpy(global_scale_path,a.path);
    strcat(global_scale_path, "/in_accel_scale");
    a.global_scale = 1;
    read_float(&a.global_scale, global_scale_path);


    a.x_path = malloc(strlen(a.path)+strlen(X_SUFFIX)+1);
    strcpy(a.x_path,a.path);
    strcat(a.x_path, X_SUFFIX);

    a.y_path = malloc(strlen(a.path)+strlen(Y_SUFFIX)+1);
    strcpy(a.y_path,a.path);
    strcat(a.y_path, Y_SUFFIX);

    a.z_path = malloc(strlen(a.path)+strlen(Z_SUFFIX)+1);
    strcpy(a.z_path,a.path);
    strcat(a.z_path, Z_SUFFIX);

    if(
        checkfile(a.x_path) ||
        checkfile(a.y_path) ||
        checkfile(a.z_path)
    ){
        fprintf(stderr, "Accelerometer is missing one of x,y,z\n");
        return 1;
    }

    *acc = a;
    return 0;
}

int read_accel(Accel *a, vec3* vec){
    float x,y,z;

    /*char accel_x_path[128];
    strcpy(accel_x_path,a->path);
    strcat(accel_x_path, "/in_accel_x_raw");
    read_float(&x, accel_x_path);

    char accel_y_path[128];
    strcpy(accel_y_path,a->path);
    strcat(accel_y_path, "/in_accel_y_raw");
    read_float(&y, accel_y_path);

    char accel_z_path[128];
    strcpy(accel_z_path,a->path);
    strcat(accel_z_path, "/in_accel_z_raw");
    read_float(&z, accel_z_path);*/
    read_float(&x, a->x_path);
    read_float(&y, a->y_path);
    read_float(&z, a->z_path);
    

    x *= a->global_scale;
    y *= a->global_scale;
    z *= a->global_scale;

    if(a->has_matrix){
        float m[3][3]; // muy guarro ya lo se
        memcpy(m,a->tranform_matrix, sizeof(m));
        
        vec->x = x*m[0][0] + y*m[0][1] + z*m[0][2];
        vec->y = x*m[1][0] + y*m[1][1] + z*m[1][2];
        vec->z = x*m[2][0] + y*m[2][1] + z*m[2][2];
    } else {
        vec->x = x;
        vec->y = y;
        vec->z = z;
    }

    return 0;
}