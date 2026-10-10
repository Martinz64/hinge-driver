#include "vec.h"
#include <math.h>



float dp3(vec3 u, vec3 v){
    return u.x*v.x + u.y*v.y + u.z*v.z;
}
vec3 norm3(vec3 u){
    float d = mag3(u);

    vec3 r;
    r.x = u.x / d;
    r.y = u.y / d;
    r.z = u.z / d;
    
    /*if(d < 1e-9){
        r.x = 0;
        r.y = 0;
        r.z = 0;
    }*/

    return r;
}

vec3 proj_perpendicular3(vec3 v, vec3 axis){
    vec3 r;
    float d = dp3(v,axis);

    r.x = v.x - d*axis.x;
    r.y = v.y - d*axis.y;
    r.z = v.z - d*axis.z;
    return r;
}

vec3 cross3(vec3 u, vec3 v){
    vec3 r;
    r.x = u.y*v.z - u.z*v.y;
    r.y = u.z*v.x - u.x*v.z;
    r.z = u.x*v.y - u.y*v.x;
    return r;
}


float mag3(vec3 v){
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}