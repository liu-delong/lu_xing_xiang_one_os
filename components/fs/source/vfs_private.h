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
 * @file        vfs_private.h
 *
 * @brief       This file declare vfs file system resources.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef VFS_PRIVATE_H__
#define VFS_PRIVATE_H__

#include <vfs.h>

#define DBG_TAG    "VFS"
#define DBG_LVL     DBG_INFO
#include <os_dbg.h>

#define NO_WORKING_DIR  "system does not support working directory\n"

extern const struct vfs_filesystem_ops *filesystem_operation_table[];
extern struct vfs_filesystem filesystem_table[];
extern const struct vfs_mount_tbl mount_table[];

extern char working_directory[];

#endif
