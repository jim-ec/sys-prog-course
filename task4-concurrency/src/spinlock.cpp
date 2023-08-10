#include "../cspinlock.h"
#include <atomic>


enum class LockState {
    Locked,
    Unlocked
};


struct SpinLock {
    std::atomic<LockState> lock = LockState::Unlocked;
};


SpinLock* cspin_alloc() {
    return new SpinLock;
}


void cspin_free(SpinLock* spin) {
    delete spin;
}


int cspin_lock(SpinLock *spin) {
    for (;;) {
        auto expected = LockState::Unlocked;
        spin->lock.compare_exchange_weak(expected, LockState::Locked);
        if (expected == LockState::Unlocked) {
            break;
        }
    }
    return 0;
}


int cspin_trylock(SpinLock *spin) {
    auto expected = LockState::Unlocked;
    spin->lock.compare_exchange_weak(expected, LockState::Locked);
    return expected == LockState::Unlocked;
}


int cspin_unlock(SpinLock *spin) {
    auto expected = LockState::Locked;
    spin->lock.compare_exchange_weak(expected, LockState::Unlocked);
    return 0;
}
