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
 * \file linkedlist-example-int.c
 * \author Martin Kloesch <martin.kloesch@gmail.com>
 * \brief Example showing how to use LinkedList for simple integers.
 */

#include <stdio.h>
#include <assert.h>
#include "linkedlist.h"

/**
 * \brief Example showing how to use LinkedList with simple integers.
 */
int main()
{
    // Prepare linked list
    LinkedList list;
    printf("preparing list\n");
    int status = linkedlist_initialize(&list, sizeof(int), NULL, NULL);
    assert(status == LINKEDLIST_SUCCESS);

    // Add items
    int value = 42;
    printf("adding %d\n", value);
    status = linkedlist_add(&list, &value);
    assert(status == LINKEDLIST_SUCCESS);

    value = 69;
    printf("adding %d\n", value);
    status = linkedlist_add(&list, &value);
    assert(status == LINKEDLIST_SUCCESS);

    // Check length of list
    size_t length = 0;
    status = linkedlist_length(&list, &length);
    assert(status == LINKEDLIST_SUCCESS);
    assert(length == 2);
    printf("len(list) -> %zu\n", length);

    // Query value from list
    value = 0;
    unsigned int index = 1;
    status = linkedlist_get(&list, index, &value);
    assert(status == LINKEDLIST_SUCCESS);
    assert(value == 69);
    printf("list[%u] -> %d\n", index, value);

    // Update value in list
    value = 1234;
    printf("updating list[%u] = %d\n", index, value);
    status = linkedlist_update(&list, index, &value);
    assert(status == LINKEDLIST_SUCCESS);

    // Iterate over list
    status = linkedlist_length(&list, &length);
    assert(status == LINKEDLIST_SUCCESS);
    assert(length == 2);
    printf("iterating:\n");
    for (index = 0; index < length; index++)
    {
        status = linkedlist_get(&list, index, &value);
        assert(status == LINKEDLIST_SUCCESS);
        printf("  list[%u]: %d\n", index, value);
    }

    // Perform cleanup
    printf("destroying list\n");
    linkedlist_destroy(&list);

    return 0;
}
