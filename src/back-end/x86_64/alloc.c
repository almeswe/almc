#include "gen.h"

extern global_gen_data glob_info;

char* alloc_float_def() {
    char* def = cnew(char, 64);
    sprintf(def, "__real@%ld",
        glob_info.count_of.floats++);
    return putalloc(def), def;
 }

char* alloc_label_def() {
    char* def = cnew(char, 64);
    sprintf(def, "_L@%ld", 
        glob_info.count_of.labels++);
    return putalloc(def), def;
}

char* alloc_str_def() {
    char* def = cnew(char, 64);
    sprintf(def, "__str@%ld", 
        glob_info.count_of.floats++);
    return putalloc(def), def;
}

char* alloc_func_def(const char* base) {
    char* def = cnew(char, strlen(base)+2);
    sprintf(def, "_%s", base);
    return putalloc(def), def;
}

char* alloc_var_def(const char* base) {
    char* def = cnew(char, strlen(base)+3);
    sprintf(def, "_%s$", base);
    return putalloc(def), def;
}