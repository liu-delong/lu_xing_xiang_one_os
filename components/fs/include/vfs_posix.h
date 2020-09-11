/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        vfs_posix.h
 *
 * @brief       Header file for POSIX I/O interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-22   OneOS team      First Version
 ***********************************************************************************************************************
 */

#ifndef __VFS_POSIX_H__
#define __VFS_POSIX_H__

#include <vfs_file.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 ***********************************************************************************************************************
 * @struct      DIR
 *
 * @brief       A structure representing a directory stream
 ***********************************************************************************************************************
 */
typedef struct
{
    int   fd;     /* Directory file */
    char  buf[512];
    int   num;
    int   cur;
} DIR;

/* Directory api*/
int      mkdir(const char *path, mode_t mode);
DIR     *opendir(const char *name);
long     telldir(DIR *d);
void     seekdir(DIR *d, off_t offset);
void     rewinddir(DIR *d);
int      closedir(DIR *d);
struct dirent *readdir(DIR *d);

/* File api*/
int      open(const char *file, int flags, ...);
int      close(int d);

#if defined(OS_USING_NEWLIB) && defined(_EXFUN)
_READ_WRITE_RETURN_TYPE _EXFUN(read, (int __fd, void *__buf, size_t __nbyte));
_READ_WRITE_RETURN_TYPE _EXFUN(write, (int __fd, const void *__buf, size_t __nbyte));
#else
int      read(int fd, void *buf, size_t len);
int      write(int fd, const void *buf, size_t len);
#endif

off_t    lseek(int fd, off_t offset, int whence);
int      rename(const char *from, const char *to);
int      unlink(const char *pathname);
int      stat(const char *path, struct stat *buf);
int      fstat(int fd, struct stat *buf);
int      fsync(int fd);
int      fcntl(int fd, int cmd, ...);
int      ioctl(int fd, int cmd, ...);

/* Directory api*/
int      rmdir(const char *path);
int      chdir(const char *path);
char    *getcwd(char *buf, size_t size);

/* File system api */
int      statfs(const char *path, struct statfs *buf);

int      access(const char *path, int amode);
int      pipe(int pipefd[2]);
int      mkfifo(const char *path, mode_t mode);

#ifdef __cplusplus
}
#endif

#endif
