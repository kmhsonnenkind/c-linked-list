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
 * \file linkedlist-example-custom.c
 * \author Martin Kloesch <martin.kloesch@gmail.com>
 * \brief Example showing how to use LinkedList with custom complex data types.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "linkedlist.h"

/** \struct ByteBuffer
 * \brief Custom data structure for a variable length byte buffer.
 */
typedef struct
{
    /**
     * \brief Number of bytes in the byte buffer.
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
 * \param[in] n Ignored.
 * \return void* \p dest pointer, \c NULL in case of error.
 */
static void *bytebuffer_copy(void *dest, const void *src,
                             __attribute__((unused)) size_t n)
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
        dest_buffer->data = malloc(src_buffer->length);
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
 * \brief Static data buffer for bytebuffer_to_string().
 */
static char FORMAT_BUFFER[1024];

/**
 * \brief Returns string representation of given ByteBuffer.
 *
 * \details Uses static buffer \ref FORMAT_BUFFER so might not work for larger
 * buffers. Implemented without dynamic memory management to simplify handling
 * in example.
 * Raises assertion error if out of memory or invalid data given.
 *
 * \param[in] buffer ByteBuffer to be converted to string.
 * \returns char* String representation of \p buffer.
 */
static char *bytebuffer_to_string(ByteBuffer *buffer)
{
    // Validate parameter
    assert(buffer != NULL);
    assert((buffer->length == 0) || (buffer->data != NULL));

    // Empty buffer
    if (buffer->length == 0)
    {
        return "[]";
    }

    // Dynamically format data
    // '[', 4 characters per byte, ',' in between, ']', '\x00'
    size_t string_length =
        1 + (buffer->length * 4) + (buffer->length - 1) + 1 + 1;
    assert(string_length <= sizeof(FORMAT_BUFFER));
    FORMAT_BUFFER[0] = '[';
    for (size_t i = 0; i < buffer->length; i++)
    {
        sprintf(1 + FORMAT_BUFFER + i * (4 + 1), "0x%02X,", buffer->data[i]);
    }
    FORMAT_BUFFER[string_length - 1 - 1] = ']';
    FORMAT_BUFFER[string_length - 1] = '\x00';
    return FORMAT_BUFFER;
}

/**
 * \brief Example showing how to use LinkedList with custom complex data types.
 */
int main()
{
    // Prepare linked list
    LinkedList list;
    printf("preparing list\n");
    int status = linkedlist_initialize(&list, sizeof(ByteBuffer),
                                       &bytebuffer_copy, &bytebuffer_dealloc);
    assert(status == LINKEDLIST_SUCCESS);

    // Add items
    uint8_t data_1[] = {0x01, 0x02, 0x03, 0x04};
    ByteBuffer buffer = {.length = sizeof(data_1), .data = data_1};
    printf("adding %s\n", bytebuffer_to_string(&buffer));
    status = linkedlist_add(&list, &buffer);
    assert(status == LINKEDLIST_SUCCESS);

    uint8_t data_2[] = {0x11, 0x12, 0x13, 0x14};
    buffer.length = sizeof(data_2);
    buffer.data = data_2;
    printf("adding %s\n", bytebuffer_to_string(&buffer));
    status = linkedlist_add(&list, &buffer);
    assert(status == LINKEDLIST_SUCCESS);

    // Check length of list
    size_t length = 0;
    status = linkedlist_length(&list, &length);
    assert(status == LINKEDLIST_SUCCESS);
    assert(length == 2);
    printf("len(list) -> %zu\n", length);

    // Query value from list
    buffer.length = 0;
    buffer.data = NULL;
    unsigned int index = 1;
    status = linkedlist_get(&list, index, &buffer);
    assert(status == LINKEDLIST_SUCCESS);
    assert(buffer.length == sizeof(data_2));
    assert(memcmp(buffer.data, data_2, buffer.length) == 0);
    printf("list[%u] -> %s\n", index, bytebuffer_to_string(&buffer));

    // Note that returned copy needs to be freed explicitly
    bytebuffer_dealloc(&buffer);

    // Update value in list
    uint8_t data_3[] = {0x21, 0x22, 0x23, 0x24};
    buffer.length = sizeof(data_3);
    buffer.data = data_3;
    printf("updating list[%u] = %s\n", index, bytebuffer_to_string(&buffer));
    status = linkedlist_update(&list, index, &buffer);
    assert(status == LINKEDLIST_SUCCESS);

    // Iterate over list
    status = linkedlist_length(&list, &length);
    assert(status == LINKEDLIST_SUCCESS);
    assert(length == 2);
    printf("iterating:\n");
    for (index = 0; index < length; index++)
    {
        status = linkedlist_get(&list, index, &buffer);
        assert(status == LINKEDLIST_SUCCESS);
        printf("  list[%u]: %s\n", index, bytebuffer_to_string(&buffer));

        // Note that returned copy needs to be freed explicitly
        bytebuffer_dealloc(&buffer);
    }

    // Perform cleanup
    printf("destroying list\n");
    linkedlist_destroy(&list);

    return 0;
}
