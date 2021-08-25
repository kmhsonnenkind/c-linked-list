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
 * \file linkedlist.h
 * \author Martin Kloesch <martin.kloesch@gmail.com>
 * \brief Thread-safe generic linked-list implementation
 */

#pragma once

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stddef.h>
#include "linkedlist-mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Return code for successful calls to any function in this library.
 */
#define LINKEDLIST_SUCCESS 0

/**
 * \brief Generic data storage for item in LinkedList.
 *
 * \details Populated by linkedlist_add() or linkedlist_update(), do not use
 * manually.
 */
typedef struct _ListItem
{
    /**
     * \brief Value of list item.
     *
     * \details Generic \c void* that needs to be casted to concrete type. It is
     * up to the user to make sure the item is used correctly. Do **NOT** use
     * directly but rather use linkedlist_get() to query value.
     */
    void *_value;

    /**
     * \brief Pointer to next item in list.
     *
     * \details Set internally by list manipulating functions, do **NOT** set
     * manually.
     */
    struct _ListItem *_next;
} ListItem;

/**
 * \brief Custom function pointer for copying an item to the LinkedList.
 *
 * \details By default linkedlist_add() and linkedlist_update() will use
 * memcpy() to copy the data. If an item needs to be copied in a more complex
 * way, a custom function of this type can be provided.
 *
 * \param[out] dest Pointer to store copy in.
 * \param[in] src Original item to be copied.
 * \param[in] n Size of item (will be set to LinkedList._item_size).
 * \return void* Pointer to destination (that is \p dest ).
 * \see memcpy()
 */
typedef void *(*linkedlist_copyfunction_t)(void *dest, const void *src,
                                           size_t n);

/**
 * \brief Custom function pointer for freeing an item's memory.
 *
 * \details By default linkedlist_destroy() will use free() to free its
 * resources. If an item needs more complex deallocation a custom function of
 * this type can be provided. Note that the base storage will be freed
 * automatically so you will only need to free nested members.
 *
 * \param[in] ptr Item's value that should be freed.
 */
typedef void (*linkedlist_deallocfunction_t)(void *ptr);

/** \struct LinkedList
 * \brief Thread-safe linked list.
 *
 * \details Populated by library functions, do not manipulate manually.
 */
typedef struct
{
    /**
     * \brief Current head (first item) of linked list.
     *
     * \details Might be \c NULL if list is empty.
     */
    ListItem *_head;

    /**
     * \brief Size of a single item's value.
     *
     * \details This is required to know how much data should be allocated and
     * copied when adding data to the list.
     */
    size_t _item_size;

    /**
     * \brief Custom function for copying an item's data.
     *
     * \details If set to \c NULL memcpy() will be used.
     *
     * \see linkedlist_copyfunction_t
     */
    linkedlist_copyfunction_t _copy;

    /**
     * \brief Custom function for freeing an item's data.
     *
     * \details The base memory for the item will be dynamically allocated by
     * library functions. If your item type is more complex and LinkedList._copy
     * allocates data this should be freed by this function. Can be set to
     * \c NULL if not custom deallocation is required.
     *
     * \see linkedlist_deallocfunction_t
     */
    linkedlist_deallocfunction_t _dealloc;

    /**
     * \brief Lock for thread-safe access to members.
     */
    linkedlist_mutex_t _lock;
} LinkedList;

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
                          linkedlist_deallocfunction_t dealloc);

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
int linkedlist_add(LinkedList *list, void *value);

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
int linkedlist_remove(LinkedList *list, unsigned int index);

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
int linkedlist_get(LinkedList *list, unsigned int index, void *buffer);

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
int linkedlist_update(LinkedList *list, unsigned int index, void *value);

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
int linkedlist_length(LinkedList *list, size_t *length_buffer);

/**
 * \brief Destroys LinkedList and frees memory of all its items.
 *
 * \param[in] list LinkedList to be destroyed.
 */
void linkedlist_destroy(LinkedList *list);

#ifdef __cplusplus
}
#endif

#endif // LINKEDLIST_H_
