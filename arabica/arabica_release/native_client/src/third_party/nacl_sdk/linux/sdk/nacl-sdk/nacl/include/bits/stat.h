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
 * NaCl Service Runtime API.
 */

#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_STAT_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_STAT_H_

#define S_IFMT       0170000
#define S_IFSOCK     0140000  /* unix-domain socket*/
#define S_IFLNK      0120000  /* symbolic link */
#define S_IFSOCKADDR 0110000  /* socket address */
#define S_IFREG      0100000  /* regular file */
#define S_IFBLK      0060000  /* block device */
#define S_IFDIR      0040000  /* directory */
#define S_IFCHR      0020000  /* character device */
#define S_IFIFO      0010000  /* fifo */

#define S_UNSUP      0170000  /* unsupported file type */
/*
 * NaCl does not support file system objects other than regular files
 * and directories, and objects of other types will appear in the
 * directory namespace but will be mapped to S_UNSUP when
 * these objects are stat(2)ed.  Opening these kinds of objects will
 * fail.
 *
 * The ABI includes these bits so (library) code that use these
 * preprocessor symbols will compile.  The semantics of having a new
 * "unsupported" file type should enable code to run in a reasonably
 * sane way, but YMMV.
 */

#define S_ISUID      0004000
#define S_ISGID      0002000
#define S_ISVTX      0001000

#define S_IREAD      0400
#define S_IWRITE     0200
#define S_IEXEC      0100

#define S_IRWXU  (S_IREAD|S_IWRITE|S_IEXEC)
#define S_IRUSR  (S_IREAD)
#define S_IWUSR  (S_IWRITE)
#define S_IXUSR  (S_IEXEC)

#define S_IRWXG  (S_IRWXU >> 3)
#define S_IRGRP  (S_IREAD >> 3)
#define S_IWGRP  (S_IWRITE >> 3)
#define S_IXGRP  (S_IEXEC >> 3)

#define S_IRWXO  (S_IRWXU >> 6)
#define S_IROTH  (S_IREAD >> 6)
#define S_IWOTH  (S_IWRITE >> 6)
#define S_IXOTH  (S_IEXEC >> 6)
/*
 * only user access bits are supported; the rest are cleared when set
 * (effectively, umask of 077) and cleared when read.
 */

#define S_ISSOCK(m)  (0)
#define S_ISLNK(m)   (0)
#define S_ISREG(m)   (((m) & S_IFMT) == S_IFREG)
#define S_ISBLK(m)   (0)
#define S_ISDIR(m)   (((m) & S_IFMT) == S_IFDIR)
#define S_ISSOCKADDR(m) \
                              (((m) & S_IFMT) == S_IFSOCKADDR)
#define S_ISCHR(m)   (0)
#define S_ISFIFO(m)  (0)

#endif
