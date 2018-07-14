/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include "internal.h"

#include <_simple.h>
#include <mach/mach_vm.h>
#include <unistd.h>
#include <spawn.h>
#include <spawn_private.h>
#include <sys/spawn_internal.h>
#include <sys/ulock.h>

// TODO: remove me when internal.h can include *_private.h itself
#include "workqueue_private.h"
#include "qos_private.h"

static pthread_priority_t _main_qos = QOS_CLASS_UNSPECIFIED;

#define PTHREAD_OVERRIDE_SIGNATURE	(0x6f766572)
#define PTHREAD_OVERRIDE_SIG_DEAD	(0x7265766f)

struct pthread_override_s
{
	uint32_t sig;
	mach_port_t kthread;
	pthread_t pthread;
	pthread_priority_t priority;
	bool malloced;
};

void
_pthread_set_main_qos(pthread_priority_t qos)
{
	_main_qos = qos;
}

int
pthread_attr_set_qos_class_np(pthread_attr_t *__attr,
							  qos_class_t __qos_class,
							  int __relative_priority)
{
	if (!(__pthread_supported_features & PTHREAD_FEATURE_BSDTHREADCTL)) {
		return ENOTSUP;
	}

	if (__relative_priority > 0 || __relative_priority < QOS_MIN_RELATIVE_PRIORITY) {
		return EINVAL;
	}

	int ret = EINVAL;
	if (__attr->sig == _PTHREAD_ATTR_SIG) {
		if (!__attr->schedset) {
			__attr->qosclass = _pthread_priority_make_newest(__qos_class, __relative_priority, 0);
			__attr->qosset = 1;
			ret = 0;
		}
	}

	return ret;
}

int
pthread_attr_get_qos_class_np(pthread_attr_t * __restrict __attr,
							  qos_class_t * __restrict __qos_class,
							  int * __restrict __relative_priority)
{
	if (!(__pthread_supported_features & PTHREAD_FEATURE_BSDTHREADCTL)) {
		return ENOTSUP;
	}

	int ret = EINVAL;
	if (__attr->sig == _PTHREAD_ATTR_SIG) {
		if (__attr->qosset) {
			qos_class_t qos; int relpri;
			_pthread_priority_split_newest(__attr->qosclass, qos, relpri);

			if (__qos_class) { *__qos_class = qos; }
			if (__relative_priority) { *__relative_priority = relpri; }
		} else {
			if (__qos_class) { *__qos_class = 0; }
			if (__relative_priority) { *__relative_priority = 0; }
		}
		ret = 0;
	}

	return ret;
}

int
pthread_set_qos_class_self_np(qos_class_t __qos_class,
							  int __relative_priority)
{
	if (!(__pthread_supported_features & PTHREAD_FEATURE_BSDTHREADCTL)) {
		return ENOTSUP;
	}

	if (__relative_priority > 0 || __relative_priority < QOS_MIN_RELATIVE_PRIORITY) {
		return EINVAL;
	}

	pthread_priority_t priority = _pthread_priority_make_newest(__qos_class, __relative_priority, 0);

	if (__pthread_supported_features & PTHREAD_FEATURE_SETSELF) {
		return _pthread_set_properties_self(_PTHREAD_SET_SELF_QOS_FLAG, priority, 0);
	} else {
		/* We set the thread QoS class in the TSD and then call into the kernel to
		 * read the value out of it and set the QoS class.
		 */
		_pthread_setspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS, priority);
		mach_port_t kport = _pthread_kernel_thread(pthread_self());
		int res = __bsdthread_ctl(BSDTHREAD_CTL_SET_QOS, kport, &pthread_self()->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS], 0);

		if (res == -1) {
			res = errno;
		}

		return res;
	}
}

int
pthread_set_qos_class_np(pthread_t __pthread,
						 qos_class_t __qos_class,
						 int __relative_priority)
{
	if (__pthread != pthread_self()) {
		/* The kext now enforces this anyway, if we check here too, it allows us to call
		 * _pthread_set_properties_self later if we can.
		 */
		return EPERM;
	}

	return pthread_set_qos_class_self_np(__qos_class, __relative_priority);
}

int
pthread_get_qos_class_np(pthread_t __pthread,
						 qos_class_t * __restrict __qos_class,
						 int * __restrict __relative_priority)
{
	if (!(__pthread_supported_features & PTHREAD_FEATURE_BSDTHREADCTL)) {
		return ENOTSUP;
	}

	pthread_priority_t priority;

	if (__pthread == pthread_self()) {
		priority = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS);
	} else {
		priority = __pthread->tsd[_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS];
	}

	qos_class_t qos; int relpri;
	_pthread_priority_split_newest(priority, qos, relpri);

	if (__qos_class) { *__qos_class = qos; }
	if (__relative_priority) { *__relative_priority = relpri; }

	return 0;
}

qos_class_t
qos_class_self(void)
{
	if (!(__pthread_supported_features & PTHREAD_FEATURE_BSDTHREADCTL)) {
		return QOS_CLASS_UNSPECIFIED;
	}

	pthread_priority_t p = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS);
	qos_class_t c = _pthread_priority_get_qos_newest(p);

	return c;
}

qos_class_t
qos_class_main(void)
{
	return _pthread_priority_get_qos_newest(_main_qos);
}

pthread_priority_t
_pthread_qos_class_encode(qos_class_t qos_class, int relative_priority, unsigned long flags)
{
	return _pthread_priority_make_newest(qos_class, relative_priority, flags);
}

qos_class_t
_pthread_qos_class_decode(pthread_priority_t priority, int *relative_priority, unsigned long *flags)
{
	qos_class_t qos; int relpri;

	_pthread_priority_split_newest(priority, qos, relpri);

	if (relative_priority) { *relative_priority = relpri; }
	if (flags) { *flags = _pthread_priority_get_flags(priority); }
	return qos;
}

// Encode a legacy workqueue API priority into a pthread_priority_t. This API
// is deprecated and can be removed when the simulator no longer uses it.
pthread_priority_t
_pthread_qos_class_encode_workqueue(int queue_priority, unsigned long flags)
{
	switch (queue_priority) {
	case WORKQ_HIGH_PRIOQUEUE:
		return _pthread_priority_make_newest(QOS_CLASS_USER_INITIATED, 0, flags);
	case WORKQ_DEFAULT_PRIOQUEUE:
		return _pthread_priority_make_newest(QOS_CLASS_DEFAULT, 0, flags);
	case WORKQ_LOW_PRIOQUEUE:
	case WORKQ_NON_INTERACTIVE_PRIOQUEUE:
		return _pthread_priority_make_newest(QOS_CLASS_UTILITY, 0, flags);
	case WORKQ_BG_PRIOQUEUE:
		return _pthread_priority_make_newest(QOS_CLASS_BACKGROUND, 0, flags);
	/* Legacy dispatch does not use QOS_CLASS_MAINTENANCE, so no need to handle it here */
	default:
		__pthread_abort();
	}
}

int
_pthread_set_properties_self(_pthread_set_flags_t flags, pthread_priority_t priority, mach_port_t voucher)
{
	if (!(__pthread_supported_features & PTHREAD_FEATURE_SETSELF)) {
		return ENOTSUP;
	}

	int rv = __bsdthread_ctl(BSDTHREAD_CTL_SET_SELF, priority, voucher, flags);

	/* Set QoS TSD if we succeeded or only failed the voucher half. */
	if ((flags & _PTHREAD_SET_SELF_QOS_FLAG) != 0) {
		if (rv == 0 || errno == ENOENT) {
			_pthread_setspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS, priority);
		}
	}

	if (rv) {
		rv = errno;
	}
	return rv;
}

int
pthread_set_fixedpriority_self(void)
{
	if (!(__pthread_supported_features & PTHREAD_FEATURE_BSDTHREADCTL)) {
		return ENOTSUP;
	}

	if (__pthread_supported_features & PTHREAD_FEATURE_SETSELF) {
		return _pthread_set_properties_self(_PTHREAD_SET_SELF_FIXEDPRIORITY_FLAG, 0, 0);
	} else {
		return ENOTSUP;
	}
}

int
pthread_set_timeshare_self(void)
{
	if (!(__pthread_supported_features & PTHREAD_FEATURE_BSDTHREADCTL)) {
		return ENOTSUP;
	}

	if (__pthread_supported_features & PTHREAD_FEATURE_SETSELF) {
		return _pthread_set_properties_self(_PTHREAD_SET_SELF_TIMESHARE_FLAG, 0, 0);
	} else {
		return ENOTSUP;
	}
}


pthread_override_t
pthread_override_qos_class_start_np(pthread_t __pthread,  qos_class_t __qos_class, int __relative_priority)
{
	pthread_override_t rv;
	kern_return_t kr;
	int res = 0;

	/* For now, we don't have access to malloc. So we'll have to vm_allocate this, which means the tiny struct is going
	 * to use an entire page.
	 */
	bool did_malloc = true;

	mach_vm_address_t vm_addr = malloc(sizeof(struct pthread_override_s));
	if (!vm_addr) {
		vm_addr = vm_page_size;
		did_malloc = false;

		kr = mach_vm_allocate(mach_task_self(), &vm_addr, round_page(sizeof(struct pthread_override_s)), VM_MAKE_TAG(VM_MEMORY_LIBDISPATCH) | VM_FLAGS_ANYWHERE);
		if (kr != KERN_SUCCESS) {
			errno = ENOMEM;
			return (_Nonnull pthread_override_t) NULL;
		}
	}

	rv = (pthread_override_t)vm_addr;
	rv->sig = PTHREAD_OVERRIDE_SIGNATURE;
	rv->pthread = __pthread;
	rv->kthread = pthread_mach_thread_np(__pthread);
	rv->priority = _pthread_priority_make_newest(__qos_class, __relative_priority, 0);
	rv->malloced = did_malloc;

	/* To ensure that the kernel port that we keep stays valid, we retain it here. */
	kr = mach_port_mod_refs(mach_task_self(), rv->kthread, MACH_PORT_RIGHT_SEND, 1);
	if (kr != KERN_SUCCESS) {
		res = EINVAL;
	}

	if (res == 0) {
		res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_OVERRIDE_START, rv->kthread, rv->priority, (uintptr_t)rv);

		if (res != 0) {
			mach_port_mod_refs(mach_task_self(), rv->kthread, MACH_PORT_RIGHT_SEND, -1);
		}
	}

	if (res != 0) {
		if (did_malloc) {
			free(rv);
		} else {
			mach_vm_deallocate(mach_task_self(), vm_addr, round_page(sizeof(struct pthread_override_s)));
		}
		rv = NULL;
	}
	return (_Nonnull pthread_override_t) rv;
}

int
pthread_override_qos_class_end_np(pthread_override_t override)
{
	kern_return_t kr;
	int res = 0;

	/* Double-free is a fault. Swap the signature and check the old one. */
	if (_pthread_atomic_xchg_uint32_relaxed(&override->sig, PTHREAD_OVERRIDE_SIG_DEAD) != PTHREAD_OVERRIDE_SIGNATURE) {
		__builtin_trap();
	}

	/* Always consumes (and deallocates) the pthread_override_t object given. */
	res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_OVERRIDE_END, override->kthread, (uintptr_t)override, 0);
	if (res == -1) { res = errno; }

	/* EFAULT from the syscall means we underflowed. Crash here. */
	if (res == EFAULT) {
		// <rdar://problem/17645082> Disable the trap-on-underflow, it doesn't co-exist
		// with dispatch resetting override counts on threads.
		//__builtin_trap();
		res = 0;
	}

	kr = mach_port_mod_refs(mach_task_self(), override->kthread, MACH_PORT_RIGHT_SEND, -1);
	if (kr != KERN_SUCCESS) {
		res = EINVAL;
	}

	if (override->malloced) {
		free(override);
	} else {
		kr = mach_vm_deallocate(mach_task_self(), (mach_vm_address_t)override, round_page(sizeof(struct pthread_override_s)));
		if (kr != KERN_SUCCESS) {
			res = EINVAL;
		}
	}

	return res;
}

int
_pthread_qos_override_start_direct(mach_port_t thread, pthread_priority_t priority, void *resource)
{
	int res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_OVERRIDE_START, thread, priority, (uintptr_t)resource);
	if (res == -1) { res = errno; }
	return res;
}

int
_pthread_qos_override_end_direct(mach_port_t thread, void *resource)
{
	int res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_OVERRIDE_END, thread, (uintptr_t)resource, 0);
	if (res == -1) { res = errno; }
	return res;
}

int
_pthread_override_qos_class_start_direct(mach_port_t thread, pthread_priority_t priority)
{
	// use pthread_self as the default per-thread memory allocation to track the override in the kernel
	return _pthread_qos_override_start_direct(thread, priority, pthread_self());
}

int
_pthread_override_qos_class_end_direct(mach_port_t thread)
{
	// use pthread_self as the default per-thread memory allocation to track the override in the kernel
	return _pthread_qos_override_end_direct(thread, pthread_self());
}

int
_pthread_workqueue_override_start_direct(mach_port_t thread, pthread_priority_t priority)
{
	int res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_OVERRIDE_DISPATCH, thread, priority, 0);
	if (res == -1) { res = errno; }
	return res;
}

int
_pthread_workqueue_override_start_direct_check_owner(mach_port_t thread, pthread_priority_t priority, mach_port_t *ulock_addr)
{
#if !TARGET_OS_IPHONE
	static boolean_t kernel_supports_owner_check = TRUE;
	if (!kernel_supports_owner_check) {
		ulock_addr = NULL;
	}
#endif

	for (;;) {
		int res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_OVERRIDE_DISPATCH, thread, priority, ulock_addr);
		if (res == -1) { res = errno; }
#if !TARGET_OS_IPHONE
		if (ulock_addr && res == EINVAL) {
			if ((uintptr_t)ulock_addr % _Alignof(_Atomic uint32_t)) {
				// do not mute bad ulock addresses related errors
				return EINVAL;
			}
			// backward compatibility for the XBS chroot
			// BSDTHREAD_CTL_QOS_OVERRIDE_DISPATCH used to return EINVAL if
			// arg3 was non NULL.
			kernel_supports_owner_check = FALSE;
			ulock_addr = NULL;
			continue;
		}
#endif
		if (ulock_addr && res == EFAULT) {
			// kernel wants us to redrive the call, so while we refault the
			// memory, also revalidate the owner
			uint32_t uval = *(uint32_t volatile *)ulock_addr;
			if (ulock_owner_value_to_port_name(uval) != thread) {
				return ESTALE;
			}
			continue;
		}

		return res;
	}
}

int
_pthread_workqueue_override_reset(void)
{
	int res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_OVERRIDE_RESET, 0, 0, 0);
	if (res == -1) { res = errno; }
	return res;
}

int
_pthread_workqueue_asynchronous_override_add(mach_port_t thread, pthread_priority_t priority, void *resource)
{
	int res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_DISPATCH_ASYNCHRONOUS_OVERRIDE_ADD, thread, priority, (uintptr_t)resource);
	if (res == -1) { res = errno; }
	return res;
}

int
_pthread_workqueue_asynchronous_override_reset_self(void *resource)
{
	int res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_DISPATCH_ASYNCHRONOUS_OVERRIDE_RESET,
							  0 /* !reset_all */,
							  (uintptr_t)resource,
							  0);
	if (res == -1) { res = errno; }
	return res;
}

int
_pthread_workqueue_asynchronous_override_reset_all_self(void)
{
	int res = __bsdthread_ctl(BSDTHREAD_CTL_QOS_DISPATCH_ASYNCHRONOUS_OVERRIDE_RESET,
							  1 /* reset_all */,
							  0,
							  0);
	if (res == -1) { res = errno; }
	return res;
}

static inline uint16_t
_pthread_workqueue_parallelism_for_priority(int qos, unsigned long flags)
{
	int rc = __bsdthread_ctl(BSDTHREAD_CTL_QOS_MAX_PARALLELISM, qos, flags, 0);
	if (os_unlikely(rc == -1)) {
		rc = errno;
		if (rc != EINVAL) {
			PTHREAD_INTERNAL_CRASH(rc, "qos_max_parallelism failed");
		}
		if (flags & _PTHREAD_QOS_PARALLELISM_COUNT_LOGICAL) {
			return *(uint8_t *)_COMM_PAGE_LOGICAL_CPUS;
		} else {
			return *(uint8_t *)_COMM_PAGE_PHYSICAL_CPUS;
		}
	}
	return (uint16_t)rc;
}

int
pthread_qos_max_parallelism(qos_class_t qos, unsigned long flags)
{
	int thread_qos = _pthread_qos_class_to_thread_qos(qos);
	if (thread_qos == THREAD_QOS_UNSPECIFIED) {
		errno = EINVAL;
		return -1;
	}

	unsigned long syscall_flags = _PTHREAD_QOS_PARALLELISM_COUNT_LOGICAL;
	uint16_t *ptr = &_pthread_globals()->qmp_logical[thread_qos];

	if (flags & PTHREAD_MAX_PARALLELISM_PHYSICAL) {
		syscall_flags = 0;
		ptr = &_pthread_globals()->qmp_physical[thread_qos];
	}
	if (*ptr == 0) {
		*ptr = _pthread_workqueue_parallelism_for_priority(thread_qos, syscall_flags);
	}
	return *ptr;
}

int
pthread_time_constraint_max_parallelism(unsigned long flags)
{
	unsigned long syscall_flags = _PTHREAD_QOS_PARALLELISM_COUNT_LOGICAL;
	uint16_t *ptr = &_pthread_globals()->qmp_logical[0];

	if (flags & PTHREAD_MAX_PARALLELISM_PHYSICAL) {
		syscall_flags = 0;
		ptr = &_pthread_globals()->qmp_physical[0];
	}
	if (*ptr == 0) {
		*ptr = _pthread_workqueue_parallelism_for_priority(0,
				syscall_flags | _PTHREAD_QOS_PARALLELISM_REALTIME);
	}
	return *ptr;
}

int
posix_spawnattr_set_qos_class_np(posix_spawnattr_t * __restrict __attr, qos_class_t __qos_class)
{
	switch (__qos_class) {
		case QOS_CLASS_UTILITY:
			return posix_spawnattr_set_qos_clamp_np(__attr, POSIX_SPAWN_PROC_CLAMP_UTILITY);
		case QOS_CLASS_BACKGROUND:
			return posix_spawnattr_set_qos_clamp_np(__attr, POSIX_SPAWN_PROC_CLAMP_BACKGROUND);
		case QOS_CLASS_MAINTENANCE:
			return posix_spawnattr_set_qos_clamp_np(__attr, POSIX_SPAWN_PROC_CLAMP_MAINTENANCE);
		default:
			return EINVAL;
	}
}

int
posix_spawnattr_get_qos_class_np(const posix_spawnattr_t *__restrict __attr, qos_class_t * __restrict __qos_class)
{
	uint64_t clamp;

	if (!__qos_class) {
		return EINVAL;
	}

	int rv = posix_spawnattr_get_qos_clamp_np(__attr, &clamp);
	if (rv != 0) {
		return rv;
	}

	switch (clamp) {
		case POSIX_SPAWN_PROC_CLAMP_UTILITY:
			*__qos_class = QOS_CLASS_UTILITY;
			break;
		case POSIX_SPAWN_PROC_CLAMP_BACKGROUND:
			*__qos_class = QOS_CLASS_BACKGROUND;
			break;
		case POSIX_SPAWN_PROC_CLAMP_MAINTENANCE:
			*__qos_class = QOS_CLASS_MAINTENANCE;
			break;
		default:
			*__qos_class = QOS_CLASS_UNSPECIFIED;
			break;
	}

	return 0;
}
