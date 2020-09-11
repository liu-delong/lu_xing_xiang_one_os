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
 * @file        dirent.h
 *
 * @brief       Header file for directory operation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-13   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __DIRENT_H__
#define __DIRENT_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_libc.h>

/* Ref: http://www.opengroup.org/onlinepubs/009695399/basedefs/dirent.h.html */

/* File types. */
#define FT_REGULAR      0   /* Regular file. */
#define FT_SOCKET       1   /* Socket file.  */
#define FT_DIRECTORY    2   /* Directory.    */
#define FT_USER         3   /* User defined. */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_DIR_STRUCTURE
typedef struct 
{
    os_int32_t fd;         /* Directory file. */
    char       buf[512];
    os_int32_t num;
    os_int32_t cur;
} DIR;
#endif

#ifndef HAVE_DIRENT_STRUCTURE
struct dirent
{
    os_uint8_t  d_type;     /* The type of the file. */
    os_uint8_t  d_namlen;   /* The length of the not including the terminating null file name. */
    os_uint16_t d_reclen;   /* length of this record. */
    char        d_name[256];/* The null-terminated file name. */
};
#endif

int            closedir(DIR *);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
int            readdir_r(DIR *, struct dirent *, struct dirent **);
void           rewinddir(DIR *);
void           seekdir(DIR *, long int);
long           telldir(DIR *);

#ifdef __cplusplus
}
#endif

#endif
