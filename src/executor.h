#ifndef __VINYL_EXECUTOR_H__
#define __VINYL_EXECUTOR_H__


#include "object.h"
#include "string.h"


typedef struct Vnl_Executor Vnl_Executor;


Vnl_Executor *vnl_exec_new();
void vnl_exec_free(Vnl_Executor *);

void vnl_exec_setvar(Vnl_Executor *, Vnl_String, Vnl_Object *);
Vnl_Object *vnl_exec_getvar(Vnl_Executor *, Vnl_String);
void vnl_exec_delvar(Vnl_Executor *, Vnl_String);

bool vnl_exec_string(Vnl_Executor *, Vnl_String);


#endif // __VINYL_EXECUTOR_H__
