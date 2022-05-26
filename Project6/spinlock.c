#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h> 
#include <ctype.h>
#include <stdbool.h>
#include <sched.h>

#include "tas.h"
#include "spinlock.h"
void spin_lock(volatile char *lock)
{
	while(tas(lock) != 0)
		sched_yield();
}
void spin_unlock(volatile char *lock)
{
	*lock = 0;
}

