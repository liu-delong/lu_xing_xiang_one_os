/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        cis_sample_defs.h
 *
 * \@brief       cis sample header
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#ifndef _OBJECT_DEFS_H_
#define _OBJECT_DEFS_H_

#include "cis_api.h"

#define SAMPLE_OBJECT_MAX 2

#define SAMPLE_OID_A (3311)
#define SAMPLE_OID_B (3340)

#define SAMPLE_A_INSTANCE_COUNT  3
#define SAMPLE_A_INSTANCE_BITMAP "111"

#define SAMPLE_B_INSTANCE_COUNT  5
#define SAMPLE_B_INSTANCE_BITMAP "11111"

typedef struct st_sample_object
{
    cis_oid_t        oid;
    cis_instcount_t  instCount;
    const char *     instBitmap;
    const cis_rid_t *attrListPtr;
    uint16_t         attrCount;
    const cis_rid_t *actListPtr;
    uint16_t         actCount;
} st_sample_object;

/* a object*/

typedef struct st_object_a
{
    int32_t intValue;
    double  floatValue;
    bool    boolValue;
    char    strValue[1024];

    uint8_t update;
} st_object_a;

typedef struct st_instance_a
{
    cis_iid_t   instId;
    bool        enabled;
    st_object_a instance;
} st_instance_a;

enum
{
    attributeA_intValue    = 5852,
    attributeA_floatValue  = 5820,
    attributeA_stringValue = 5701,
};

enum
{
    actionA_1 = 100,
};

static const cis_rid_t const_AttrIds_a[] = {
    attributeA_intValue,
    attributeA_floatValue,
    attributeA_stringValue,
};

static const cis_rid_t const_ActIds_a[] = {
    actionA_1,
};

/* b object */

typedef struct st_object_b
{
    int32_t intValue;
    double  floatValue;
    bool    boolValue;
    char    strValue[1024];

    uint8_t update;
} st_object_b;

typedef struct st_instance_b
{
    cis_iid_t   instId;
    bool        enabled;
    st_object_b instance;
} st_instance_b;

enum
{
    attributeB_intValue    = 5501,
    attributeB_floatValue  = 5544,
    attributeB_stringValue = 5750,
};

enum
{
    actionB_1 = 5523,
};

static const cis_rid_t const_AttrIds_b[] = {
    attributeB_intValue,
    attributeB_floatValue,
    attributeB_stringValue,
};

static const cis_rid_t const_ActIds_b[] = {
    actionB_1,
};

typedef enum
{
    SAMPLE_CALLBACK_UNKNOWN = 0,
    SAMPLE_CALLBACK_READ,
    SAMPLE_CALLBACK_WRITE,
    SAMPLE_CALLBACK_EXECUTE,
    SAMPLE_CALLBACK_OBSERVE,
    SAMPLE_CALLBACK_SETPARAMS,
    SAMPLE_CALLBACK_DISCOVER,
} et_callback_type_t1;

struct st_observe_info
{
    struct st_observe_info *next;
    cis_listid_t            mid;
    cis_uri_t               uri;
    cis_observe_attr_t      params;
};

struct st_callback_info
{
    struct st_callback_info *next;
    cis_listid_t             mid;
    et_callback_type_t1      flag;
    cis_uri_t                uri;

    union
    {
        struct
        {
            cis_data_t *    value;
            cis_attrcount_t count;
        } asWrite;

        struct
        {
            uint8_t *buffer;
            uint32_t length;
        } asExec;

        struct
        {
            bool flag;
        } asObserve;

        struct
        {
            cis_observe_attr_t params;
        } asObserveParam;
    } param;
};

#endif    /* _OBJECT_DEFS_H_ */
