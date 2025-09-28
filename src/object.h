#ifndef __VINYL_OBJECT_H__
#define __VINYL_OBJECT_H__

#include "string.h"
#include <stddef.h>

typedef enum Vnl_ObjectType Vnl_ObjectType;
typedef struct Vnl_Object Vnl_Object;
typedef struct Vnl_NumberObject Vnl_NumberObject;
typedef struct Vnl_StringObject Vnl_StringObject;
typedef struct Vnl_ArrayObject Vnl_ArrayObject;

enum Vnl_ObjectType {
	VNL_OBJTYPE_NUMBER = 1,
	VNL_OBJTYPE_STRING = 2,
	VNL_OBJTYPE_ARRAY  = 3,
};

#define VNL_OBJECT_HEAD Vnl_Object __base__
#define VNL_OBJECT_HEAD_INIT(type) (Vnl_Object){ .refcount = 0, .type = (type) }

struct Vnl_Object {
	size_t refcount;
	Vnl_ObjectType type;
};


struct Vnl_NumberObject {
	VNL_OBJECT_HEAD;
	double value;
};

struct Vnl_StringObject {
	VNL_OBJECT_HEAD;
	Vnl_StringBuffer value;
};

struct Vnl_ArrayObject {
	VNL_OBJECT_HEAD;
	Vnl_Object **items;
	size_t len;
	size_t cap;
};


void *vnl_object_create(size_t, Vnl_ObjectType);
void vnl_object_destroy(Vnl_Object *);
void vnl_object_acquire(Vnl_Object *);
void vnl_object_release(Vnl_Object *);


#endif // __VINYL_OBJECT_H__
