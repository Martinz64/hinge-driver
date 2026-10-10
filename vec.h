#ifndef VEC_H
#define VEC_H

typedef struct{
    float x;
    float y;
    float z;
} vec3;


float dp3(vec3 u, vec3 v);
vec3 norm3(vec3 u);
vec3 proj_perpendicular3(vec3 v, vec3 axis);
vec3 cross3(vec3 u, vec3 v);
float mag3(vec3 v);

#endif