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
 * @file        backend_fs.c
 *
 * @brief       This file implements backend for filesystem to record dlog functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-30   OneOS team      First Version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>

#ifdef OS_USING_VFS

#ifdef DLOG_BACKEND_USING_FILESYSTEM
#include <vfs_posix.h>
#include <os_util.h>
#include <string.h>
#include <unistd.h>
#include <dlog.h>

static int  dlog_fd;
static int  file_count;
static char file_name[16];
static char file_type[8];

void dlog_console_backend_filesystem_init(struct dlog_backend *backend)
{   
    char *temp;
    int   iLoop;
    char  dlog_file_name[32];
    off_t file_positon;

    /* Create log record file path */
    mkdir(DLOG_FILE_DIR, 0);

    memset(file_name, 0, sizeof(file_name));
    temp = strstr(DLOG_FILE_NAME, ".");

    if(temp == OS_NULL)
    {
        strcpy(file_name, DLOG_FILE_NAME);
    }
    else
    {
        strcpy(file_type, temp);
        strncpy(file_name, DLOG_FILE_NAME, strlen(DLOG_FILE_NAME) - strlen(temp));
    }

    /* Search file to record log and open it*/
    for(iLoop = 0; iLoop < DLOG_FILE_NUM; iLoop++)
    {
        os_snprintf(dlog_file_name, 
                    sizeof(dlog_file_name), 
                    "%s%s%02d%s", 
                    DLOG_FILE_DIR, 
                    file_name, 
                    iLoop, 
                    file_type);
        
        dlog_fd = open(dlog_file_name, O_RDWR|O_CREAT);

        if(dlog_fd < OS_FALSE)
        {
            os_kprintf("Open dlog log file %s failed !\r\n", dlog_file_name);
            break;
        }

        file_positon = lseek(dlog_fd, 0, SEEK_END);

        if(file_positon < DLOG_FILE_SIZE)
        {
            file_count = iLoop;
            break;
        }

        close(dlog_fd);

        if(iLoop == (DLOG_FILE_NUM-1))
        {
            os_snprintf(dlog_file_name, 
                        sizeof(dlog_file_name), 
                        "%s%s%02d%s", 
                        DLOG_FILE_DIR, 
                        file_name, 
                        0, 
                        file_type);
        
            dlog_fd = open(dlog_file_name, O_RDWR|O_CREAT);
        }
    }

    if(dlog_fd > OS_FALSE)
    {
        os_kprintf("Open record log file is %s%02d%s\n",file_name,file_count,file_type);
    }
}

void dlog_console_backend_filesystem_deinit(struct dlog_backend *backend)
{   
    /* close current record file */
    close(dlog_fd);

    os_kprintf("close record log file is %s%02d%s\n",file_name,file_count,file_type);
}

void dlog_console_backend_filesystem_output(dlog_backend_t *backend,
                                                            os_uint16_t     level,
                                                            const char     *tag,
                                                            os_bool_t       is_raw,
                                                            const char     *log,
                                                            os_size_t       len)
{
    int   ret;
    off_t file_positon;
    char  dlog_file_name[32];

    file_positon = lseek(dlog_fd, 0, SEEK_CUR);
    ret = 0;

    /* record log into file limited file size*/
    if(file_positon < DLOG_FILE_SIZE && len <= (DLOG_FILE_SIZE - file_positon))
    {
        ret = write(dlog_fd, log, len);
        
        if(ret == len)
        {
            fsync(dlog_fd);
        }
        else
        {
            os_kprintf("filesystem no space!\r\n");
            return;
        }
    }
    else if(file_positon <= DLOG_FILE_SIZE && len > (DLOG_FILE_SIZE - file_positon))
    {
        if(file_positon < DLOG_FILE_SIZE)
        {
            ret = write(dlog_fd,log,(DLOG_FILE_SIZE - file_positon));
        }
        
        close(dlog_fd);

        if(file_count == (DLOG_FILE_NUM-1))
        {
            file_count = 0;
        }
        else
        {
            file_count ++;
        }
        
        os_snprintf(dlog_file_name,
                    sizeof(dlog_file_name),
                    "%s%s%02d%s", 
                    DLOG_FILE_DIR, 
                    file_name, 
                    file_count,
                    file_type);

        os_kprintf("file name is %s\r\n", dlog_file_name);
        
        dlog_fd = open(dlog_file_name, O_RDWR|O_CREAT);

        ret = write(dlog_fd, log + ret, len - ret);

        fsync(dlog_fd);
    }
    else if(file_positon > DLOG_FILE_SIZE)
    {
        ret = write(dlog_fd, log, len);
        
        if(ret == len)
        {
            fsync(dlog_fd);
        }
        else
        {
            os_kprintf("filesystem no space!\r\n");
            return;
        }
    }
   
}

#endif /* DLOG_BACKEND_USING_FILESYSTEM */
#endif /* OS_USING_VFS */
