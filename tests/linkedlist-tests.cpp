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
 * \file linkedlist-tests.cpp
 * \author Martin Kloesch <martin@gmail.com>
 * \brief Tests for linked-list implementation
 */
#include "catch2/catch.hpp"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "linkedlist.h"

/** \struct ByteBuffer
 * \brief Example structure for a byte buffer.
 *
 * \details Used to test custom copy / deallocation behaviour.
 */
typedef struct
{
    /**
     * \brief Number of bytes in byte buffer.
     */
    size_t length;

    /**
     * \brief Actual byte buffer data.
     */
    uint8_t *data;
} ByteBuffer;

/**
 * \brief Custom \ref linkedlist_copyfunction_t for ByteBuffer objects.
 *
 * \param[out] dest ByteBuffer object to store data to.
 * \param[in] src ByteBuffer object to copy data from.
 * \param[in] _ Ignored.
 * \return void* \p dest pointer, \c NULL in case of error.
 */
static void *bytebuffer_copy(void *dest, const void *src, size_t _)
{
    // Validate parameters
    if ((dest == NULL) || (src == NULL))
    {
        return NULL;
    }

    // Cast to more usable format
    ByteBuffer *dest_buffer = (ByteBuffer *) dest;
    ByteBuffer *src_buffer = (ByteBuffer *) src;

    // Check if data needs to be copied
    if (src_buffer->length > 0)
    {
        // Invalid ByteBuffer
        if (src_buffer->data == NULL)
        {
            return NULL;
        }

        // Allocate memory
        dest_buffer->data = (uint8_t *) malloc(src_buffer->length);
        if (dest_buffer->data == NULL)
        {
            return NULL;
        }
        memcpy(dest_buffer->data, src_buffer->data, src_buffer->length);
    }
    else
    {
        // Empty buffer
        dest_buffer->data = NULL;
    }
    dest_buffer->length = src_buffer->length;

    return dest_buffer;
}

/**
 * \brief Custom \ref linkedlist_deallocfunction_t for ByteBuffer objects.
 *
 * \param[in] ptr ByteBuffer object to be deallocated.
 */
static void bytebuffer_dealloc(void *ptr)
{
    if (ptr != NULL)
    {
        ByteBuffer *buffer = (ByteBuffer *) ptr;
        if ((buffer->length > 0) && (buffer->data != NULL))
        {
            free(buffer->data);
        }
        buffer->data = NULL;
        buffer->length = 0;
    }
}

/**
 * \test Tests that linkedlist_initialize() correctly sets default values for
 * members.
 */
TEST_CASE("initialize() default", "[linkedlist_initialize()]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Check that members have been correctly set to default values
    REQUIRE(list._head == NULL);
    REQUIRE(list._item_size == sizeof(int));
    REQUIRE(list._copy == memcpy);
    REQUIRE(list._dealloc == NULL);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_initialize() correctly sets members for custom
 * data types.
 */
TEST_CASE("initialize() custom", "[linkedlist_initialize(), custom]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), &bytebuffer_copy,
                                  &bytebuffer_dealloc) == LINKEDLIST_SUCCESS);

    // Check that members have been correctly set to default values
    REQUIRE(list._head == NULL);
    REQUIRE(list._item_size == sizeof(int));
    REQUIRE(list._copy == &bytebuffer_copy);
    REQUIRE(list._dealloc == &bytebuffer_dealloc);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_initialize() correctly detects invalid \c
 * NULL parameter.
 */
TEST_CASE("initialize() list NULL", "[linkedlist_initialize(), error]")
{
    REQUIRE(linkedlist_initialize(NULL, sizeof(int), NULL, NULL) == EINVAL);
}

/**
 * \test Tests that linkedlist_initialize() correctly detects invalid
 * itemsize.
 */
TEST_CASE("initialize() item size 0", "[linkedlist_initialize(), error]")
{
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, 0, NULL, NULL) == EINVAL);
}

/**
 * \test Tests that linkedlist_add() correctly adds elements to empty list.
 */
TEST_CASE("add() first", "[linkedlist_add()]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Check length before call
    size_t length = 1;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 0);

    // Add item to empty list
    int value = 42;
    REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);

    // Check that length has been updated accordingly
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 1);

    // Check that value was correctly set
    int check;
    REQUIRE(linkedlist_get(&list, 0, &check) == LINKEDLIST_SUCCESS);
    REQUIRE(check == value);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_add() correctly adds elements to list.
 */
TEST_CASE("add()", "[linkedlist_add()]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    for (int i = 0; i < 5; i++)
    {
        // Check length before call
        size_t length = 0;
        REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
        REQUIRE(length == i);

        // Add item
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);

        // Check length after call
        REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
        REQUIRE(length == i + 1);

        // Check that item was correctly set
        int value;
        REQUIRE(linkedlist_get(&list, i, &value) == LINKEDLIST_SUCCESS);
        REQUIRE(value == i);
    }

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_add() correctly adds elements with custom copy
 * function to list.
 */
TEST_CASE("add() custom", "[linkedlist_add(), custom]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(ByteBuffer), &bytebuffer_copy,
                                  &bytebuffer_dealloc) == LINKEDLIST_SUCCESS);

    for (int i = 0; i < 5; i++)
    {
        // Check length before call
        size_t length = 0;
        REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
        REQUIRE(length == i);

        // Add item
        uint8_t data[] = {
            ((uint8_t) ((0x00 + i) & 0xff)), ((uint8_t) ((0x01 + i) & 0xff)),
            ((uint8_t) ((0x02 + i) & 0xff)), ((uint8_t) ((0x03 + i) & 0xff))};
        ByteBuffer buffer = {sizeof(data), data};
        REQUIRE(linkedlist_add(&list, &buffer) == LINKEDLIST_SUCCESS);

        // Check length after call
        REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
        REQUIRE(length == i + 1);

        // Check that item was correctly set
        ByteBuffer check;
        REQUIRE(linkedlist_get(&list, i, &check) == LINKEDLIST_SUCCESS);
        REQUIRE(buffer.length == check.length);
        REQUIRE(memcmp(buffer.data, check.data, buffer.length) == 0);
        bytebuffer_dealloc(&check);
    }

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_add() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("add() list NULL", "[linkedlist_add(), error]")
{
    int value = 42;
    REQUIRE(linkedlist_add(NULL, &value) == EINVAL);
}

/**
 * \test Tests that linkedlist_add() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("add() value NULL", "[linkedlist_add(), error]")
{
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    REQUIRE(linkedlist_add(&list, NULL) == EINVAL);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_add() correctly detects destroyed list.
 */
TEST_CASE("add() destroyed list", "[linkedlist_add(), error]")
{
    // Prepare empty list and immediately destroy it
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    linkedlist_destroy(&list);

    // Make sure add() correctly detects invalid list
    int value = 42;
    REQUIRE(linkedlist_add(&list, &value) == EINVAL);
}

/**
 * \test Tests that linkedlist_remove() correctly removes elements from
 * list.
 */
TEST_CASE("remove()", "[linkedlist_remove()]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Add some items to the list
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);
    }
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Remove middle item
    REQUIRE(linkedlist_remove(&list, 1) == LINKEDLIST_SUCCESS);
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 2);

    // Remove last item
    REQUIRE(linkedlist_remove(&list, 1) == LINKEDLIST_SUCCESS);
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 1);

    // Remove first item
    REQUIRE(linkedlist_remove(&list, 0) == LINKEDLIST_SUCCESS);
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 0);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_remove() correctly removes elements with custom
 * dealloc function from list.
 */
TEST_CASE("remove() custom", "[linkedlist_remove(), custom]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(ByteBuffer), &bytebuffer_copy,
                                  &bytebuffer_dealloc) == LINKEDLIST_SUCCESS);

    // Add some items to the list
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03};
    for (int i = 0; i < 3; i++)
    {
        ByteBuffer buffer = {sizeof(data), data};
        REQUIRE(linkedlist_add(&list, &buffer) == LINKEDLIST_SUCCESS);
    }
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Remove middle item
    REQUIRE(linkedlist_remove(&list, 1) == LINKEDLIST_SUCCESS);
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 2);

    // Remove last item
    REQUIRE(linkedlist_remove(&list, 1) == LINKEDLIST_SUCCESS);
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 1);

    // Remove first item
    REQUIRE(linkedlist_remove(&list, 0) == LINKEDLIST_SUCCESS);
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 0);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_remove() correctly removes first element from
 * list.
 */
TEST_CASE("remove() first", "[linkedlist_remove()]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Add some item to the list
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);
    }
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Remove first item
    REQUIRE(linkedlist_remove(&list, 0) == LINKEDLIST_SUCCESS);

    // Check length has been updated correctly
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 2);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_remove() correctly removes element from the
 * middle of the list.
 */
TEST_CASE("remove() middle", "[linkedlist_remove()]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Add some item to the list
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);
    }
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Remove last item
    REQUIRE(linkedlist_remove(&list, 1) == LINKEDLIST_SUCCESS);

    // Check length has been updated correctly
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 2);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_remove() correctly removes last element from
 * list.
 */
TEST_CASE("remove() last", "[linkedlist_remove()]")
{
    // Initialize list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Add some item to the list
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);
    }
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Remove last item
    REQUIRE(linkedlist_remove(&list, 2) == LINKEDLIST_SUCCESS);

    // Check length has been updated correctly
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 2);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_remove() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("remove() list NULL", "[linkedlist_remove(), error]")
{
    REQUIRE(linkedlist_remove(NULL, 0) == EINVAL);
}

/**
 * \test Tests that linkedlist_remove() correctly detects invalid index.
 */
TEST_CASE("remove() invalid index", "[linkedlist_remove(), error]")
{
    // Add some items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);
    }

    // Make sure remove() correctly detects invalid index
    REQUIRE(linkedlist_remove(&list, 5) == ERANGE);
    REQUIRE(linkedlist_remove(&list, 4) == ERANGE);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_remove() correctly detects empty list.
 */
TEST_CASE("remove() empty list", "[linkedlist_remove(), error]")
{
    // Prepare empty list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Make sure remove() correctly detects invalid index
    REQUIRE(linkedlist_remove(&list, 0) == ERANGE);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_remove() correctly detects destroyed list.
 */
TEST_CASE("remove() destroyed list", "[linkedlist_remove(), error]")
{
    // Prepare empty list and immediately destroy it
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    linkedlist_destroy(&list);

    // Make sure remove() correctly detects invalid list
    REQUIRE(linkedlist_remove(&list, 0) == EINVAL);
}

/**
 * \test Tests that linkedlist_get() correctly returns values from list.
 */
TEST_CASE("get()", "[linkedlist_get()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Check that all values are set correctly
    for (unsigned int i = 0; i < 3; i++)
    {
        int value = 0;
        REQUIRE(linkedlist_get(&list, i, &value) == LINKEDLIST_SUCCESS);
        REQUIRE(value == (42 + i));
    }

    // Re-check length of list just to be sure
    length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_get() correctly returns values with custom copy
 * function from list.
 */
TEST_CASE("get() custom", "[linkedlist_get(), custom]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(ByteBuffer), &bytebuffer_copy,
                                  &bytebuffer_dealloc) == LINKEDLIST_SUCCESS);
    for (int i = 0; i < 3; i++)
    {
        // Add item
        uint8_t data[] = {
            ((uint8_t) ((0x00 + i) & 0xff)), ((uint8_t) ((0x01 + i) & 0xff)),
            ((uint8_t) ((0x02 + i) & 0xff)), ((uint8_t) ((0x03 + i) & 0xff))};
        ByteBuffer buffer = {sizeof(data), data};
        REQUIRE(linkedlist_add(&list, &buffer) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Check that all values are set correctly
    for (unsigned int i = 0; i < 3; i++)
    {
        uint8_t expected_data[] = {
            ((uint8_t) ((0x00 + i) & 0xff)), ((uint8_t) ((0x01 + i) & 0xff)),
            ((uint8_t) ((0x02 + i) & 0xff)), ((uint8_t) ((0x03 + i) & 0xff))};
        ByteBuffer expected = {sizeof(expected_data), expected_data};
        ByteBuffer value;
        REQUIRE(linkedlist_get(&list, i, &value) == LINKEDLIST_SUCCESS);
        REQUIRE(expected.length == value.length);
        REQUIRE(memcmp(expected.data, value.data, expected.length) == 0);
        bytebuffer_dealloc(&value);
    }

    // Re-check length of list just to be sure
    length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_get() correctly returns first value from
 * list.
 */
TEST_CASE("get() first", "[linkedlist_get()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Get first item from list
    int value = 0;
    REQUIRE(linkedlist_get(&list, 0, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == 42);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_get() correctly returns value from middle of
 * list.
 */
TEST_CASE("get() middle", "[linkedlist_get()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Get middle item from list
    int value = 0;
    REQUIRE(linkedlist_get(&list, 1, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == (42 + 1));

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_get() correctly returns value from end of
 * list.
 */
TEST_CASE("get() last", "[linkedlist_get()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Get last item from list
    int value = 0;
    REQUIRE(linkedlist_get(&list, 2, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == (42 + 2));

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_get() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("get() list NULL", "[linkedlist_get(), error]")
{
    int value;
    REQUIRE(linkedlist_get(NULL, 0, &value) == EINVAL);
}

/**
 * \test Tests that linkedlist_get() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("get() buffer NULL", "[linkedlist_get(), error]")
{
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    REQUIRE(linkedlist_get(&list, 0, NULL) == EINVAL);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_get() correctly detects invalid index.
 */
TEST_CASE("get() invalid index", "[linkedlist_get(), error]")
{
    // Add some items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);
    }

    // Make sure get() correctly detects invalid index
    int value = 0;
    REQUIRE(linkedlist_get(&list, 5, &value) == ERANGE);
    REQUIRE(linkedlist_get(&list, 4, &value) == ERANGE);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_get() correctly detects empty list.
 */
TEST_CASE("get() empty list", "[linkedlist_get(), error]")
{
    // Prepare empty list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Make sure get() correctly detects invalid index
    int value = 0;
    REQUIRE(linkedlist_get(&list, 0, &value) == ERANGE);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_get() correctly detects destroyed list.
 */
TEST_CASE("get() destroyed list", "[linkedlist_get(), error]")
{
    // Prepare empty list and immediately destroy it
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    linkedlist_destroy(&list);

    // Make sure get() correctly detects invalid list
    int value = 0;
    REQUIRE(linkedlist_get(&list, 0, &value) == EINVAL);
}

/**
 * \test Tests that linkedlist_update() correctly updates values in list.
 */
TEST_CASE("update()", "[linkedlist_update()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Check that all values are set correctly
    for (unsigned int i = 0; i < 3; i++)
    {
        int value = 0;
        REQUIRE(linkedlist_get(&list, i, &value) == LINKEDLIST_SUCCESS);
        REQUIRE(value == (42 + i));
    }

    // Update values in list
    for (unsigned int i = 0; i < 3; i++)
    {
        int value = 69 + i;
        REQUIRE(linkedlist_update(&list, i, &value) == LINKEDLIST_SUCCESS);
    }

    // Re-check length of list just to be sure
    length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Check that all values are updated correctly
    for (unsigned int i = 0; i < 3; i++)
    {
        int value = 0;
        REQUIRE(linkedlist_get(&list, i, &value) == LINKEDLIST_SUCCESS);
        REQUIRE(value == (69 + i));
    }

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_update() correctly updates values with custom
 * copy and dealloc functions in list.
 */
TEST_CASE("update() custom", "[linkedlist_update(), custom]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(ByteBuffer), &bytebuffer_copy,
                                  &bytebuffer_dealloc) == LINKEDLIST_SUCCESS);
    for (int i = 0; i < 3; i++)
    {
        uint8_t data[] = {
            ((uint8_t) ((0x00 + i) & 0xff)), ((uint8_t) ((0x01 + i) & 0xff)),
            ((uint8_t) ((0x02 + i) & 0xff)), ((uint8_t) ((0x03 + i) & 0xff))};
        ByteBuffer buffer = {sizeof(data), data};
        REQUIRE(linkedlist_add(&list, &buffer) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Check that all values are set correctly
    for (unsigned int i = 0; i < 3; i++)
    {
        uint8_t expected_data[] = {
            ((uint8_t) ((0x00 + i) & 0xff)), ((uint8_t) ((0x01 + i) & 0xff)),
            ((uint8_t) ((0x02 + i) & 0xff)), ((uint8_t) ((0x03 + i) & 0xff))};
        ByteBuffer expected = {sizeof(expected_data), expected_data};
        ByteBuffer value;
        REQUIRE(linkedlist_get(&list, i, &value) == LINKEDLIST_SUCCESS);
        REQUIRE(expected.length == value.length);
        REQUIRE(memcmp(expected.data, value.data, expected.length) == 0);
        bytebuffer_dealloc(&value);
    }

    // Update values in list
    for (unsigned int i = 0; i < 3; i++)
    {
        uint8_t data[] = {
            ((uint8_t) ((0x10 + i) & 0xff)), ((uint8_t) ((0x11 + i) & 0xff)),
            ((uint8_t) ((0x12 + i) & 0xff)), ((uint8_t) ((0x13 + i) & 0xff))};
        ByteBuffer buffer = {sizeof(data), data};
        REQUIRE(linkedlist_update(&list, i, &buffer) == LINKEDLIST_SUCCESS);
    }

    // Re-check length of list just to be sure
    length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Check that all values are updated correctly
    for (unsigned int i = 0; i < 3; i++)
    {
        uint8_t expected_data[] = {
            ((uint8_t) ((0x10 + i) & 0xff)), ((uint8_t) ((0x11 + i) & 0xff)),
            ((uint8_t) ((0x12 + i) & 0xff)), ((uint8_t) ((0x13 + i) & 0xff))};
        ByteBuffer expected = {sizeof(expected_data), expected_data};
        ByteBuffer value;
        REQUIRE(linkedlist_get(&list, i, &value) == LINKEDLIST_SUCCESS);
        REQUIRE(expected.length == value.length);
        REQUIRE(memcmp(expected.data, value.data, expected.length) == 0);
        bytebuffer_dealloc(&value);
    }

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_update() correctly updates first value in
 * list.
 */
TEST_CASE("update() first", "[linkedlist_update()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Get first item from list
    int value = 0;
    REQUIRE(linkedlist_get(&list, 0, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == 42);

    // Update first item in list
    value = 69;
    REQUIRE(linkedlist_update(&list, 0, &value) == LINKEDLIST_SUCCESS);

    // Check that value was correctly updated
    value = 0;
    REQUIRE(linkedlist_get(&list, 0, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == 69);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_update() correctly updates value in middle of
 * list.
 */
TEST_CASE("update() middle", "[linkedlist_update()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Get item from middle of list
    int value = 0;
    REQUIRE(linkedlist_get(&list, 1, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == (42 + 1));

    // Update item in middle of list
    value = 69;
    REQUIRE(linkedlist_update(&list, 1, &value) == LINKEDLIST_SUCCESS);

    // Check that value was correctly updated
    value = 0;
    REQUIRE(linkedlist_get(&list, 1, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == 69);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_update() correctly updates value at end of
 * list.
 */
TEST_CASE("update() last", "[linkedlist_update()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Get last item from list
    int value = 0;
    REQUIRE(linkedlist_get(&list, 2, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == (42 + 2));

    // Update last item in list
    value = 69;
    REQUIRE(linkedlist_update(&list, 2, &value) == LINKEDLIST_SUCCESS);

    // Check that value was correctly updated
    value = 0;
    REQUIRE(linkedlist_get(&list, 2, &value) == LINKEDLIST_SUCCESS);
    REQUIRE(value == 69);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_update() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("update() list NULL", "[linkedlist_update(), error]")
{
    // Make sure update() correctly detects invalid parameter
    int value = 42;
    REQUIRE(linkedlist_update(NULL, 0, &value) == EINVAL);
}

/**
 * \test Tests that linkedlist_update() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("update() value NULL", "[linkedlist_update(), error]")
{
    // Add some items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int value = 42; value < (42 + 3); value++)
    {
        REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);
    }

    // Make sure update() correctly detects invalid parameter
    REQUIRE(linkedlist_update(&list, 0, NULL) == EINVAL);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_update() correctly detects invalid index.
 */
TEST_CASE("update() invalid index", "[linkedlist_update(), error]")
{
    // Add some items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);
    }

    // Make sure update() correctly detects invalid index
    int value = 0;
    REQUIRE(linkedlist_update(&list, 5, &value) == ERANGE);
    REQUIRE(linkedlist_update(&list, 4, &value) == ERANGE);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_update() correctly detects empty list.
 */
TEST_CASE("update() empty list", "[linkedlist_update(), error]")
{
    // Prepare empty list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Make sure update() correctly detects invalid index
    int value = 0;
    REQUIRE(linkedlist_update(&list, 0, &value) == ERANGE);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_update() correctly detects destroyed list.
 */
TEST_CASE("update() destroyed list", "[linkedlist_update(), error]")
{
    // Prepare empty list and immediately destroy it
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    linkedlist_destroy(&list);

    // Make sure update() correctly detects invalid list
    int value = 0;
    REQUIRE(linkedlist_update(&list, 0, &value) == EINVAL);
}

/**
 * \test Tests that linkedlist_length() correctly calculates length of list.
 */
TEST_CASE("length()", "[linkedlist_length()]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(linkedlist_add(&list, &i) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Add one more item
    int value = 4;
    REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);

    // Check that length has been updated
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 4);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_length() correctly calculates length of list for
 * items with custom copy / deallocation functions.
 */
TEST_CASE("length() custom", "[linkedlist_length(), custom]")
{
    // Add items to list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(ByteBuffer), bytebuffer_copy,
                                  bytebuffer_dealloc) == LINKEDLIST_SUCCESS);
    for (int i = 0; i < 3; i++)
    {
        uint8_t data[] = {
            ((uint8_t) ((0x00 + i) & 0xff)), ((uint8_t) ((0x01 + i) & 0xff)),
            ((uint8_t) ((0x02 + i) & 0xff)), ((uint8_t) ((0x03 + i) & 0xff))};
        ByteBuffer buffer = {sizeof(data), data};
        REQUIRE(linkedlist_add(&list, &buffer) == LINKEDLIST_SUCCESS);
    }

    // Check length of list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 3);

    // Add one more item
    uint8_t data[] = {0x04, 0x05, 0x06, 0x07};
    ByteBuffer value = {sizeof(data), data};
    REQUIRE(linkedlist_add(&list, &value) == LINKEDLIST_SUCCESS);

    // Check that length has been updated
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 4);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_length() correctly detects empty list.
 */
TEST_CASE("length() empty", "[linkedlist_length()]")
{
    // Prepare list
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);

    // Check length of empty list
    size_t length = 1;
    REQUIRE(linkedlist_length(&list, &length) == LINKEDLIST_SUCCESS);
    REQUIRE(length == 0);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_length() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("length() list NULL", "[linkedlist_length(), error]")
{
    size_t length;
    REQUIRE(linkedlist_length(NULL, &length) == EINVAL);
}

/**
 * \test Tests that linkedlist_length() correctly detects invalid \c NULL
 * parameter.
 */
TEST_CASE("length() length NULL", "[linkedlist_length(), error]")
{
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    REQUIRE(linkedlist_length(&list, NULL) == EINVAL);

    // Cleanup
    linkedlist_destroy(&list);
}

/**
 * \test Tests that linkedlist_length() correctly detects destroyed list.
 */
TEST_CASE("length() destroyed list", "[linkedlist_length(), error]")
{
    // Prepare empty list and immediately destroy it
    LinkedList list;
    REQUIRE(linkedlist_initialize(&list, sizeof(int), NULL, NULL) ==
            LINKEDLIST_SUCCESS);
    linkedlist_destroy(&list);

    // Make sure length() correctly detects invalid list
    size_t length = 0;
    REQUIRE(linkedlist_length(&list, &length) == EINVAL);
}
