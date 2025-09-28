
#include "object.h"
#include "common.h"
#include "string.h"
#include <stddef.h>
#include <string.h>


void *vnl_object_create(size_t objsize, Vnl_ObjectType type) {
	Vnl_Object *obj = vnl_malloc(objsize);
	*obj = VNL_OBJECT_HEAD_INIT(type);
	return obj;
}

void vnl_object_destroy(Vnl_Object *self) {
	switch (self->type) {
		case VNL_OBJTYPE_NUMBER: {
			vnl_free(self);
		} break;
		case VNL_OBJTYPE_STRING: {
			Vnl_StringObject *obj = (void *)self;
			vnl_strbuf_free(&obj->value);
			vnl_free(obj);
		} break;
		case VNL_OBJTYPE_ARRAY: {
			Vnl_ArrayObject *obj = (void *)self;
			for (size_t i = 0; i < obj->len; ++i) {
				vnl_object_release(obj->items[i]);
			}
			vnl_free(obj->items);
			vnl_free(obj);
		} break;
	}
}

void vnl_object_acquire(Vnl_Object *self) {
	self->refcount++;
}

void vnl_object_release(Vnl_Object *self) {
	if (self->refcount == 0) {
		vnl_object_destroy(self);
	} else {
		self->refcount--;
	}
}
