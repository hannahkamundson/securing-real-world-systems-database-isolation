/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * NaCl kernel / service run-time system call ABI.
 * This file defines nacl target machine dependent types.
 */

#ifndef SERVICE_RUNTIME_INCLUDE_MACHINE__TYPES_H_
#define SERVICE_RUNTIME_INCLUDE_MACHINE__TYPES_H_

#ifdef __native_client__
# include <stdint.h>
# include <machine/_default_types.h>
#else
# include "native_client/src/include/portability.h"
#endif

#define __need_size_t
#include <stddef.h>

#ifndef NULL
#define NULL 0
#endif

/*
 * Some of these use (unsigned) int/long versus int<size>_t.  For
 * reasons why, see the discussion in <bits/types.h> and values in
 * <bits/typesizes.h>
 */
#ifndef __dev_t_defined
#define __dev_t_defined
typedef int64_t       __dev_t;
#ifndef __native_client__
typedef __dev_t dev_t;
#endif
#endif

#ifndef __ino_t_defined
#define __ino_t_defined
typedef unsigned long __ino_t;
#ifndef __native_client__
typedef __ino_t ino_t;
#endif
#endif

#ifndef __mode_t_defined
#define __mode_t_defined
typedef uint32_t      __mode_t;
#ifndef __native_client__
typedef __mode_t mode_t;
#endif
#endif

#ifndef __nlink_t_defined
#define __nlink_t_defined
typedef unsigned int  __nlink_t;
#ifndef __native_client__
typedef __nlink_t nlink_t;
#endif
#endif

#ifndef __uid_t_defined
#define __uid_t_defined
typedef uint32_t      __uid_t;
#ifndef __native_client__
typedef __uid_t uid_t;
#endif
#endif

#ifndef __gid_t_defined
#define __gid_t_defined
typedef uint32_t      __gid_t;
#ifndef __native_client__
typedef __gid_t gid_t;
#endif
#endif

#ifndef __off_t_defined
#define __off_t_defined
typedef long int      _off_t;
#ifndef __native_client__
typedef _off_t off_t;
#endif
#endif

#ifndef __blksize_t_defined
#define __blksize_t_defined
typedef long int      __blksize_t;
typedef __blksize_t blksize_t;
#endif

#ifndef __blkcnt_t_defined
#define __blkcnt_t_defined
typedef long int      __blkcnt_t;
typedef __blkcnt_t blkcnt_t;
#endif

#ifndef __time_t_defined
#define __time_t_defined
typedef int32_t       __time_t;
typedef __time_t time_t;
#endif

/*
 * stddef.h defines size_t, and we cannot export another definition.
 * see __need_size_t above and stddef.h
 * (BUILD/gcc-4.2.2/gcc/ginclude/stddef.h) contents.
 */
#define NACL_NO_STRIP(t) nacl_ ## abi_ ## t

#ifndef size_t_defined
#define size_t_defined
typedef uint32_t NACL_NO_STRIP(size_t);
#endif

#ifndef ssize_t_defined
#define ssize_t_defined
typedef int32_t NACL_NO_STRIP(ssize_t);
#endif

#undef NACL_NO_STRIP

#endif
