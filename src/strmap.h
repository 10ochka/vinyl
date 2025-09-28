
#ifndef __VINYL_STRMAP_H__
#define __VINYL_STRMAP_H__

#include "object.h"
#include "string.h"
#include <stddef.h>


typedef struct Vnl_StringMap Vnl_StringMap;

typedef void(*Vnl_StringMapCallback)(Vnl_String, const Vnl_Object *);

Vnl_StringMap *vnl_strmap_new();
void vnl_strmap_free(Vnl_StringMap *);
void vnl_strmap_clear(Vnl_StringMap *);
void vnl_strmap_insert(Vnl_StringMap *, Vnl_String, Vnl_Object *);
Vnl_Object *vnl_strmap_pop(Vnl_StringMap *, Vnl_String);
Vnl_Object *vnl_strmap_find(Vnl_StringMap *, Vnl_String);
bool vnl_strmap_contains(Vnl_StringMap *, Vnl_String);

void vnl_strmap_foreach(const Vnl_StringMap *, Vnl_StringMapCallback);


#endif // __VINYL_STRMAP_H__
