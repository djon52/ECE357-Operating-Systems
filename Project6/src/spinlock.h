#ifndef __SPINLOCK_H
void spin_lock(volatile char *lock);
void spin_unlock(volatile char *lock);
#define __SPINLOCK_H
#endif