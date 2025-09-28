#ifndef __VINYL_COMMON__
#define __VINYL_COMMON__


#include <stddef.h>
#include <stdio.h>


#define VNL_ANSICOL_RESET   "\e[0m"
#define VNL_ANSICOL_BLACK   "\e[30m"
#define VNL_ANSICOL_RED     "\e[31m"
#define VNL_ANSICOL_GREEN   "\e[32m"
#define VNL_ANSICOL_YELLOW  "\e[33m"
#define VNL_ANSICOL_BLUE    "\e[34m"
#define VNL_ANSICOL_MAGENTA "\e[35m"
#define VNL_ANSICOL_CYAN    "\e[36m"
#define VNL_ANSICOL_WHITE   "\e[37m"


void *vnl_malloc(size_t);
void *vnl_realloc(void *, size_t);
void vnl_free(void *);

bool vnl_memcmpeq(const void *, const void *, size_t);

#endif // __VINYL_COMMON__
