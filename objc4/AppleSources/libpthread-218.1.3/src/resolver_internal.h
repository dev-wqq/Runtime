/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __PTHREAD_RESOLVER_INTERNAL_H__
#define __PTHREAD_RESOLVER_INTERNAL_H__

#include <os/base.h>
#include <TargetConditionals.h>
#include <machine/cpu_capabilities.h>
#if defined(__arm__)
#include <arm/arch.h>
#endif

// XXX <rdar://problem/24290376> eOS version of libpthread doesn't have UP optimizations
#if !defined(VARIANT_STATIC) && \
     defined(_ARM_ARCH_7) && !defined(__ARM_ARCH_7S__)

#if OS_ATOMIC_UP
#define OS_VARIANT_SELECTOR up
#else
#define OS_VARIANT_SELECTOR mp
#endif

#endif // !VARIANT_STATIC && _ARM_ARCH_7 && !__ARM_ARCH_7S__

#define OS_VARIANT(f, v) OS_CONCAT(f, OS_CONCAT($VARIANT$, v))

#ifdef OS_VARIANT_SELECTOR
#define PTHREAD_NOEXPORT_VARIANT  PTHREAD_NOEXPORT
#else
#define PTHREAD_NOEXPORT_VARIANT
#endif

#endif // __PTHREAD_RESOLVER_H__
