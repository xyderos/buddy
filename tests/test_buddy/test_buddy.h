#pragma once
#include <check.h>
#include <stdlib.h>

#include "../../src/buddy.h"

Suite *buddy_suite(void);

START_TEST(test_BuddyInit_Success)
{
	int result = buddy_initialize(1024, 32, 512);
	ck_assert_int_eq(result, 0);
}
END_TEST

START_TEST(test_BuddyInit_Fail)
{
	int result = buddy_initialize(1000, 32, 512);
	ck_assert_int_eq(result, -1);
}
END_TEST

START_TEST(test_BuddyAlloc_ValidSize)
{
	blk_t *block = NULL;

	buddy_initialize(1024, 32, 512);

	block = buddy_allocate(128);
	ck_assert_ptr_nonnull(block);
	ck_assert_int_eq(block->size, 128);

	buddy_deallocate(block);
	buddy_destroy();
}
END_TEST

START_TEST(test_FreeBuddyMemory)
{
	blk_t *block = NULL;

	buddy_initialize(1024, 32, 512);

	block = buddy_allocate(128);
	ck_assert_ptr_nonnull(block);

	buddy_deallocate(block);
	buddy_destroy();
}
END_TEST

START_TEST(test_BuddyDestroy)
{
	buddy_initialize(1024, 32, 512);
	buddy_allocate(128);
	buddy_destroy();
}
END_TEST
