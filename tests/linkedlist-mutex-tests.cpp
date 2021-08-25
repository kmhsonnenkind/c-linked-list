/*
 * MIT License
 *
 * Copyright (c) 2021 Martin Kloesch
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * \file linkedlist-mutex-tests.cpp
 * \author Martin Kloesch <martin@gmail.com>
 * \brief Tests for mutex / lock implementation
 */
#include "catch2/catch.hpp"
#include <errno.h>
#include <stddef.h>
#include "linkedlist-mutex.h"

/**
 * \test Tests that linkedlist_mutex_initialize() correctly initializes mutex.
 */
TEST_CASE("initialize()", "[linkedlist_mutex_initialize()]")
{
    // Initialize mutex
    linkedlist_mutex_t mutex = NULL;
    REQUIRE(linkedlist_mutex_initialize(&mutex) == LINKEDLIST_MUTEX_SUCCESS);

    // Check that mutex was successfully initialized
    REQUIRE(mutex != NULL);

    // Cleanup
    linkedlist_mutex_destroy(&mutex);
}

/**
 * \test Tests that linkedlist_mutex_initialize() correctly detects invalid
 * \c NULL parameter.
 */
TEST_CASE("initialize() NULL", "[linkedlist_mutex_initialize(), error]")
{
    // Make sure initialize() correctly detects invalid mutex
    REQUIRE(linkedlist_mutex_initialize(NULL) == EINVAL);
}

/**
 * \test Tests that linkedlist_mutex_lock() works with initialized lock.
 *
 * \details No checks are performed if mutex is actually acquired.
 */
TEST_CASE("lock()", "[linkedlist_mutex_lock()]")
{
    // Initialize mutex
    linkedlist_mutex_t mutex = NULL;
    REQUIRE(linkedlist_mutex_initialize(&mutex) == LINKEDLIST_MUTEX_SUCCESS);

    // Check that mutex can be locked
    REQUIRE(linkedlist_mutex_lock(&mutex) == LINKEDLIST_MUTEX_SUCCESS);

    // Cleanup
    linkedlist_mutex_unlock(&mutex);
    linkedlist_mutex_destroy(&mutex);
}

/**
 * \test Tests that linkedlist_mutex_lock() correctly detects invalid
 * \c NULL parameter.
 */
TEST_CASE("lock() NULL", "[linkedlist_mutex_lock(), error]")
{
    // Make sure lock() correctly detects invalid mutex
    REQUIRE(linkedlist_mutex_lock(NULL) == EINVAL);
}

/**
 * \test Tests that linkedlist_mutex_lock() correctly detects uninitialized
 * lock.
 */
TEST_CASE("lock() uninitialized", "[linkedlist_mutex_lock(), error]")
{
    // Make sure lock() correctly detects invalid mutex
    linkedlist_mutex_t mutex = NULL;
    REQUIRE(linkedlist_mutex_lock(&mutex) == EINVAL);
}

/**
 * \test Tests that linkedlist_mutex_lock() correctly detects destroyed lock.
 */
TEST_CASE("lock() destroyed", "[linkedlist_mutex_lock(), error]")
{
    // Initialize mutex and immediately destroy it
    linkedlist_mutex_t mutex = NULL;
    REQUIRE(linkedlist_mutex_initialize(&mutex) == LINKEDLIST_MUTEX_SUCCESS);
    linkedlist_mutex_destroy(&mutex);

    // Make sure lock() correctly detects invalid mutex
    REQUIRE(linkedlist_mutex_lock(&mutex) == EINVAL);
}

/**
 * \test Tests that linkedlist_mutex_unlock() works with initialized lock.
 *
 * \details No checks are performed if mutex is actually released.
 */
TEST_CASE("unlock()", "[linkedlist_mutex_unlock()]")
{
    // Initialize and lock mutex
    linkedlist_mutex_t mutex = NULL;
    REQUIRE(linkedlist_mutex_initialize(&mutex) == LINKEDLIST_MUTEX_SUCCESS);
    REQUIRE(linkedlist_mutex_lock(&mutex) == LINKEDLIST_MUTEX_SUCCESS);

    // Check that mutex can be unlocked
    REQUIRE(linkedlist_mutex_unlock(&mutex) == LINKEDLIST_MUTEX_SUCCESS);

    // Cleanup
    linkedlist_mutex_destroy(&mutex);
}

/**
 * \test Tests that linkedlist_mutex_unlock() correctly detects invalid
 * \c NULL parameter.
 */
TEST_CASE("unlock() NULL", "[linkedlist_mutex_unlock(), error]")
{
    // Make sure unlock() correctly detects invalid mutex
    REQUIRE(linkedlist_mutex_unlock(NULL) == EINVAL);
}

/**
 * \test Tests that linkedlist_mutex_unlock() correctly detects uninitialized
 * lock.
 */
TEST_CASE("unlock() uninitialized", "[linkedlist_mutex_unlock(), error]")
{
    // Make sure unlock() correctly detects invalid mutex
    linkedlist_mutex_t mutex = NULL;
    REQUIRE(linkedlist_mutex_unlock(&mutex) == EINVAL);
}

/**
 * \test Tests that linkedlist_mutex_lock() correctly detects destroyed lock.
 */
TEST_CASE("unlock() destroyed", "[linkedlist_mutex_unlock(), error]")
{
    // Initialize mutex and immediately destroy it
    linkedlist_mutex_t mutex = NULL;
    REQUIRE(linkedlist_mutex_initialize(&mutex) == LINKEDLIST_MUTEX_SUCCESS);
    linkedlist_mutex_destroy(&mutex);

    // Make sure unlock() correctly detects invalid mutex
    REQUIRE(linkedlist_mutex_unlock(&mutex) == EINVAL);
}
