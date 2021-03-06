/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _TEST_UTIL_H_
#define _TEST_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define PASS    (0)
#define FAIL    (-1)

#define SuccessOrQuit(ERR, MSG)						      \
  do {                                                                        \
    if ((ERR))                                                                \
    {                                                                         \
      fprintf(stderr, "\nFAILED %s:%d - %s\n", __FUNCTION__, __LINE__, MSG);  \
      exit(-1);                                                               \
    }                                                                         \
  } while (false)

#define VerifyOrQuit(TST, MSG)                                                \
  do {                                                                        \
    if (!(TST))                                                               \
    {                                                                         \
      fprintf(stderr, "\nFAILED %s:%d - %s\n", __FUNCTION__, __LINE__, MSG);  \
      exit(-1);                                                               \
    }                                                                         \
  } while (false)


#ifdef __cplusplus
}
#endif

#endif  /* _TEST_UTIL_H_ */
