

#ifndef XREAD_H
#define XREAD_H

#include <stdint.h>



typedef enum xr_type {
    xr_type_attribute,
    xr_type_element_start,
    xr_type_element_end,
    xr_type_error,
} xr_type_t;

typedef struct xr_str {
    const char* cstr;
    int32_t     len;
} xr_str_t;

typedef void (*xr_callback)(xr_type_t type, const xr_str_t* name, const xr_str_t* value, void* user_data);

void xr_read(xr_callback cb, const char* doc, void* user_data);




#endif 
