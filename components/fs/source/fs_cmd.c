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
 * @file        poll.c
 *
 * @brief       This file implements file system related shell cmds.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-20   OneOS team      First Version
 ***********************************************************************************************************************
 */
 
#include <oneos_config.h>
#include <os_util.h>
#include <os_errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef OS_USING_SHELL
#include <shell.h>

#ifdef OS_USING_VFS
#include <vfs_posix.h>
#include <vfs_fs.h>
#ifdef VFS_USING_WORKDIR
extern char working_directory[];
#endif

static os_err_t sh_ls(os_int32_t argc, char **argv)
{
    extern void ls(const char *pathname);

    if (argc == 1)
    {
#ifdef VFS_USING_WORKDIR
        ls(working_directory);
#else
        ls("/");
#endif
    }
    else
    {
        ls(argv[1]);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(ls, sh_ls, "List information about the FILEs.");

static os_err_t sh_cp(os_int32_t argc, char **argv)
{
    void copy(const char *src, const char *dst);

    if (argc != 3)
    {
        os_kprintf("Usage: cp SOURCE DEST\n");
        os_kprintf("Copy SOURCE to DEST.\n");
    }
    else
    {
        copy(argv[1], argv[2]);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(cp, sh_cp, "Copy SOURCE to DEST.");

static os_err_t move(const char *src, const char *dst)
{
#define FLAG_SRC_TYPE      0x03
#define FLAG_SRC_IS_DIR    0x01
#define FLAG_SRC_IS_FILE   0x02
#define FLAG_SRC_NON_EXSIT 0x00

#define FLAG_DST_TYPE      0x0C
#define FLAG_DST_IS_DIR    0x04
#define FLAG_DST_IS_FILE   0x08
#define FLAG_DST_NON_EXSIT 0x00

    struct stat stat;
    uint32_t flag = 0;
    int fd;
    char *dest = OS_NULL;

    /* check the staus of src and dst */
    if (vfs_file_stat(src, &stat) < 0)
    {
        os_kprintf("move failed, bad source: %s\r\n", src);
        return OS_EINVAL;
    }
    if (S_ISDIR(stat.st_mode))
        flag |= FLAG_SRC_IS_DIR;
    else
        flag |= FLAG_SRC_IS_FILE;

    if (vfs_file_stat(dst, &stat) < 0)
    {
        flag |= FLAG_DST_NON_EXSIT;
    }
    else
    {
        if (S_ISDIR(stat.st_mode))
            flag |= FLAG_DST_IS_DIR;
        else
            flag |= FLAG_DST_IS_FILE;
    }

    if((flag & FLAG_SRC_IS_DIR) && (flag & FLAG_DST_IS_FILE))
    {
        os_kprintf("mv failed, mv a dir to a file is not permitted!\r\n");
        return OS_EINVAL;
    }
    
    os_kprintf("%s => %s\n", src, dst);

    fd = open(dst, O_DIRECTORY, 0);
    if (fd >= 0)
    {
        char *src_tmp;

        close(fd);

        /* it's a directory */
        dest = (char *)os_malloc(VFS_PATH_MAX);
        if (dest == OS_NULL)
        {
            os_kprintf("out of memory\n");
            return OS_ENOMEM;
        }

        src_tmp = (char *)src + strlen(src);
        while (src_tmp != src)
        {
            if (*src_tmp == '/') break;
            src_tmp --;
        }

        os_snprintf(dest, VFS_PATH_MAX - 1, "%s/%s", dst, src_tmp);
    }
    else
    {
        fd = open(dst, O_RDONLY, 0);
        if (fd >= 0)
        {
            close(fd);

            unlink(dst);
        }

        dest = (char *)dst;
    }

    rename(src, dest);
    if (dest != OS_NULL && dest != dst) os_free(dest);
    
    return OS_EOK;
}

static os_err_t sh_mv(os_int32_t argc, char **argv)
{
    if (argc != 3)
    {
        os_kprintf("Usage: mv SOURCE DEST\n");
        os_kprintf("Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.\n");
    }
    else
    {
        move(argv[1], argv[2]);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mv, sh_mv, "Rename SOURCE to DEST.");

static os_err_t sh_cat(os_int32_t argc, char **argv)
{
    int index;
    extern void cat(const char *filename);

    if (argc == 1)
    {
        os_kprintf("Usage: cat [FILE]...\n");
        os_kprintf("Concatenate FILE(s)\n");
        return OS_EOK;
    }

    for (index = 1; index < argc; index ++)
    {
        cat(argv[index]);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(cat, sh_cat, "Concatenate FILE(s)");

static os_err_t sh_rm(os_int32_t argc, char **argv)
{
    int index;

    if (argc == 1)
    {
        os_kprintf("Usage: rm FILE...\n");
        os_kprintf("Remove (unlink) the FILE(s).\n");
        return OS_EOK;
    }

    for (index = 1; index < argc; index ++)
    {
        unlink(argv[index]);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(rm, sh_rm, "Remove(unlink) the FILE(s).");

#ifdef VFS_USING_WORKDIR
static os_err_t sh_cd(os_int32_t argc, char **argv)
{
    if (argc == 1)
    {
        os_kprintf("%s\n", working_directory);
    }
    else if (argc == 2)
    {
        if (chdir(argv[1]) != 0)
        {
            os_kprintf("No such directory: %s\n", argv[1]);
        }
    }

    return OS_EOK;
}
SH_CMD_EXPORT(cd, sh_cd, "Change the shell working directory.");

static os_err_t sh_pwd(os_int32_t argc, char **argv)
{
    os_kprintf("%s\n", working_directory);
    return OS_EOK;
}
SH_CMD_EXPORT(pwd, sh_pwd, "Print the name of the current working directory.");
#endif

static os_err_t sh_mkdir(os_int32_t argc, char **argv)
{
    if (argc == 1)
    {
        os_kprintf("Usage: mkdir [OPTION] DIRECTORY\n");
        os_kprintf("Create the DIRECTORY, if they do not already exist.\n");
    }
    else
    {
        mkdir(argv[1], 0);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mkdir, sh_mkdir, "Create the DIRECTORY.");

static os_err_t sh_mkfs(os_int32_t argc, char **argv)
{
    int result = 0;
    char *type = "fat"; /* use the default file system type as 'fatfs' */

    if (argc == 2)
    {
        result = vfs_mkfs(type, argv[1]);
    }
    else if (argc == 4)
    {
        if (strcmp(argv[1], "-t") == 0)
        {
            type = argv[2];
            result = vfs_mkfs(type, argv[3]);
        }
    }
    else
    {
        os_kprintf("Usage: mkfs [-t type] device\n");
        return OS_EOK;
    }

    if (result != OS_EOK)
    {
        os_kprintf("mkfs failed, result=%d\n", result);
    }

    return OS_EOK;
}
SH_CMD_EXPORT(mkfs, sh_mkfs, "format disk with file system");

extern int df(const char *path);
static os_err_t sh_df(os_int32_t argc, char **argv)
{
    if (argc != 2)
    {
        df("/");
    }
    else
    {
        if ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0))
        {
            os_kprintf("df [path]\n");
        }
        else
        {
            df(argv[1]);
        }
    }

    return OS_EOK;
}
SH_CMD_EXPORT(df, sh_df, "disk free");

static os_err_t sh_echo(os_int32_t argc, char **argv)
{
    if (argc == 2)
    {
        os_kprintf("%s\n", argv[1]);
    }
    else if (argc == 3)
    {
        int fd;

        fd = open(argv[2], O_RDWR | O_APPEND | O_CREAT, 0);
        if (fd >= 0)
        {
            write (fd, argv[1], strlen(argv[1]));
            close(fd);
        }
        else
        {
            os_kprintf("open file:%s failed!\n", argv[2]);
        }
    }
    else
    {
        os_kprintf("Usage: echo \"string\" [filename]\n");
    }

    return OS_EOK;
}
SH_CMD_EXPORT(echo, sh_echo, "echo string to file");
#endif /*OS_USING_VFS */

#endif
