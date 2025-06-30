#ifdef _WIN32
#include <windows.h>
typedef SRWLOCK rw_lock_t;
#define RW_LOCK_INIT(lock) InitializeSRWLock(lock)
#define RW_READ_LOCK(lock) AcquireSRWLockShared(lock)
#define RW_WRITE_LOCK(lock) AcquireSRWLockExclusive(lock)
#define RW_READ_UNLOCK(lock) ReleaseSRWLockShared(lock)
#define RW_WRITE_UNLOCK(lock) ReleaseSRWLockExclusive(lock)
#define RW_DESTROY_LOCK(lock)
#else
#include <pthread.h>
typedef pthread_rwlock_t rw_lock_t;
#define RW_LOCK_INIT(lock) pthread_rwlock_init(lock, NULL)
#define RW_READ_LOCK(lock) pthread_rwlock_rdlock(lock)
#define RW_WRITE_LOCK(lock) pthread_rwlock_wrlock(lock)
#define RW_READ_UNLOCK(lock) pthread_rwlock_unlock(lock)
#define RW_WRITE_UNLOCK(lock) pthread_rwlock_unlock(lock)
#define RW_DESTROY_LOCK(lock) pthread_rwlock_destroy(lock)
#endif