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
 * \file linkedlist.c
 * \author Martin Kloesch <martin.kloesch@gmail.com>
 * \brief Thread-safe linked-list implementation
 */
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "linkedlist.h"

/**
 * \brief Initializes LinkedList for usage.
 *
 * \details This function must be called before any other library function.
 * If a list was successfully initialized it also needs to be destroyed by
 * calling linkedlist_destroy(). Sets up internal structures and asures correct
 * behaviour of list.
 *
 * \param[in] list LinkedList object to be initialized.
 * \param[in] item_size Size of a single item's data.
 * \param[in] copy Custom function for copying an item's data. If member data
 * needs more complex copying mechanism you can provide a custom function that
 * performs the copy here, otherwise use \c NULL for standard memcpy().
 * \param[in] dealloc Custom function for freeing an item's data. If \p copy
 * performs any allocation free the data using this function otherwise use \c
 * NULL if no cleanup necessary.
 * \return int \c LINKEDLIST_SUCCESS if successful, any other value in case of
 * error.
 * \retval LINKEDLIST_SUCCESS If list was successfully initialized.
 * \retval EINVAL If invalid parameter given.
 */
int linkedlist_initialize(LinkedList *list, size_t item_size,
                          linkedlist_copyfunction_t copy,
                          linkedlist_deallocfunction_t dealloc)
{
    // Validate parameters
    if ((list == NULL) || (item_size == 0))
    {
        return EINVAL;
    }

    // Prepare lock for thread-safe access
    int status = pthread_mutex_init(&list->_lock, NULL);
    if (status != 0)
    {
        return status; // LCOV_EXCL_LINE
    }

    // Set initial member values
    list->_head = NULL;
    list->_item_size = item_size;
    list->_copy = (copy != NULL) ? copy : memcpy;
    list->_dealloc = dealloc;

    return LINKEDLIST_SUCCESS;
}

/**
 * \brief Checks that given LinkedList is correctly initialized.
 *
 * \param[in] list LinkedList to be validated.
 * \return bool \c true if list is correctly initialized, \c false otherwise.
 */
static bool linkedlist_initialized(LinkedList *list)
{
    return (list != NULL) && (list->_item_size != 0) && (list->_copy != NULL);
}

/**
 * \brief Adds item to LinkedList.
 *
 * \details Items are copied to list and need to be freed by calling
 * linkedlist_destroy(). As the data is copied, changes to the original object
 * will not result in changes in the list's item.
 * Note that \c NULL is not a valid value and cannot be stored in the list.
 *
 * \param[in] list LinkedList to add item to.
 * \param[in] value Value to be added.
 * \return int \c LINKEDLIST_SUCCESS if successful, any other value in case of
 * error.
 * \retval LINKEDLIST_SUCCESS If item was successfully added.
 * \retval EINVAL If invalid list given.
 * \retval ENOMEM If no more memory available for new item.
 */
int linkedlist_add(LinkedList *list, void *value)
{
    // Validate list is initialized
    if (!linkedlist_initialized(list))
    {
        return EINVAL;
    }

    // Validate value
    if (value == NULL)
    {
        return EINVAL;
    }

    // Create new linked list item
    ListItem *item = malloc(sizeof(ListItem));
    if (item == NULL)
    {
        return ENOMEM; // LCOV_EXCL_LINE
    }
    item->_next = NULL;

    // Copy value to item
    item->_value = malloc(list->_item_size);
    if (item->_value == NULL)
    {
        // LCOV_EXCL_START
        free(item);
        return ENOMEM;
        // LCOV_EXCL_STOP
    }
    list->_copy(item->_value, value, list->_item_size);

    // Update linked list
    int status = pthread_mutex_lock(&list->_lock);
    if (status != 0)
    {
        // LCOV_EXCL_START
        if (list->_dealloc != NULL)
        {
            list->_dealloc(item->_value);
        }
        free(item->_value);
        free(item);
        return status;
        // LCOV_EXCL_STOP
    }
    if (list->_head == NULL)
    {
        // Just use new item as first item in list
        list->_head = item;
    }
    else
    {
        // Otherwise append to end of list
        ListItem *current = list->_head;
        while (current->_next != NULL)
        {
            current = current->_next;
        }
        current->_next = item;
    }
    pthread_mutex_unlock(&list->_lock);

    return LINKEDLIST_SUCCESS;
}

/**
 * \brief Removes item from LinkedList.
 *
 * \details Item will be removed and following indices updated accordingly.
 *
 * \param[in] list LinkedList to remove item from.
 * \param[in] index Index of item to be removed.
 * \return int \c LINKEDLIST_SUCCESS if successful, any other value in case of
 * error.
 * \retval LINKEDLIST_SUCCESS If item was successfully removed.
 * \retval EINVAL If invalid list given.
 * \retval ERANGE If given index is out of bounds.
 */
int linkedlist_remove(LinkedList *list, unsigned int index)
{
    // Validate list is initialized
    if (!linkedlist_initialized(list))
    {
        return EINVAL;
    }

    // Find item to be removed
    int status = pthread_mutex_lock(&list->_lock);
    if (status != 0)
    {
        return status; // LCOV_EXCL_LINE
    }
    ListItem *to_be_freed = NULL;
    do
    {
        // Cannot remove from empty list
        if (list->_head == NULL)
        {
            status = ERANGE;
            break;
        }

        // Special case: Remove first item
        if (index == 0)
        {
            // Update linked list
            to_be_freed = list->_head;
            list->_head = list->_head->_next;
            status = LINKEDLIST_SUCCESS;
        }
        // Otherwise remove from rest of list
        else
        {
            // Find position to remove item from
            ListItem *current = list->_head;
            for (int _ = 0; _ < (index - 1); _++)
            {
                // End of list reached early
                if (current == NULL)
                {
                    break;
                }
                current = current->_next;
            }

            // Index out of range
            if ((current == NULL) || (current->_next == NULL))
            {
                status = ERANGE;
                break;
            }

            // Update linked list
            to_be_freed = current->_next;
            current->_next = current->_next->_next;
            status = LINKEDLIST_SUCCESS;
        }
    } while (0);
    pthread_mutex_unlock(&list->_lock);

    // Actually free item
    if (to_be_freed != NULL)
    {
        if (to_be_freed->_value != NULL)
        {
            if (list->_dealloc != NULL)
            {
                list->_dealloc(to_be_freed->_value);
            }
            free(to_be_freed->_value);
        }
        to_be_freed->_value = NULL;
        free(to_be_freed);
    }

    return status;
}

/**
 * \brief Returns item from LinkedList.
 *
 * \details Returns copy to avoid memory issues during cleanup. Use
 * linkedlist_update() to update value if you need to change it.
 *
 * \param[in] list LinkedList to get item from.
 * \param[in] index Index in linked list to get value for.
 * \param[out] buffer Buffer to store copy of value in.
 * \return int \c LINKEDLIST_SUCCESS if successful, any other value in case of
 * error.
 * \retval LINKEDLIST_SUCCESS If item was successfully queried.
 * \retval EINVAL If invalid list given.
 * \retval ERANGE If given index is out of bounds.
 */
int linkedlist_get(LinkedList *list, unsigned int index, void *buffer)
{
    // Validate list is initialized
    if (!linkedlist_initialized(list))
    {
        return EINVAL;
    }

    // Validate output parameter
    if (buffer == NULL)
    {
        return EINVAL;
    }

    // Search item in list
    int status = pthread_mutex_lock(&list->_lock);
    if (status != 0)
    {
        return status; // LCOV_EXCL_LINE
    }
    ListItem *item = list->_head;
    for (unsigned int _ = 0; _ < index; _++)
    {
        if (item == NULL)
        {
            break;
        }
        item = item->_next;
    }

    // Item not found
    if (item == NULL)
    {
        pthread_mutex_unlock(&list->_lock);
        return ERANGE;
    }

    // Copy item to output
    list->_copy(buffer, item->_value, list->_item_size);

    pthread_mutex_unlock(&list->_lock);
    return LINKEDLIST_SUCCESS;
}

/**
 * \brief Updates value in LinkedList.
 *
 * \details linkedlist_get() will return a copy of the current value. Use this
 * to override existing value.
 *
 * \param[in] list LinkedList to update item for.
 * \param[in] index Index in linked list to set value for.
 * \param[in] value New value to be set.
 * \return int \c LINKEDLIST_SUCCESS if successful, any other value in case of
 * error.
 * \retval LINKEDLIST_SUCCESS If item was successfully updated.
 * \retval EINVAL If invalid list given.
 * \retval ERANGE If given index is out of bounds.
 */
int linkedlist_update(LinkedList *list, unsigned int index, void *value)
{
    // Validate list is initialized
    if (!linkedlist_initialized(list))
    {
        return EINVAL;
    }

    // Validate value
    if (value == NULL)
    {
        return EINVAL;
    }

    // Search item in list
    int status = pthread_mutex_lock(&list->_lock);
    if (status != 0)
    {
        return status; // LCOV_EXCL_LINE
    }
    ListItem *item = list->_head;
    for (unsigned int _ = 0; _ < index; _++)
    {
        if (item == NULL)
        {
            break;
        }
        item = item->_next;
    }

    // Update item
    do
    {
        // Item not found
        if (item == NULL)
        {
            status = ERANGE;
            break;
        }

        // Free memory for existing item
        if (list->_dealloc != NULL)
        {
            list->_dealloc(item->_value);
        }

        // Update value
        list->_copy(item->_value, value, list->_item_size);
        status = LINKEDLIST_SUCCESS;
    } while (0);

    pthread_mutex_unlock(&list->_lock);
    return status;
}

/**
 * \brief Returns current length of given LinkedList (current number of items).
 *
 * \param[in] list LinkedList to get length from.
 * \param[out] length_buffer Buffer to store length in.
 * \return int \c LINKEDLIST_SUCCESS if successful, any other value in case of
 * error.
 * \retval LINKEDLIST_SUCCESS If length successfully queried.
 * \retval EINVAL If invalid list or buffer given.
 */
int linkedlist_length(LinkedList *list, size_t *length_buffer)
{
    // Validate list is initialized
    if (!linkedlist_initialized(list))
    {
        return EINVAL;
    }

    // Validate output parameter
    if (length_buffer == NULL)
    {
        return EINVAL;
    }

    // Iterate through all elements to get length
    int status = pthread_mutex_lock(&list->_lock);
    if (status != 0)
    {
        return status; // LCOV_EXCL_LINE
    }
    *length_buffer = 0;
    ListItem *current = list->_head;
    while (current != NULL)
    {
        *length_buffer += 1;
        current = current->_next;
    }
    pthread_mutex_unlock(&list->_lock);

    return LINKEDLIST_SUCCESS;
}

/**
 * \brief Destroys LinkedList and frees memory of all its items.
 *
 * \param[in] list LinkedList to be destroyed.
 */
void linkedlist_destroy(LinkedList *list)
{
    if (list != NULL)
    {
        // Delete all items
        ListItem *current = list->_head;
        while (current != NULL)
        {
            ListItem *to_be_freed = current;
            current = current->_next;
            if (to_be_freed->_value != NULL)
            {
                if (list->_dealloc != NULL)
                {
                    list->_dealloc(to_be_freed->_value);
                }
                free(to_be_freed->_value);
            }
            free(to_be_freed);
        }

        // Destroy lock
        pthread_mutex_destroy(&list->_lock);

        // Invalidate list
        list->_head = NULL;
        list->_copy = NULL;
        list->_dealloc = NULL;
    }
}
