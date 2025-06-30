#ifndef LOCKING_H
#define LOCKING_H

/**
 * @file locking.h
 * @brief Cross-platform read/write lock abstraction.
 *
 * This header defines a minimal wrapper around platform specific
 * read/write lock primitives. It allows the rest of the codebase to use
 * the same API on both Windows and POSIX systems.
 */

#ifdef _WIN32
#include <windows.h>
typedef SRWLOCK rw_lock_t;
#define RW_LOCK_INIT(lock) InitializeSRWLock(lock)          /**< Initialize lock */
#define RW_READ_LOCK(lock) AcquireSRWLockShared(lock)       /**< Acquire read lock */
#define RW_WRITE_LOCK(lock) AcquireSRWLockExclusive(lock)   /**< Acquire write lock */
#define RW_READ_UNLOCK(lock) ReleaseSRWLockShared(lock)     /**< Release read lock */
#define RW_WRITE_UNLOCK(lock) ReleaseSRWLockExclusive(lock) /**< Release write lock */
#define RW_DESTROY_LOCK(lock)                               /**< SRW locks do not need destruction */
#else
#include <pthread.h>
typedef pthread_rwlock_t rw_lock_t;
#define RW_LOCK_INIT(lock) pthread_rwlock_init(lock, NULL) /**< Initialize lock */
#define RW_READ_LOCK(lock) pthread_rwlock_rdlock(lock)      /**< Acquire read lock */
#define RW_WRITE_LOCK(lock) pthread_rwlock_wrlock(lock)     /**< Acquire write lock */
#define RW_READ_UNLOCK(lock) pthread_rwlock_unlock(lock)    /**< Release read lock */
#define RW_WRITE_UNLOCK(lock) pthread_rwlock_unlock(lock)   /**< Release write lock */
#define RW_DESTROY_LOCK(lock) pthread_rwlock_destroy(lock)  /**< Destroy lock */
#endif

#endif /* LOCKING_H */
