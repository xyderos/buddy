#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "buddy.h"

static ul_t maximum_memory_size;
static ul_t minimum_size;
static ul_t maximum_size;

static blk_t *first_hole, *first_object;

static inline ul_t
is_power_of_two(ul_t n)
{
	return (n > 0) && ((n & (n - 1)) == 0);
}

static inline ul_t
order(ul_t size, ul_t low)
{
	ul_t order = 0;

	while ((1 << order) * low < size)
		++order;

	return order;
}

static ul_t
buddy_find_address(blk_t *block)
{
	ul_t block_size = 0;

	if (!block) {
		return (size_t)(0 - 1);
	}

	block_size = (1 << block->order) * minimum_size;

	return block_size ^ block->start_address;
}

static blk_t *
buddy_find(blk_t *hole)
{
	blk_t *previous_hole = NULL, *next_hole = NULL;
	ul_t buddyAddress = 0;

	if (!hole)
		return NULL;

	buddyAddress = buddy_find_address(hole);

	previous_hole = hole->previous;
	next_hole = hole->next;

	if (previous_hole && previous_hole->order == hole->order &&
	    previous_hole->start_address == buddyAddress)
		return previous_hole;

	if (next_hole && next_hole->order == hole->order &&
	    next_hole->start_address == buddyAddress)
		return next_hole;

	return NULL;
}

static blk_t *
buddy_merge_holes(blk_t *hole)
{
	blk_t *block = buddy_find(hole), *next_hole = NULL;

	if (!block)
		return hole;

	if (hole->previous == block) {
		return buddy_merge_holes(block);
	}

	next_hole = block->next;

	hole->size += block->size;
	++hole->order;
	hole->next = next_hole;

	if (next_hole)
		next_hole->previous = hole;

	free(block);

	return buddy_merge_holes(hole);
}

static blk_t *
buddy_partition_hole(blk_t *chosenHole, ul_t stopOrder)
{
	blk_t *previous_hole = NULL, *new_hole = NULL, *current_hole = NULL;

	if (!chosenHole || stopOrder >= chosenHole->order)
		return NULL;

	previous_hole = chosenHole->previous;
	new_hole = NULL;
	current_hole = chosenHole;

	while (current_hole->order != stopOrder) {

		new_hole = (blk_t *)malloc(sizeof(blk_t));
		new_hole->start_address = current_hole->start_address;
		new_hole->size = current_hole->size >> 1;
		new_hole->next = current_hole;
		new_hole->previous = NULL;
		new_hole->order = current_hole->order - 1;

		current_hole->size = current_hole->size >> 1;
		current_hole->previous = new_hole;
		current_hole->order -= 1;
		current_hole->start_address += current_hole->size;

		current_hole = new_hole;
	}

	new_hole->previous = previous_hole;

	if (previous_hole)
		previous_hole->next = new_hole;

	while (first_hole->previous)
		first_hole = first_hole->previous;

	return new_hole;
}

int
buddy_initialize(ul_t size, ul_t low, ul_t high)
{
	if (!(is_power_of_two(size) && is_power_of_two(low) &&
		is_power_of_two(high)))
		return -1;

	maximum_memory_size = size;
	minimum_size = low;
	maximum_size = high;

	first_hole = (blk_t *)malloc(sizeof(blk_t));

	first_hole->next = NULL;
	first_hole->previous = NULL;
	first_hole->size = maximum_memory_size;
	first_hole->start_address = 0;
	first_hole->order = (ul_t)log2((double)size / low);

	return 0;
}

blk_t *
buddy_allocate(ul_t size)
{
	ul_t current_order = 0, min_order = INT_MAX;
	blk_t *current_hole = NULL, *next_hole = NULL, *previous_hole = NULL,
	      *current_object = NULL, *next_object = NULL,
	      *previous_object = NULL, *choosen_hole = NULL;

	if (size > maximum_size)
		return NULL;

	current_order = order(size, minimum_size);
	current_hole = first_hole;

	while (current_hole) {

		if (current_hole->order == current_order)
			break;

		current_hole = current_hole->next;
	}

	if (!current_hole) {

		choosen_hole = NULL;

		current_hole = first_hole;

		while (current_hole) {
			if (min_order >= current_hole->order &&
			    current_hole->order > current_order) {

				min_order = current_hole->order;
				choosen_hole = current_hole;
			}

			current_hole = current_hole->next;
		}

		if (!choosen_hole)
			return NULL;

		current_hole = buddy_partition_hole(choosen_hole,
		    current_order);
	}

	next_hole = current_hole->next;
	previous_hole = current_hole->previous;

	if (previous_hole)
		previous_hole->next = next_hole;

	if (next_hole) {
		next_hole->previous = previous_hole;
	}

	if (current_hole == first_hole)
		first_hole = next_hole;

	current_hole->previous = NULL;
	current_hole->next = NULL;
	current_hole->size = size;

	if (!first_object) {
		first_object = current_hole;
		return first_object;
	}

	next_object = NULL;
	previous_object = NULL;
	current_object = first_object;

	while (current_object) {

		if (current_object->start_address <
		    current_hole->start_address) {

			previous_object = current_object;
			next_object = current_object->next;
		} else {

			next_object = current_object;
			break;
		}

		current_object = current_object->next;
	}

	if (previous_object) {
		previous_object->next = current_hole;
		current_hole->previous = previous_object;
	} else {
		first_object = current_hole;
	}

	if (next_object) {

		current_hole->next = next_object;
		next_object->previous = current_hole;
	}

	return current_hole;
}

void
buddy_deallocate(blk_t *object)
{
	blk_t *previous_object = object->previous, *next_object = object->next,
	      *previous_hole = NULL, *next_hole = NULL, *current_hole = NULL;

	if (previous_object)
		previous_object->next = next_object;

	if (next_object)
		next_object->previous = previous_object;

	if (object == first_object)
		first_object = next_object;

	object->next = NULL;
	object->previous = NULL;
	object->size = (1 << object->order) * minimum_size;

	previous_hole = NULL;
	next_hole = NULL;
	current_hole = first_hole;

	while (current_hole) {

		if (current_hole->start_address < object->start_address) {
			previous_hole = current_hole;
			next_hole = current_hole->next;
		} else {
			next_hole = current_hole;
			break;
		}

		current_hole = current_hole->next;
	}

	if (previous_hole) {
		object->previous = previous_hole;
		previous_hole->next = object;
	} else
		first_hole = object;

	if (next_hole) {
		next_hole->previous = object;
		object->next = next_hole;
	}
	buddy_merge_holes(object);
}

static void
buddy_reset(void)
{
	while (first_object) {
		buddy_deallocate(first_object);
	}
}

void
buddy_destroy(void)
{
	buddy_reset();
	free(first_hole);
	first_hole = NULL;
	maximum_memory_size = 0;
	minimum_size = 0;
	maximum_size = 0;
}
