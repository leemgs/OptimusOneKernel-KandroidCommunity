

#ifndef DIAGMEM_H
#define DIAGMEM_H
#include "diagchar.h"

void *diagmem_alloc(struct diagchar_dev *driver, int size, int pool_type);
void diagmem_free(struct diagchar_dev *driver, void *buf, int pool_type);
void diagmem_init(struct diagchar_dev *driver);
void diagmem_exit(struct diagchar_dev *driver);

#endif
