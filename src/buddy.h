#pragma once

typedef unsigned long ul_t;

typedef struct blk {
	ul_t size, start_address, order;
	struct blk *next, *previous;
} blk_t;

int buddy_initialize(ul_t size, ul_t low, ul_t high);

blk_t *buddy_allocate(ul_t size);

void buddy_deallocate(blk_t *object);

void buddy_destroy(void);
