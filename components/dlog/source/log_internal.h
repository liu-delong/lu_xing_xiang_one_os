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
 * @file        log_internal.h
 *
 * @brief       Header file for dlog internal use.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-24   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __LOG_INTERNAL_H__
#define __LOG_INTERNAL_H__

#define LOG_FACMASK                 0x03f8                      /* Mask to extract facility part */
#define LOG_FAC(p)                  (((p) & LOG_FACMASK) >> 3)  /* Facility of pri */

#define LOG_MASK(pri)               (1 << (pri))                /* Mask for one priority */
#define LOG_UPTO(pri)               ((1 << ((pri) + 1)) - 1)    /* All priorities through pri */

#define LOG_PRIMASK                 0x07
#define LOG_PRI(p)                  ((p) & LOG_PRIMASK)
#define LOG_MAKEPRI(fac, pri)       (((fac) << 3) | (pri))

/* Output filter's tag max length */
#ifndef DLOG_FILTER_TAG_MAX_LEN
#define DLOG_FILTER_TAG_MAX_LEN     23
#endif

#endif /* __LOG_INTERNAL_H__ */

