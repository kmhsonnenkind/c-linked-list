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
 * \file win32-mutex.c
 * \author Martin Kloesch <martin.kloesch@gmail.com>
 * \brief Mutex implementation based on win32 API.
 */

#include <windows.h>
#include <errno.h>
#include "linkedlist-mutex.h"

/**
 * \brief Initializes given mutex.
 *
 * \details This function must be called before any other library function.
 * If a mutex was successfully initialized it also needs to be destroyed by
 * calling linkedlist_mutex_destroy(). Sets up internal structures and asures
 * correct behaviour of mutex.
 *
 * \param[in] lock Lock to be initialized.
 * \return int \c LINKEDLIST_MUTEX_SUCCESS if successful, any other value in
 * case of error.
 * \see CreateMutex()
 */
int linkedlist_mutex_initialize(linkedlist_mutex_t *lock)
{
    // Validate parameters
    if (lock == NULL)
    {
        return EINVAL;
    }

    // Initialize native mutex
    *lock = CreateMutex(NULL, FALSE, NULL);
    if ((*lock) == NULL)
    {
        return ENOMEM;
    }

    return LINKEDLIST_MUTEX_SUCCESS;
}

/**
 * \brief Acquires given lock.
 *
 * \details This call might block if another thread already holds the lock.
 * Locking an already held mutex will result in undefined behaviour.
 *
 * \param[in] lock Lock to be acquired.
 * \return int \c LINKEDLIST_MUTEX_SUCCESS if successful, any other value in
 * case of error.
 * \see WaitForSingleObject()
 */
int linkedlist_mutex_lock(linkedlist_mutex_t *lock)
{
    // Validate parameters
    if ((lock == NULL) || ((*lock) == NULL))
    {
        return EINVAL;
    }

    // Wrap to native function
    int status = WaitForSingleObject(*lock, INFINITE);
    if (status != WAIT_OBJECT_0)
    {
        return status;
    }
    return LINKEDLIST_MUTEX_SUCCESS;
}

/**
 * \brief Releases given lock.
 *
 * \details Lock must have been acquired previously using
 * linkedlist_mutex_lock(), otherwise this will result in undefined behaviour.
 *
 * \param[in] lock Held lock to be released.
 * \return int \c LINKEDLIST_MUTEX_SUCCESS if successful, any other value in
 * case of error.
 * \see ReleaseMutex()
 */
int linkedlist_mutex_unlock(linkedlist_mutex_t *lock)
{
    // Validate parameters
    if ((lock == NULL) || ((*lock) == NULL))
    {
        return EINVAL;
    }

    // Wrap to native function
    int status = ReleaseMutex(*lock);
    if (!status)
    {
        return EINVAL;
    }
    return LINKEDLIST_MUTEX_SUCCESS;
}

/**
 * \brief Destroys given lock and frees all its data.
 *
 * \details Lock has to have been initialized by linkedlist_mutex_initialize().
 *
 * \param[in] lock Lock to be destroyed.
 * \see CloseHandle()
 */
void linkedlist_mutex_destroy(linkedlist_mutex_t *lock)
{
    if ((lock != NULL) && ((*lock) != NULL))
    {
        // Destroy native object
        CloseHandle(*lock);

        // Invalidate lock
        *lock = NULL;
    }
}
