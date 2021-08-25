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
 * \file linkedlist-mutex.h
 * \author Martin Kloesch <martin.kloesch@gmail.com>
 * \brief Generic interface for a thread-safe mutex / lock.
 */

#pragma once

#ifndef LOCK_H_
#define LOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Return code for successful calls to any function in this library.
 */
#define LINKEDLIST_MUTEX_SUCCESS 0

/**
 * \brief Generic mutex / lock interface.
 */
typedef void *linkedlist_mutex_t;

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
 */
int linkedlist_mutex_initialize(linkedlist_mutex_t *lock);

/**
 * \brief Acquires given lock.
 *
 * \details This call might block if another thread already holds the lock.
 * Locking an already held mutex will result in undefined behaviour.
 *
 * \param[in] lock Lock to be acquired.
 * \return int \c LINKEDLIST_MUTEX_SUCCESS if successful, any other value in
 * case of error.
 */
int linkedlist_mutex_lock(linkedlist_mutex_t *lock);

/**
 * \brief Releases given lock.
 *
 * \details Lock must have been acquired previously using
 * linkedlist_mutex_lock(), otherwise this will result in undefined behaviour.
 *
 * \param[in] lock Held lock to be released.
 * \return int \c LINKEDLIST_MUTEX_SUCCESS if successful, any other value in
 * case of error.
 */
int linkedlist_mutex_unlock(linkedlist_mutex_t *lock);

/**
 * \brief Destroys given lock and frees all its data.
 *
 * \details Lock has to have been initialized by linkedlist_mutex_initialize().
 *
 * \param[in] lock Lock to be destroyed.
 */
void linkedlist_mutex_destroy(linkedlist_mutex_t *lock);

#ifdef __cplusplus
}
#endif

#endif // LOCK_H_
