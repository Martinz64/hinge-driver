#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <dirent.h>

#include <stdlib.h>
#include <argp.h>

#include "vec.h"
#include "accel.h"

#include "uinput-thing.h"

typedef struct{
    int n;
    int size;
    bool seeded;
    float* data;
} float_histogram;

int init_float_histogram(float_histogram* hist, int size){
    float* data = malloc(sizeof(float) * size);
    if(data == NULL) return 1;
    hist->data = data;
    hist->size = size;
    hist->n = 0;
    hist->seeded = false;
    return 0;
}

float run_float_histogram(float_histogram* hist, float input){
    hist->data[hist->n] = input;
    hist->n = (hist->n+1) % hist->size;
    if(!hist->seeded) return input;

    float cnt = 0;
    for (int i = 0; i < hist->size; i++)cnt+=hist->data[i];
    return cnt/hist->size;   
}

bool hist_all_equal(float_histogram* hist){
    float last = hist->data[0];

    for (int i = 0; i < hist->size; i++)
    {
        if(fabs(hist->data[i] - last) > 0.01f)return false;
        last = hist->data[i];
    }
    return true;
}

const char *argp_program_version = "hinge-driver v0.1";
static char doc[] = "2-in-1 laptop hinge driver";
static char args_doc[] = "";
static struct argp_option options[] = { 
    { "lid",                    'l',    "FILE",     0,  "IIO device name for lid accelerometer"},
    { "base",                   'b',    "FILE",     0,  "IIO device name for base accelerometer"},
    { "limit_angle",            'a',    "ANGLE",    0,  "Hinge axis"},
    { "limit_hysteresis",       'h',    "ANGLE",    0,  "Hysteresis on limit angle (angle+/-hysteresis)"},
    { "extremes_hysteresis",    'e',    "ANGLE",    0,  "Hysteresis on extremes angle (0+hysteresis, 360-hysteresis)"},
    { "tilt_limit",             't',    "ANGLE",    0,  "Tilt angle limit"},
    { "test",                   'n',    0,          0,  "Test mode, do not send uinput events"},
    { "verbose",                'v',    0,          0,  "View live accelerometer data"},
    { 0 } 
};

struct arguments {    
    char* base;
    char* lid;
    
    float limit_angle;
    bool invert;
    float limit_hysteresis;
    float extremes_hysteresis;

    float tilt_limit;

    bool verbose;
    bool test;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
        case 'b': arguments->base=arg; break;
        case 'l': arguments->lid=arg; break;

        case 'a': arguments->limit_angle = atof(arg); break;
        case 'h': arguments->limit_hysteresis = atof(arg); break;
        case 'e': arguments->extremes_hysteresis = atof(arg); break;
        case 't': arguments->tilt_limit = atof(arg); break;
        
        case 'v': arguments->verbose = true; break;
        case 'n': arguments->test = true; break;

        case ARGP_KEY_ARG: return 0;
        default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc};

#define DEFAULT_PATH "/sys/bus/iio/devices/"
char* fix_path(char* path){
    DIR* dir = opendir(path);
    if(dir){
        closedir(dir);
    } else if(errno == ENOENT){
        //printf("aaaab\n");
        char* temp = malloc(strlen(path) + strlen(DEFAULT_PATH)+2);
        //printf("aaaab\n");
        strcpy(temp, DEFAULT_PATH);
        //printf("aaaab\n");
        strcat(temp, path);
        //printf("final: %s\n",temp);
        
        return temp;
    }
    char* temp = malloc(strlen(path)+2);
    strcat(temp, path);
    return temp;
}


float rad2deg(float in){
    return (in/(2*M_PI))*360;
}

int main(int argc, char **argv){

    struct arguments args;

    args.base = "iio:device1";
    args.lid = "iio:device0";

    args.limit_angle = 180.0f;
    args.limit_hysteresis = 15.0f;
    args.extremes_hysteresis = 30.0f;
    args.invert = false;
    args.tilt_limit = 20.0f;

    args.test = false;
    args.verbose = false;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    args.base = fix_path(args.base);
    args.lid = fix_path(args.lid);

    printf("Base: %s\n", args.base);
    printf("Lid: %s\n", args.lid);

    Accel base, lid;
    if(new_accel(&base,args.base)){
        fprintf(stderr,"Failed to open base accelerometer\n");
        return 1;
    }
    if(new_accel(&lid,args.lid)){
        fprintf(stderr,"Failed to open lid accelerometer\n");
        return 1;
    }

    int uinput_fd = init_uinput();
    if(uinput_fd < 0){
        fprintf(stderr,"Uinput error\n");
        return 1;
    }

    bool tablet = false;
    bool tablet_prev = -1;

    float_histogram hist_hinge = {0};
    init_float_histogram(&hist_hinge,10);

    float_histogram hist_tablet = {0};
    float_histogram hist_tablet_set = {0};
    init_float_histogram(&hist_tablet,10);
    init_float_histogram(&hist_tablet_set,10);

    for(;;){
        vec3 vec_base, vec_lid;
        read_accel(&base, &vec_base);
        read_accel(&lid, &vec_lid);

        //math gotten from here:
        //https://github.com/rhalkyard/minibook-dual-accelerometer/blob/main/angle-sensor-service/angle-sensor.py
        //https://github.com/MechanoPixel/nanote-next-tablet-mode-driver/blob/main/nanote-next-tablet-mode-driver.py

        float lid_yz = rad2deg(atan2(vec_lid.y,vec_lid.z));
        float base_yz = rad2deg(atan2(vec_base.y,vec_base.z));
        float base_tilt = rad2deg(atan2(vec_base.x,vec_base.z));

        if(lid_yz < 0.0f)lid_yz += 360.0f;
        if(base_yz < 0.0f)base_yz += 360.0f;

        float hinge = base_yz - lid_yz;
        if(hinge < 0.0f)hinge += 360.0f;

        hinge = run_float_histogram(&hist_hinge,hinge);

        if(args.invert)hinge *= -1.0f;

        bool tablet_set = false;

        //check if laptop angle
        if(
            hinge > args.extremes_hysteresis &&
            hinge < args.limit_angle - args.limit_hysteresis
        ){
            tablet = false;
            tablet_set = true;
        }

        //tablet mode
        if(
            hinge < 360.0f-args.extremes_hysteresis &&
            hinge > args.limit_angle + args.limit_hysteresis
        ){
            tablet = true;
            tablet_set = true;
        }

        bool tilt_allowed = false;
        float delta;
        delta = fabs(base_tilt - 0);
        if(fmin(delta, 360-delta) < args.tilt_limit){
            tilt_allowed = true;
        }

        delta = fabs(base_tilt - 180);
        if(fmin(delta, 360-delta) < args.tilt_limit){
            tilt_allowed = true;
        }
        
        //dirty hack to check if values have settled
        run_float_histogram(&hist_tablet, tablet ? 1.0f : 0.0f);
        run_float_histogram(&hist_tablet_set, tablet_set ? 1.0f : 0.0f);

        bool settled = hist_all_equal(&hist_tablet) && hist_all_equal(&hist_tablet_set);

        if(settled && tilt_allowed){
            if(tablet != tablet_prev){
                if(!args.test)switch_tablet_mode(uinput_fd,tablet);
            }
            tablet_prev = tablet;
        }

        if(args.verbose){
            printf("\n\n");
            printf("Input data:\n");
            printf("base = %7.5f, %7.5f, %7.5f\n", vec_base.x, vec_base.y, vec_base.z);
            printf("lid  = %7.5f, %7.5f, %7.5f\n", vec_lid.x, vec_lid.y, vec_lid.z);

            printf("Angles:\n");
            printf("lid_yz  = %7.5f\n",lid_yz);
            printf("base_yz = %7.5f\n",base_yz);
            printf("tilt    = %7.5f\n",base_tilt);
            printf("hinge   = %7.5f\n",hinge);

            printf("Decisions:\n");
            printf("tablet     = %s\n", tablet ? "true" : "false");
            printf("tablet_set = %s\n", tablet_set ? "true" : "false");
            printf("settled    = %s\n", settled ? "true" : "false");
            printf("tilt_allow = %s\n", tilt_allowed ? "true" : "false");    
        }

        usleep(1000*50);
    }

}
