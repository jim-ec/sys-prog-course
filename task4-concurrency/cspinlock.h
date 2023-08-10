#ifdef __cplusplus
extern "C" {
#endif


typedef struct SpinLock cspinlock_t;


/// Acquire the lock.
extern int cspin_lock(cspinlock_t *spin);

/// If the lock can not be acquired, return immediately.
extern int cspin_trylock(cspinlock_t *spin);

/// Release the lock.
extern int cspin_unlock(cspinlock_t *spin);

/// Allocate a lock.
extern cspinlock_t* cspin_alloc();

/// Free a lock.
extern void cspin_free(cspinlock_t* spin);


#ifdef __cplusplus
}
#endif
