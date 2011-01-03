#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H



#include <linux/typecheck.h>
#include <linux/preempt.h>
#include <linux/linkage.h>
#include <linux/compiler.h>
#include <linux/thread_info.h>
#include <linux/kernel.h>
#include <linux/stringify.h>
#include <linux/bottom_half.h>

#include <asm/system.h>


#define LOCK_SECTION_NAME ".text.lock."KBUILD_BASENAME

#define LOCK_SECTION_START(extra)               \
        ".subsection 1\n\t"                     \
        extra                                   \
        ".ifndef " LOCK_SECTION_NAME "\n\t"     \
        LOCK_SECTION_NAME ":\n\t"               \
        ".endif\n"

#define LOCK_SECTION_END                        \
        ".previous\n\t"

#define __lockfunc __attribute__((section(".spinlock.text")))


#include <linux/spinlock_types.h>

extern int __lockfunc generic__raw_read_trylock(raw_rwlock_t *lock);


#ifdef CONFIG_SMP
# include <asm/spinlock.h>
#else
# include <linux/spinlock_up.h>
#endif

#ifdef CONFIG_DEBUG_SPINLOCK
  extern void __spin_lock_init(spinlock_t *lock, const char *name,
			       struct lock_class_key *key);
# define spin_lock_init(lock)					\
do {								\
	static struct lock_class_key __key;			\
								\
	__spin_lock_init((lock), #lock, &__key);		\
} while (0)

#else
# define spin_lock_init(lock)					\
	do { *(lock) = SPIN_LOCK_UNLOCKED; } while (0)
#endif

#ifdef CONFIG_DEBUG_SPINLOCK
  extern void __rwlock_init(rwlock_t *lock, const char *name,
			    struct lock_class_key *key);
# define rwlock_init(lock)					\
do {								\
	static struct lock_class_key __key;			\
								\
	__rwlock_init((lock), #lock, &__key);			\
} while (0)
#else
# define rwlock_init(lock)					\
	do { *(lock) = RW_LOCK_UNLOCKED; } while (0)
#endif

#define spin_is_locked(lock)	__raw_spin_is_locked(&(lock)->raw_lock)

#ifdef CONFIG_GENERIC_LOCKBREAK
#define spin_is_contended(lock) ((lock)->break_lock)
#else

#ifdef __raw_spin_is_contended
#define spin_is_contended(lock)	__raw_spin_is_contended(&(lock)->raw_lock)
#else
#define spin_is_contended(lock)	(((void)(lock), 0))
#endif 
#endif


#ifndef ARCH_HAS_SMP_MB_AFTER_LOCK
static inline void smp_mb__after_lock(void) { smp_mb(); }
#endif


#define spin_unlock_wait(lock)	__raw_spin_unlock_wait(&(lock)->raw_lock)

#ifdef CONFIG_DEBUG_SPINLOCK
 extern void _raw_spin_lock(spinlock_t *lock);
#define _raw_spin_lock_flags(lock, flags) _raw_spin_lock(lock)
 extern int _raw_spin_trylock(spinlock_t *lock);
 extern void _raw_spin_unlock(spinlock_t *lock);
 extern void _raw_read_lock(rwlock_t *lock);
#define _raw_read_lock_flags(lock, flags) _raw_read_lock(lock)
 extern int _raw_read_trylock(rwlock_t *lock);
 extern void _raw_read_unlock(rwlock_t *lock);
 extern void _raw_write_lock(rwlock_t *lock);
#define _raw_write_lock_flags(lock, flags) _raw_write_lock(lock)
 extern int _raw_write_trylock(rwlock_t *lock);
 extern void _raw_write_unlock(rwlock_t *lock);
#else
# define _raw_spin_lock(lock)		__raw_spin_lock(&(lock)->raw_lock)
# define _raw_spin_lock_flags(lock, flags) \
		__raw_spin_lock_flags(&(lock)->raw_lock, *(flags))
# define _raw_spin_trylock(lock)	__raw_spin_trylock(&(lock)->raw_lock)
# define _raw_spin_unlock(lock)		__raw_spin_unlock(&(lock)->raw_lock)
# define _raw_read_lock(rwlock)		__raw_read_lock(&(rwlock)->raw_lock)
# define _raw_read_lock_flags(lock, flags) \
		__raw_read_lock_flags(&(lock)->raw_lock, *(flags))
# define _raw_read_trylock(rwlock)	__raw_read_trylock(&(rwlock)->raw_lock)
# define _raw_read_unlock(rwlock)	__raw_read_unlock(&(rwlock)->raw_lock)
# define _raw_write_lock(rwlock)	__raw_write_lock(&(rwlock)->raw_lock)
# define _raw_write_lock_flags(lock, flags) \
		__raw_write_lock_flags(&(lock)->raw_lock, *(flags))
# define _raw_write_trylock(rwlock)	__raw_write_trylock(&(rwlock)->raw_lock)
# define _raw_write_unlock(rwlock)	__raw_write_unlock(&(rwlock)->raw_lock)
#endif

#define read_can_lock(rwlock)		__raw_read_can_lock(&(rwlock)->raw_lock)
#define write_can_lock(rwlock)		__raw_write_can_lock(&(rwlock)->raw_lock)


#define spin_trylock(lock)		__cond_lock(lock, _spin_trylock(lock))
#define read_trylock(lock)		__cond_lock(lock, _read_trylock(lock))
#define write_trylock(lock)		__cond_lock(lock, _write_trylock(lock))

#define spin_lock(lock)			_spin_lock(lock)

#ifdef CONFIG_DEBUG_LOCK_ALLOC
# define spin_lock_nested(lock, subclass) _spin_lock_nested(lock, subclass)
# define spin_lock_nest_lock(lock, nest_lock)				\
	 do {								\
		 typecheck(struct lockdep_map *, &(nest_lock)->dep_map);\
		 _spin_lock_nest_lock(lock, &(nest_lock)->dep_map);	\
	 } while (0)
#else
# define spin_lock_nested(lock, subclass) _spin_lock(lock)
# define spin_lock_nest_lock(lock, nest_lock) _spin_lock(lock)
#endif

#define write_lock(lock)		_write_lock(lock)
#define read_lock(lock)			_read_lock(lock)

#if defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)

#define spin_lock_irqsave(lock, flags)			\
	do {						\
		typecheck(unsigned long, flags);	\
		flags = _spin_lock_irqsave(lock);	\
	} while (0)
#define read_lock_irqsave(lock, flags)			\
	do {						\
		typecheck(unsigned long, flags);	\
		flags = _read_lock_irqsave(lock);	\
	} while (0)
#define write_lock_irqsave(lock, flags)			\
	do {						\
		typecheck(unsigned long, flags);	\
		flags = _write_lock_irqsave(lock);	\
	} while (0)

#ifdef CONFIG_DEBUG_LOCK_ALLOC
#define spin_lock_irqsave_nested(lock, flags, subclass)			\
	do {								\
		typecheck(unsigned long, flags);			\
		flags = _spin_lock_irqsave_nested(lock, subclass);	\
	} while (0)
#else
#define spin_lock_irqsave_nested(lock, flags, subclass)			\
	do {								\
		typecheck(unsigned long, flags);			\
		flags = _spin_lock_irqsave(lock);			\
	} while (0)
#endif

#else

#define spin_lock_irqsave(lock, flags)			\
	do {						\
		typecheck(unsigned long, flags);	\
		_spin_lock_irqsave(lock, flags);	\
	} while (0)
#define read_lock_irqsave(lock, flags)			\
	do {						\
		typecheck(unsigned long, flags);	\
		_read_lock_irqsave(lock, flags);	\
	} while (0)
#define write_lock_irqsave(lock, flags)			\
	do {						\
		typecheck(unsigned long, flags);	\
		_write_lock_irqsave(lock, flags);	\
	} while (0)
#define spin_lock_irqsave_nested(lock, flags, subclass)	\
	spin_lock_irqsave(lock, flags)

#endif

#define spin_lock_irq(lock)		_spin_lock_irq(lock)
#define spin_lock_bh(lock)		_spin_lock_bh(lock)
#define read_lock_irq(lock)		_read_lock_irq(lock)
#define read_lock_bh(lock)		_read_lock_bh(lock)
#define write_lock_irq(lock)		_write_lock_irq(lock)
#define write_lock_bh(lock)		_write_lock_bh(lock)
#define spin_unlock(lock)		_spin_unlock(lock)
#define read_unlock(lock)		_read_unlock(lock)
#define write_unlock(lock)		_write_unlock(lock)
#define spin_unlock_irq(lock)		_spin_unlock_irq(lock)
#define read_unlock_irq(lock)		_read_unlock_irq(lock)
#define write_unlock_irq(lock)		_write_unlock_irq(lock)

#define spin_unlock_irqrestore(lock, flags)		\
	do {						\
		typecheck(unsigned long, flags);	\
		_spin_unlock_irqrestore(lock, flags);	\
	} while (0)
#define spin_unlock_bh(lock)		_spin_unlock_bh(lock)

#define read_unlock_irqrestore(lock, flags)		\
	do {						\
		typecheck(unsigned long, flags);	\
		_read_unlock_irqrestore(lock, flags);	\
	} while (0)
#define read_unlock_bh(lock)		_read_unlock_bh(lock)

#define write_unlock_irqrestore(lock, flags)		\
	do {						\
		typecheck(unsigned long, flags);	\
		_write_unlock_irqrestore(lock, flags);	\
	} while (0)
#define write_unlock_bh(lock)		_write_unlock_bh(lock)

#define spin_trylock_bh(lock)	__cond_lock(lock, _spin_trylock_bh(lock))

#define spin_trylock_irq(lock) \
({ \
	local_irq_disable(); \
	spin_trylock(lock) ? \
	1 : ({ local_irq_enable(); 0;  }); \
})

#define spin_trylock_irqsave(lock, flags) \
({ \
	local_irq_save(flags); \
	spin_trylock(lock) ? \
	1 : ({ local_irq_restore(flags); 0; }); \
})

#define write_trylock_irqsave(lock, flags) \
({ \
	local_irq_save(flags); \
	write_trylock(lock) ? \
	1 : ({ local_irq_restore(flags); 0; }); \
})


#include <asm/atomic.h>

extern int _atomic_dec_and_lock(atomic_t *atomic, spinlock_t *lock);
#define atomic_dec_and_lock(atomic, lock) \
		__cond_lock(lock, _atomic_dec_and_lock(atomic, lock))


#define spin_can_lock(lock)	(!spin_is_locked(lock))


#if defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)
# include <linux/spinlock_api_smp.h>
#else
# include <linux/spinlock_api_up.h>
#endif

#endif 
