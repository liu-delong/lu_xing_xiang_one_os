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
 * @file        dlfcn.h
 *
 * @brief       Header file for posix dlfcn (dynamic linking) interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __DLFCN_H_
#define __DLFCN_H_

#define RTLD_LAZY       0x00000
#define RTLD_NOW        0x00001

#define RTLD_LOCAL      0x00000
#define RTLD_GLOBAL     0x10000

#define RTLD_DEFAULT    ((void*)1)
#define RTLD_NEXT       ((void*)2)

void       *dlopen (const char *filename, int flag);
const char *dlerror(void);
void       *dlsym(void *handle, const char *symbol);
int         dlclose (void *handle);

#endif
