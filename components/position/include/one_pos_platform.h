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
 * @file        one_pos_platform.h
 * 
 * @brief       Platform control
 * 
 * @details     Control over different platforms
 * 
 * @revision
 * Date         Author          Notes
 * 2020-07-17   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __ONE_POS_PLATFORM_H__
#define __ONE_POS_PLATFORM_H__

#if defined _MSC_VER
#ifndef __OPS_WINDOWS__
#define __OPS_WINDOWS__ /* Windows platform */
#endif /* __OPS_WINDOWS__ */
#else
#ifndef __OPS_ONE_OS__
#define __OPS_ONE_OS__  /* OneOS platform */
#endif /* __OPS_ONE_OS__ */
#endif

#endif /* __ONE_POS_PLATFORM_H__ */
