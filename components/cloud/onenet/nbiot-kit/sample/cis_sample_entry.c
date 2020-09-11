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
 * \@file        cis_sample_entry.c
 *
 * \@brief       cis sample file
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <cis_def.h>
/* #include <cis_api.h> */
#include <cis_if_sys.h>
#include <cis_log.h>
#include <cis_list.h>

#include "cis_sample_defs.h"
#include <string.h>
#include "os_task.h"

void taskThread(void *lpParam);
void updateThread(void *lpParam);
void keyThread(void *lpParam);

static void prv_observeNotify(void *context, cis_uri_t *uri, cis_mid_t mid);

static void prv_readResponse(void *context, cis_uri_t *uri, cis_mid_t mid);
static void prv_discoverResponse(void *context, cis_uri_t *uri, cis_mid_t mid);
static void prv_writeResponse(void              *context, 
                              cis_uri_t         *uri, 
                              const cis_data_t  *value, 
                              cis_attrcount_t    count, 
                              cis_mid_t          mid);
static void prv_execResponse(void *context, cis_uri_t *uri, const uint8_t *value, uint32_t length, cis_mid_t mid);
static void prv_paramsResponse(void *context, cis_uri_t *uri, cis_observe_attr_t parameters, cis_mid_t mid);
/* static cis_data_t* prv_dataDup(const cis_data_t* value, cis_attrcount_t attrcount); */

static cis_coapret_t cis_onRead(void *context, cis_uri_t *uri, cis_mid_t mid);
static cis_coapret_t cis_onWrite(void               *context, 
                                 cis_uri_t          *uri, 
                                 const cis_data_t   *value, 
                                 cis_attrcount_t     attrcount, 
                                 cis_mid_t           mid);
static cis_coapret_t cis_onExec(void *context, cis_uri_t *uri, const uint8_t *value, uint32_t length, cis_mid_t mid);
static cis_coapret_t cis_onObserve(void *context, cis_uri_t *uri, bool flag, cis_mid_t mid);
static cis_coapret_t cis_onParams(void *context, cis_uri_t *uri, cis_observe_attr_t parameters, cis_mid_t mid);
static cis_coapret_t cis_onDiscover(void *context, cis_uri_t *uri, cis_mid_t mid);
static void          cis_onEvent(void *context, cis_evt_t eid, void *param);

static struct st_callback_info *g_callbackList = NULL;
static struct st_observe_info  *g_observeList  = NULL;

void       *g_context      = NULL;
static bool g_shutdown     = false;
static bool g_doUnregister = false;
static bool g_doRegister   = false;

/* static cis_time_t g_lifetimeLast = 0; */
static cis_time_t g_notifyLast = 0;

static st_sample_object g_objectList[SAMPLE_OBJECT_MAX];
static st_instance_a    g_instList_a[SAMPLE_A_INSTANCE_COUNT];
static st_instance_b    g_instList_b[SAMPLE_B_INSTANCE_COUNT];

static cis_callback_t callback;
static cis_time_t     g_lifetime = 720;

static void prv_make_sample_data()
{
    int             i = 0;
    cis_instcount_t instIndex;
    cis_instcount_t instCount;
    for (i = 0; i < SAMPLE_OBJECT_MAX; i++)
    {
        st_sample_object *obj = &g_objectList[i];
        switch (i)
        {
        case 0:
        {
            obj->oid        = SAMPLE_OID_A;
            obj->instBitmap = SAMPLE_A_INSTANCE_BITMAP;
            instCount       = SAMPLE_A_INSTANCE_COUNT;
            for (instIndex = 0; instIndex < instCount; instIndex++)
            {
                if (obj->instBitmap[instIndex] != '1')
                {
                    g_instList_a[instIndex].instId  = instIndex;
                    g_instList_a[instIndex].enabled = false;
                }
                else
                {
                    g_instList_a[instIndex].instId  = instIndex;
                    g_instList_a[instIndex].enabled = true;

                    g_instList_a[instIndex].instance.floatValue = cissys_rand() * 10.0 / 0xFFFFFFFF;
                    g_instList_a[instIndex].instance.intValue   = 666;
                    strcpy(g_instList_a[instIndex].instance.strValue, "hello onenet");
                }
            }
            obj->attrCount   = sizeof(const_AttrIds_a) / sizeof(cis_rid_t);
            obj->attrListPtr = const_AttrIds_a;

            obj->actCount   = sizeof(const_ActIds_a) / sizeof(cis_rid_t);
            obj->actListPtr = const_ActIds_a;
        }
        break;
        case 1:
        {
            obj->oid        = SAMPLE_OID_B;
            obj->instBitmap = SAMPLE_B_INSTANCE_BITMAP;
            instCount       = SAMPLE_B_INSTANCE_COUNT;
            for (instIndex = 0; instIndex < instCount; instIndex++)
            {
                if (obj->instBitmap[instIndex] != '1')
                {
                    g_instList_b[instIndex].instId  = instIndex;
                    g_instList_b[instIndex].enabled = false;
                }
                else
                {
                    g_instList_b[instIndex].instId  = instIndex;
                    g_instList_b[instIndex].enabled = true;

                    g_instList_b[instIndex].instance.boolValue  = true;
                    g_instList_b[instIndex].instance.floatValue = cissys_rand() * 10.0 / 0xFFFFFFFF;
                    g_instList_b[instIndex].instance.intValue   = 555;
                    strcpy(g_instList_b[instIndex].instance.strValue, "test");
                }
            }

            obj->attrCount   = sizeof(const_AttrIds_b) / sizeof(cis_rid_t);
            obj->attrListPtr = const_AttrIds_b;

            obj->actCount   = sizeof(const_ActIds_b) / sizeof(cis_rid_t);
            obj->actListPtr = const_ActIds_b;
        }
        break;
        }
    }
}

void taskThread(void *lpParam)
{
    while (!g_shutdown)
    {
        uint32_t pumpRet;
        {
            if (g_doRegister)
            {
                g_doRegister = false;
                cis_register(g_context, g_lifetime, &callback);
            }

            if (g_doUnregister)
            {
                g_doUnregister = false;
                cis_unregister(g_context);
                struct st_observe_info *delnode;
                while (g_observeList != NULL)
                {
                    g_observeList = (struct st_observe_info *)CIS_LIST_RM((cis_list_t *)g_observeList,
                                                                          g_observeList->mid,
                                                                          (cis_list_t **)&delnode);
                    cis_free(delnode);
                }
                cissys_sleepms(1000);
                g_doRegister = 1;
            }
        }

        /*pump function*/
        pumpRet = cis_pump(g_context);
        if (pumpRet == PUMP_RET_CUSTOM)
        {
            /* LOGI("pump sleep(1000)"); */
            /* cissys_sleepms(1000); */
        }
        uint32_t nowtime;
        /*data observe data report*/
        nowtime                              = cissys_gettime();
        struct st_observe_info *observe_node = g_observeList;
        if (nowtime - g_notifyLast > 60 * 1000)
        {
            g_notifyLast = nowtime;
            while (observe_node != NULL)
            {
                if (observe_node->mid == 0)
                {
                    continue;
                }
                if (observe_node->uri.flag == 0)
                {
                    continue;
                }
                cis_uri_t uriLocal;
                uriLocal = observe_node->uri;
                prv_observeNotify(g_context, &uriLocal, observe_node->mid);
                observe_node = observe_node->next;
            }
        }
        struct st_callback_info *node;
        if (g_callbackList == NULL)
        {
            cissys_sleepms(100);
            continue;
        }
        node           = g_callbackList;
        g_callbackList = g_callbackList->next;

        switch (node->flag)
        {
        case 0:
            break;
        case SAMPLE_CALLBACK_READ:
        {
            cis_uri_t uriLocal;
            uriLocal = node->uri;
            prv_readResponse(g_context, &uriLocal, node->mid);
        }
        break;
        case SAMPLE_CALLBACK_DISCOVER:
        {
            cis_uri_t uriLocal;
            uriLocal = node->uri;
            prv_discoverResponse(g_context, &uriLocal, node->mid);
        }
        break;
        case SAMPLE_CALLBACK_WRITE:
        {
            /* write */
            prv_writeResponse(g_context, &node->uri, node->param.asWrite.value, node->param.asWrite.count, node->mid);
            cis_data_t *    data  = node->param.asWrite.value;
            cis_attrcount_t count = node->param.asWrite.count;

            for (int i = 0; i < count; i++)
            {
                if (data[i].type == cis_data_type_string || data[i].type == cis_data_type_opaque)
                {
                    if (data[i].asBuffer.buffer != NULL)
                    {
                        cis_free(data[i].asBuffer.buffer);
                    }
                }
            }
            cis_free(data);
        }
        break;
        case SAMPLE_CALLBACK_EXECUTE:
        {
            /* exec and notify */
            prv_execResponse(g_context, &node->uri, node->param.asExec.buffer, node->param.asExec.length, node->mid);
            cis_free(node->param.asExec.buffer);
        }
        break;
        case SAMPLE_CALLBACK_SETPARAMS:
        {
            /* set parameters and notify */
            prv_paramsResponse(g_context, &node->uri, node->param.asObserveParam.params, node->mid);
        }
        break;
        case SAMPLE_CALLBACK_OBSERVE:
        {
            if (node->param.asObserve.flag)
            {
                uint16_t                count = 0;
                struct st_observe_info *observe_new =
                    (struct st_observe_info *)cis_malloc(sizeof(struct st_observe_info));
                observe_new->mid  = node->mid;
                observe_new->uri  = node->uri;
                observe_new->next = NULL;

                g_observeList =
                    (struct st_observe_info *)cis_list_add((cis_list_t *)g_observeList, (cis_list_t *)observe_new);

                LOGD("cis_on_observe set(%d): %d/%d/%d",
                     count,
                     observe_new->uri.objectId,
                     CIS_URI_IS_SET_INSTANCE(&observe_new->uri) ? observe_new->uri.instanceId : -1,
                     CIS_URI_IS_SET_RESOURCE(&observe_new->uri) ? observe_new->uri.resourceId : -1);

                cis_response(g_context, NULL, NULL, node->mid, CIS_RESPONSE_OBSERVE);
            }
            else
            {
                struct st_observe_info *delnode = g_observeList;

                while (delnode)
                {
                    if (node->uri.flag == delnode->uri.flag && node->uri.objectId == delnode->uri.objectId)
                    {
                        if (node->uri.instanceId == delnode->uri.instanceId)
                        {
                            if (node->uri.resourceId == delnode->uri.resourceId)
                            {
                                break;
                            }
                        }
                    }
                    delnode = delnode->next;
                }
                if (delnode != NULL)
                {
                    g_observeList = (struct st_observe_info *)cis_list_remove((cis_list_t *)g_observeList,
                                                                              delnode->mid,
                                                                              (cis_list_t **)&delnode);

                    LOGD("cis_on_observe cancel: %d/%d/%d\n",
                         delnode->uri.objectId,
                         CIS_URI_IS_SET_INSTANCE(&delnode->uri) ? delnode->uri.instanceId : -1,
                         CIS_URI_IS_SET_RESOURCE(&delnode->uri) ? delnode->uri.resourceId : -1);

                    cis_free(delnode);
                    cis_response(g_context, NULL, NULL, node->mid, CIS_RESPONSE_OBSERVE);
                }
                else
                {
                    return;    /* CIS_RESPONSE_NOT_FOUND */
                }
            }
        }
        default:
            break;
        }

        cis_free(node);
    }

    cis_deinit(&g_context);

    struct st_observe_info *delnode;
    while (g_observeList != NULL)
    {
        g_observeList = (struct st_observe_info *)CIS_LIST_RM((cis_list_t *)g_observeList,
                                                              g_observeList->mid,
                                                              (cis_list_t **)&delnode);
        cis_free(delnode);
    }

    cissys_sleepms(2000);
}

int cis_sample_entry(cis_user_cfg_t *cis_user_cfg)
{
    cis_version_t ver;
    int           index       = 0;
    callback.onRead           = cis_onRead;
    callback.onWrite          = cis_onWrite;
    callback.onExec           = cis_onExec;
    callback.onObserve        = cis_onObserve;
    callback.onSetParams      = cis_onParams;
    callback.onEvent          = cis_onEvent;
    callback.onDiscover       = cis_onDiscover;
    os_task_t *cis_taskhandle = NULL;

    /*init sample data*/
    prv_make_sample_data();

    /* read cis_net and cis_sys cfg */
    if (cis_init(&g_context, cis_user_cfg, NULL) != CIS_RET_OK)
    {
        if (g_context != NULL)
            cis_deinit(&g_context);
        LOGD("cis entry init failed.\n");
        return -1;
    }

    cis_version(&ver);
    LOGI("CIS SDK Version:%u.%u", ver.major, ver.minor);

    for (index = 0; index < SAMPLE_OBJECT_MAX; index++)
    {
        cis_inst_bitmap_t bitmap;
        cis_res_count_t   rescount;
        cis_instcount_t   instCount, instBytes;
        const char       *instAsciiPtr;
        uint8_t          *instPtr;
        cis_oid_t         oid;
        int16_t           i;
        st_sample_object *obj = &g_objectList[index];

        oid       = obj->oid;
        instCount = utils_strlen(obj->instBitmap);
        instBytes = (instCount - 1) / 8 + 1;
        instAsciiPtr = obj->instBitmap;
        instPtr      = (uint8_t *)cis_malloc(instBytes);
        cissys_assert(instPtr != NULL);
        cis_memset(instPtr, 0, instBytes);

        for (i = 0; i < instCount; i++)
        {
            cis_instcount_t instBytePos    = i / 8;
            cis_instcount_t instByteOffset = 7 - (i % 8);
            if (instAsciiPtr[i] == '1')
            {
                instPtr[instBytePos] += 0x01 << instByteOffset;
            }
        }

        bitmap.instanceCount  = instCount;
        bitmap.instanceBitmap = instPtr;
        bitmap.instanceBytes  = instBytes;

        rescount.attrCount = obj->attrCount;
        rescount.actCount  = obj->actCount;

        cis_addobject(g_context, oid, &bitmap, &rescount);
        cis_free(instPtr);
    }

    g_shutdown     = false;
    g_doUnregister = false;

    /* register enabled */
    g_doRegister = true;

    cis_taskhandle = os_task_create("cis_process", taskThread, NULL, 2048, OS_TASK_PRIORITY_MAX / 2, 10);
    if (NULL == cis_taskhandle)
    {
        return -1;
    }
    os_task_startup(cis_taskhandle);

    return 0;
}

/* private funcation; */
static void prv_observeNotify(void *context, cis_uri_t *uri, cis_mid_t mid)
{
    uint8_t           index;
    st_sample_object *object = NULL;
    cis_data_t        value;
    for (index = 0; index < SAMPLE_OBJECT_MAX; index++)
    {
        if (g_objectList[index].oid == uri->objectId)
        {
            object = &g_objectList[index];
        }
    }

    if (object == NULL)
    {
        return;
    }

    if (!CIS_URI_IS_SET_INSTANCE(uri) && !CIS_URI_IS_SET_RESOURCE(uri))    /* one object */
    {
        switch (uri->objectId)
        {
        case SAMPLE_OID_A:
        {
            for (index = 0; index < SAMPLE_A_INSTANCE_COUNT; index++)
            {
                st_instance_a *inst = &g_instList_a[index];
                if (inst != NULL && inst->enabled == true)
                {
                    cis_data_t tmpdata[3];

                    tmpdata[0].type            = cis_data_type_integer;
                    tmpdata[0].value.asInteger = inst->instance.intValue;
                    uri->instanceId            = index;
                    uri->resourceId            = attributeA_intValue;
                    cis_uri_update(uri);
                    cis_notify(context, uri, &tmpdata[0], mid, CIS_NOTIFY_CONTINUE, false);

                    tmpdata[1].type          = cis_data_type_float;
                    tmpdata[1].value.asFloat = inst->instance.floatValue;
                    uri->instanceId          = index;
                    uri->resourceId          = attributeA_floatValue;
                    cis_uri_update(uri);
                    cis_notify(context, uri, &tmpdata[1], mid, CIS_NOTIFY_CONTINUE, false);

                    tmpdata[2].type            = cis_data_type_string;
                    tmpdata[2].asBuffer.length = strlen(inst->instance.strValue);
                    tmpdata[2].asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                    uri->instanceId            = index;
                    uri->resourceId            = attributeA_stringValue;
                    cis_uri_update(uri);
                    cis_notify(context, uri, &tmpdata[2], mid, CIS_NOTIFY_CONTENT, false);
                }
            }
        }
        break;
        case SAMPLE_OID_B:
        {
            for (index = 0; index < SAMPLE_B_INSTANCE_COUNT; index++)
            {
                st_instance_b *inst = &g_instList_b[index];
                if (inst != NULL && inst->enabled == true)
                {
                    cis_data_t tmpdata[4];

                    tmpdata[0].type            = cis_data_type_integer;
                    tmpdata[0].value.asInteger = inst->instance.intValue;
                    uri->instanceId            = index;
                    uri->resourceId            = attributeB_intValue;
                    cis_uri_update(uri);
                    cis_notify(context, uri, &tmpdata[0], mid, CIS_NOTIFY_CONTINUE, false);

                    tmpdata[1].type          = cis_data_type_float;
                    tmpdata[1].value.asFloat = inst->instance.floatValue;
                    uri->instanceId          = index;
                    uri->resourceId          = attributeB_floatValue;
                    cis_uri_update(uri);
                    cis_notify(context, uri, &tmpdata[1], mid, CIS_NOTIFY_CONTINUE, false);

                    tmpdata[2].type            = cis_data_type_string;
                    tmpdata[2].asBuffer.length = strlen(inst->instance.strValue);
                    tmpdata[2].asBuffer.buffer = (uint8_t *)(inst->instance.strValue);

                    uri->instanceId = index;
                    uri->resourceId = attributeB_stringValue;
                    cis_uri_update(uri);
                    cis_notify(context, uri, &tmpdata[2], mid, CIS_NOTIFY_CONTENT, false);
                }
            }
        }
        break;
        }
    }
    else if (CIS_URI_IS_SET_INSTANCE(uri))
    {
        switch (object->oid)
        {
        case SAMPLE_OID_A:
        {
            if (uri->instanceId > SAMPLE_A_INSTANCE_COUNT)
            {
                return;
            }
            st_instance_a *inst = &g_instList_a[uri->instanceId];
            if (inst == NULL || inst->enabled == false)
            {
                return;
            }

            if (CIS_URI_IS_SET_RESOURCE(uri))
            {
                if (uri->resourceId == attributeA_intValue)
                {
                    value.type            = cis_data_type_integer;
                    value.value.asInteger = inst->instance.intValue;
                }
                else if (uri->resourceId == attributeA_floatValue)
                {
                    value.type          = cis_data_type_float;
                    value.value.asFloat = inst->instance.floatValue;
                }
                else if (uri->resourceId == attributeA_stringValue)
                {
                    value.type            = cis_data_type_string;
                    value.asBuffer.length = strlen(inst->instance.strValue);
                    value.asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                }
                else
                {
                    return;
                }

                cis_notify(context, uri, &value, mid, CIS_NOTIFY_CONTENT, false);
            }
            else
            {
                cis_data_t tmpdata[3];

                tmpdata[0].type            = cis_data_type_integer;
                tmpdata[0].value.asInteger = inst->instance.intValue;
                uri->resourceId            = attributeA_intValue;
                cis_uri_update(uri);
                cis_notify(context, uri, &tmpdata[0], mid, CIS_NOTIFY_CONTINUE, false);

                tmpdata[1].type          = cis_data_type_float;
                tmpdata[1].value.asFloat = inst->instance.floatValue;
                uri->resourceId          = attributeA_floatValue;
                cis_uri_update(uri);
                cis_notify(context, uri, &tmpdata[1], mid, CIS_NOTIFY_CONTINUE, false);

                tmpdata[2].type            = cis_data_type_string;
                tmpdata[2].asBuffer.length = strlen(inst->instance.strValue);
                tmpdata[2].asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                uri->resourceId            = attributeA_stringValue;
                cis_uri_update(uri);
                cis_notify(context, uri, &tmpdata[2], mid, CIS_NOTIFY_CONTENT, false);
            }
        }
        break;
        case SAMPLE_OID_B:
        {
            if (uri->instanceId > SAMPLE_B_INSTANCE_COUNT)
            {
                return;
            }
            st_instance_b *inst = &g_instList_b[uri->instanceId];
            if (inst == NULL || inst->enabled == false)
            {
                return;
            }

            if (CIS_URI_IS_SET_RESOURCE(uri))
            {
                if (uri->resourceId == attributeB_intValue)
                {
                    value.type            = cis_data_type_integer;
                    value.value.asInteger = inst->instance.intValue;
                }
                else if (uri->resourceId == attributeB_floatValue)
                {
                    value.type          = cis_data_type_float;
                    value.value.asFloat = inst->instance.floatValue;
                }
                else if (uri->resourceId == attributeB_stringValue)
                {
                    value.type            = cis_data_type_string;
                    value.asBuffer.length = strlen(inst->instance.strValue);
                    value.asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                }
                else
                {
                    return;
                }

                cis_notify(context, uri, &value, mid, CIS_NOTIFY_CONTENT, false);
            }
            else
            {
                cis_data_t tmpdata[3];

                tmpdata[0].type            = cis_data_type_integer;
                tmpdata[0].value.asInteger = inst->instance.intValue;
                uri->resourceId            = attributeB_intValue;
                cis_uri_update(uri);
                cis_notify(context, uri, &tmpdata[0], mid, CIS_NOTIFY_CONTINUE, false);

                tmpdata[1].type          = cis_data_type_float;
                tmpdata[1].value.asFloat = inst->instance.floatValue;
                uri->resourceId          = attributeB_floatValue;
                cis_uri_update(uri);
                cis_notify(context, uri, &tmpdata[1], mid, CIS_NOTIFY_CONTINUE, false);

                tmpdata[2].type            = cis_data_type_string;
                tmpdata[2].asBuffer.length = strlen(inst->instance.strValue);
                tmpdata[2].asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                uri->resourceId            = attributeB_stringValue;
                cis_uri_update(uri);
                cis_notify(context, uri, &tmpdata[2], mid, CIS_NOTIFY_CONTENT, false);
            }
        }
        break;
        }
    }
}

static void prv_readResponse(void *context, cis_uri_t *uri, cis_mid_t mid)
{
    uint8_t           index;
    st_sample_object *object = NULL;
    cis_data_t        value;
    for (index = 0; index < SAMPLE_OBJECT_MAX; index++)
    {
        if (g_objectList[index].oid == uri->objectId)
        {
            object = &g_objectList[index];
        }
    }

    if (object == NULL)
    {
        return;
    }

    if (!CIS_URI_IS_SET_INSTANCE(uri) && !CIS_URI_IS_SET_RESOURCE(uri))    /* one object */
    {
        switch (uri->objectId)
        {
        case SAMPLE_OID_A:
        {
            for (index = 0; index < SAMPLE_A_INSTANCE_COUNT; index++)
            {
                st_instance_a *inst = &g_instList_a[index];
                if (inst != NULL && inst->enabled == true)
                {
                    cis_data_t tmpdata[3];

                    tmpdata[0].type            = cis_data_type_integer;
                    tmpdata[0].value.asInteger = inst->instance.intValue;
                    uri->instanceId            = inst->instId;
                    uri->resourceId            = attributeA_intValue;
                    cis_uri_update(uri);
                    cis_response(context, uri, &tmpdata[0], mid, CIS_RESPONSE_CONTINUE);

                    tmpdata[1].type          = cis_data_type_float;
                    tmpdata[1].value.asFloat = inst->instance.floatValue;
                    uri->resourceId          = attributeA_floatValue;
                    uri->instanceId          = inst->instId;
                    cis_uri_update(uri);
                    cis_response(context, uri, &tmpdata[1], mid, CIS_RESPONSE_CONTINUE);

                    tmpdata[2].type            = cis_data_type_string;
                    tmpdata[2].asBuffer.length = strlen(inst->instance.strValue);
                    tmpdata[2].asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                    uri->resourceId            = attributeA_stringValue;
                    uri->instanceId            = inst->instId;
                    cis_uri_update(uri);
                    cis_response(context, uri, &tmpdata[2], mid, CIS_RESPONSE_CONTINUE);
                }
            }
        }
        break;
        case SAMPLE_OID_B:
        {
            for (index = 0; index < SAMPLE_B_INSTANCE_COUNT; index++)
            {
                st_instance_b *inst = &g_instList_b[index];
                if (inst != NULL && inst->enabled == true)
                {
                    cis_data_t tmpdata[3];

                    tmpdata[0].type            = cis_data_type_integer;
                    tmpdata[0].value.asInteger = inst->instance.intValue;
                    uri->instanceId            = inst->instId;
                    uri->resourceId            = attributeB_intValue;
                    cis_uri_update(uri);
                    cis_response(context, uri, &tmpdata[0], mid, CIS_RESPONSE_CONTINUE);

                    tmpdata[1].type          = cis_data_type_float;
                    tmpdata[1].value.asFloat = inst->instance.floatValue;
                    uri->resourceId          = attributeB_floatValue;
                    uri->instanceId          = inst->instId;
                    cis_uri_update(uri);
                    cis_response(context, uri, &tmpdata[1], mid, CIS_RESPONSE_CONTINUE);

                    tmpdata[2].type            = cis_data_type_string;
                    tmpdata[2].asBuffer.length = strlen(inst->instance.strValue);
                    tmpdata[2].asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                    uri->resourceId            = attributeB_stringValue;
                    uri->instanceId            = inst->instId;
                    cis_uri_update(uri);
                    cis_response(context, uri, &tmpdata[2], mid, CIS_RESPONSE_CONTINUE);
                }
            }
        }
        break;
        }
        cis_response(context, NULL, NULL, mid, CIS_RESPONSE_READ);
    }
    else
    {
        switch (object->oid)
        {
        case SAMPLE_OID_A:
        {
            if (uri->instanceId > SAMPLE_A_INSTANCE_COUNT)
            {
                return;
            }
            st_instance_a *inst = &g_instList_a[uri->instanceId];
            if (inst == NULL || inst->enabled == false)
            {
                return;
            }

            if (CIS_URI_IS_SET_RESOURCE(uri))
            {
                if (uri->resourceId == attributeA_intValue)
                {
                    value.type            = cis_data_type_integer;
                    value.value.asInteger = inst->instance.intValue;
                }
                else if (uri->resourceId == attributeA_floatValue)
                {
                    value.type          = cis_data_type_float;
                    value.value.asFloat = inst->instance.floatValue;
                }
                else if (uri->resourceId == attributeA_stringValue)
                {
                    value.type            = cis_data_type_string;
                    value.asBuffer.length = strlen(inst->instance.strValue);
                    value.asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                }
                else
                {
                    return;
                }

                cis_response(context, uri, &value, mid, CIS_RESPONSE_READ);
            }
            else
            {
                cis_data_t tmpdata[3];

                tmpdata[0].type            = cis_data_type_integer;
                tmpdata[0].value.asInteger = inst->instance.intValue;
                uri->resourceId            = attributeA_intValue;
                cis_uri_update(uri);
                cis_response(context, uri, &tmpdata[0], mid, CIS_RESPONSE_CONTINUE);

                tmpdata[1].type          = cis_data_type_float;
                tmpdata[1].value.asFloat = inst->instance.floatValue;
                uri->resourceId          = attributeA_floatValue;
                cis_uri_update(uri);
                cis_response(context, uri, &tmpdata[1], mid, CIS_RESPONSE_CONTINUE);

                tmpdata[2].type            = cis_data_type_string;
                tmpdata[2].asBuffer.length = strlen(inst->instance.strValue);
                tmpdata[2].asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                uri->resourceId            = attributeA_stringValue;
                cis_uri_update(uri);
                cis_response(context, uri, &tmpdata[2], mid, CIS_RESPONSE_READ);
            }
        }
        break;
        case SAMPLE_OID_B:
        {
            if (uri->instanceId > SAMPLE_B_INSTANCE_COUNT)
            {
                return;
            }
            st_instance_b *inst = &g_instList_b[uri->instanceId];
            if (inst == NULL || inst->enabled == false)
            {
                return;
            }

            if (CIS_URI_IS_SET_RESOURCE(uri))
            {
                if (uri->resourceId == attributeB_intValue)
                {
                    value.type            = cis_data_type_integer;
                    value.value.asInteger = inst->instance.intValue;
                }
                else if (uri->resourceId == attributeB_floatValue)
                {
                    value.type          = cis_data_type_float;
                    value.value.asFloat = inst->instance.floatValue;
                }
                else if (uri->resourceId == attributeB_stringValue)
                {
                    value.type            = cis_data_type_string;
                    value.asBuffer.length = strlen(inst->instance.strValue);
                    value.asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                }
                else
                {
                    return;
                }

                cis_response(context, uri, &value, mid, CIS_RESPONSE_READ);
            }
            else
            {
                cis_data_t tmpdata[3];

                tmpdata[0].type            = cis_data_type_integer;
                tmpdata[0].value.asInteger = inst->instance.intValue;
                uri->resourceId            = attributeB_intValue;
                cis_uri_update(uri);
                cis_response(context, uri, &tmpdata[0], mid, CIS_RESPONSE_CONTINUE);

                tmpdata[1].type          = cis_data_type_float;
                tmpdata[1].value.asFloat = inst->instance.floatValue;
                uri->resourceId          = attributeB_floatValue;
                cis_uri_update(uri);
                cis_response(context, uri, &tmpdata[1], mid, CIS_RESPONSE_CONTINUE);

                tmpdata[2].type            = cis_data_type_string;
                tmpdata[2].asBuffer.length = strlen(inst->instance.strValue);
                tmpdata[2].asBuffer.buffer = (uint8_t *)(inst->instance.strValue);
                uri->resourceId            = attributeB_stringValue;
                cis_uri_update(uri);
                cis_response(context, uri, &tmpdata[2], mid, CIS_RESPONSE_READ);
            }
        }
        break;
        }
    }
}

static void prv_discoverResponse(void *context, cis_uri_t *uri, cis_mid_t mid)
{
    uint8_t           index;
    st_sample_object *object = NULL;

    for (index = 0; index < SAMPLE_OBJECT_MAX; index++)
    {
        if (g_objectList[index].oid == uri->objectId)
        {
            object = &g_objectList[index];
        }
    }

    if (object == NULL)
    {
        return;
    }

    if (CIS_URI_IS_SET_INSTANCE(uri) || CIS_URI_IS_SET_RESOURCE(uri))
    {
        return;
    }

    switch (uri->objectId)
    {
    case SAMPLE_OID_A:
    {
        uri->objectId   = URI_INVALID;
        uri->instanceId = URI_INVALID;
        uri->resourceId = attributeA_intValue;
        cis_uri_update(uri);
        cis_response(context, uri, NULL, mid, CIS_RESPONSE_CONTINUE);

        uri->objectId   = URI_INVALID;
        uri->instanceId = URI_INVALID;
        uri->resourceId = attributeA_floatValue;
        cis_uri_update(uri);
        cis_response(context, uri, NULL, mid, CIS_RESPONSE_CONTINUE);

        uri->objectId   = URI_INVALID;
        uri->instanceId = URI_INVALID;
        uri->resourceId = attributeA_stringValue;
        cis_uri_update(uri);
        cis_response(context, uri, NULL, mid, CIS_RESPONSE_CONTINUE);

        uri->objectId   = URI_INVALID;
        uri->instanceId = URI_INVALID;
        uri->resourceId = actionA_1;
        cis_uri_update(uri);
        cis_response(context, uri, NULL, mid, CIS_RESPONSE_CONTINUE);
    }
    break;
    case SAMPLE_OID_B:
    {
        uri->objectId   = URI_INVALID;
        uri->instanceId = URI_INVALID;
        uri->resourceId = attributeB_intValue;
        cis_uri_update(uri);
        cis_response(context, uri, NULL, mid, CIS_RESPONSE_CONTINUE);

        uri->objectId   = URI_INVALID;
        uri->instanceId = URI_INVALID;
        uri->resourceId = attributeB_floatValue;
        cis_uri_update(uri);
        cis_response(context, uri, NULL, mid, CIS_RESPONSE_CONTINUE);

        uri->objectId   = URI_INVALID;
        uri->instanceId = URI_INVALID;
        uri->resourceId = attributeB_stringValue;
        cis_uri_update(uri);
        cis_response(context, uri, NULL, mid, CIS_RESPONSE_CONTINUE);

        uri->objectId   = URI_INVALID;
        uri->instanceId = URI_INVALID;
        uri->resourceId = actionB_1;
        cis_uri_update(uri);
        cis_response(context, uri, NULL, mid, CIS_RESPONSE_CONTINUE);
    }
    break;
    }
    cis_response(context, NULL, NULL, mid, CIS_RESPONSE_DISCOVER);
}

static void prv_writeResponse(void              *context, 
                              cis_uri_t         *uri, 
                              const cis_data_t  *value, 
                              cis_attrcount_t    count, 
                              cis_mid_t          mid)
{

    uint8_t           index;
    st_sample_object *object = NULL;

    if (!CIS_URI_IS_SET_INSTANCE(uri))
    {
        return;
    }

    for (index = 0; index < SAMPLE_OBJECT_MAX; index++)
    {
        if (g_objectList[index].oid == uri->objectId)
        {
            object = &g_objectList[index];
        }
    }

    if (object == NULL)
    {
        return;
    }

    switch (object->oid)
    {
    case SAMPLE_OID_A:
    {
        if (uri->instanceId > SAMPLE_B_INSTANCE_COUNT)
        {
            return;
        }
        st_instance_a *inst = &g_instList_a[uri->instanceId];
        if (inst == NULL || inst->enabled == false)
        {
            return;
        }

        for (int i = 0; i < count; i++)
        {
            LOGI("write %d/%d/%d", uri->objectId, uri->instanceId, value[i].id);
            switch (value[i].id)
            {
            case attributeA_intValue:
            {
                inst->instance.intValue = value[i].value.asInteger;
            }
            break;
            case attributeA_floatValue:
            {
                inst->instance.floatValue = value[i].value.asFloat;
            }
            break;
            case attributeA_stringValue:
            {
                memset(inst->instance.strValue, 0, sizeof(inst->instance.strValue));
                strncpy(inst->instance.strValue, (char *)value[i].asBuffer.buffer, value[i].asBuffer.length);
            }
            break;
            }
        }
    }
    break;
    case SAMPLE_OID_B:
    {
        if (uri->instanceId > SAMPLE_B_INSTANCE_COUNT)
        {
            return;
        }
        st_instance_b *inst = &g_instList_b[uri->instanceId];
        if (inst == NULL || inst->enabled == false)
        {
            return;
        }

        for (int i = 0; i < count; i++)
        {
            LOGI("write %d/%d/%d", uri->objectId, uri->instanceId, value[i].id);
            switch (value[i].id)
            {
            case attributeB_intValue:
            {
                inst->instance.intValue = value[i].value.asInteger;
            }
            break;
            case attributeB_floatValue:
            {
                inst->instance.floatValue = value[i].value.asFloat;
            }
            break;
            case attributeB_stringValue:
            {
                memset(inst->instance.strValue, 0, sizeof(inst->instance.strValue));
                strncpy(inst->instance.strValue, (char *)value[i].asBuffer.buffer, value[i].asBuffer.length);
            }
            break;
            }
        }
    }
    break;
    }

    cis_response(context, NULL, NULL, mid, CIS_RESPONSE_WRITE);
}

static void prv_execResponse(void *context, cis_uri_t *uri, const uint8_t *value, uint32_t length, cis_mid_t mid)
{

    uint8_t           index;
    st_sample_object *object = NULL;

    for (index = 0; index < SAMPLE_OBJECT_MAX; index++)
    {
        if (g_objectList[index].oid == uri->objectId)
        {
            object = &g_objectList[index];
        }
    }

    if (object == NULL)
    {
        return;
    }

    switch (object->oid)
    {
    case SAMPLE_OID_A:
    {
        if (uri->instanceId > SAMPLE_B_INSTANCE_COUNT)
        {
            return;
        }
        st_instance_a *inst = &g_instList_a[uri->instanceId];
        if (inst == NULL || inst->enabled == false)
        {
            return;
        }

        if (uri->resourceId == actionA_1)
        {
            /*
             *\call action;
             */
            LOGI("exec actionA_1");
            cis_response(context, NULL, NULL, mid, CIS_RESPONSE_EXECUTE);
        }
        else
        {
            return;
        }
    }
    break;
    case SAMPLE_OID_B:
    {
        if (uri->instanceId > SAMPLE_B_INSTANCE_COUNT)
        {
            return;
        }
        st_instance_b *inst = &g_instList_b[uri->instanceId];
        if (inst == NULL || inst->enabled == false)
        {
            return;
        }

        if (uri->resourceId == actionB_1)
        {
            /*
             *\call action;
             */
            LOGI("exec actionB_1");
            cis_response(context, NULL, NULL, mid, CIS_RESPONSE_EXECUTE);
        }
        else
        {
            return;
        }
    }
    break;
    }
}

static void prv_paramsResponse(void *context, cis_uri_t *uri, cis_observe_attr_t parameters, cis_mid_t mid)
{
    uint8_t           index;
    st_sample_object *object = NULL;

    if (CIS_URI_IS_SET_RESOURCE(uri))
    {
        LOGI("prv_params:(%d/%d/%d)", uri->objectId, uri->instanceId, uri->resourceId);
    }

    if (!CIS_URI_IS_SET_INSTANCE(uri))
    {
        return;
    }

    for (index = 0; index < SAMPLE_OBJECT_MAX; index++)
    {
        if (g_objectList[index].oid == uri->objectId)
        {
            object = &g_objectList[index];
        }
    }

    if (object == NULL)
    {
        return;
    }

    /*set parameter to observe resource*/
    /*do*/

    LOGI("set:%x,clr:%x", parameters.toSet, parameters.toClear);
    LOGI("min:%d,max:%d,gt:%f,lt:%f,st:%f",
         parameters.minPeriod,
         parameters.maxPeriod,
         parameters.greaterThan,
         parameters.lessThan,
         parameters.step);

    cis_response(context, NULL, NULL, mid, CIS_RESPONSE_OBSERVE_PARAMS);
}

static cis_data_t *sample_dataDup(const cis_data_t *value, cis_attrcount_t attrcount)
{
    cis_data_t *newData;
    newData = (cis_data_t *)cis_malloc(attrcount * sizeof(cis_data_t));
    if (newData == NULL)
    {
        return NULL;
    }
    cis_attrcount_t index;
    for (index = 0; index < attrcount; index++)
    {
        newData[index].id              = value[index].id;
        newData[index].type            = value[index].type;
        newData[index].asBuffer.length = value[index].asBuffer.length;
        newData[index].asBuffer.buffer = (uint8_t *)cis_malloc(value[index].asBuffer.length);
        cis_memcpy(newData[index].asBuffer.buffer, value[index].asBuffer.buffer, value[index].asBuffer.length);

        cis_memcpy(&newData[index].value.asInteger, &value[index].value.asInteger, sizeof(newData[index].value));
    }
    return newData;
}

static cis_coapret_t cis_onRead(void *context, cis_uri_t *uri, cis_mid_t mid)
{
    struct st_callback_info *newNode = (struct st_callback_info *)cis_malloc(sizeof(struct st_callback_info));
    newNode->next                    = NULL;
    newNode->flag                    = SAMPLE_CALLBACK_READ;
    newNode->mid                     = mid;
    newNode->uri                     = *uri;
    g_callbackList                   = (struct st_callback_info *)CIS_LIST_ADD(g_callbackList, newNode);

    LOGI("cis_onRead:(%d/%d/%d)", uri->objectId, uri->instanceId, uri->resourceId);

    return CIS_CALLBACK_CONFORM;
}

static cis_coapret_t cis_onDiscover(void *context, cis_uri_t *uri, cis_mid_t mid)
{

    struct st_callback_info *newNode = (struct st_callback_info *)cis_malloc(sizeof(struct st_callback_info));
    newNode->next                    = NULL;
    newNode->flag                    = SAMPLE_CALLBACK_DISCOVER;
    newNode->mid                     = mid;
    newNode->uri                     = *uri;
    g_callbackList                   = (struct st_callback_info *)CIS_LIST_ADD(g_callbackList, newNode);

    LOGI("cis_onDiscover:(%d/%d/%d)", uri->objectId, uri->instanceId, uri->resourceId);

    return CIS_CALLBACK_CONFORM;
}

static cis_coapret_t
cis_onWrite(void *context, cis_uri_t *uri, const cis_data_t *value, cis_attrcount_t attrcount, cis_mid_t mid)
{
    if (CIS_URI_IS_SET_RESOURCE(uri))
    {
        LOGI("cis_onWrite:(%d/%d/%d)", uri->objectId, uri->instanceId, uri->resourceId);
    }
    else
    {
        LOGI("cis_onWrite:(%d/%d)", uri->objectId, uri->instanceId);
    }

    struct st_callback_info *newNode = (struct st_callback_info *)cis_malloc(sizeof(struct st_callback_info));
    newNode->next                    = NULL;
    newNode->flag                    = SAMPLE_CALLBACK_WRITE;
    newNode->mid                     = mid;
    newNode->uri                     = *uri;
    newNode->param.asWrite.count     = attrcount;
    newNode->param.asWrite.value     = sample_dataDup(value, attrcount);
    g_callbackList                   = (struct st_callback_info *)CIS_LIST_ADD(g_callbackList, newNode);

    return CIS_CALLBACK_CONFORM;
}

static cis_coapret_t cis_onExec(void *context, cis_uri_t *uri, const uint8_t *value, uint32_t length, cis_mid_t mid)
{
    if (CIS_URI_IS_SET_RESOURCE(uri))
    {
        LOGI("cis_onExec:(%d/%d/%d)", uri->objectId, uri->instanceId, uri->resourceId);
    }
    else
    {
        return CIS_CALLBACK_METHOD_NOT_ALLOWED;
    }

    if (!CIS_URI_IS_SET_INSTANCE(uri))
    {
        return CIS_CALLBACK_BAD_REQUEST;
    }

    struct st_callback_info *newNode = (struct st_callback_info *)cis_malloc(sizeof(struct st_callback_info));
    newNode->next                    = NULL;
    newNode->flag                    = SAMPLE_CALLBACK_EXECUTE;
    newNode->mid                     = mid;
    newNode->uri                     = *uri;
    newNode->param.asExec.buffer     = (uint8_t *)cis_malloc(length);
    newNode->param.asExec.length     = length;
    cis_memcpy(newNode->param.asExec.buffer, value, length);
    g_callbackList = (struct st_callback_info *)CIS_LIST_ADD(g_callbackList, newNode);

    return CIS_CALLBACK_CONFORM;
}

static cis_coapret_t cis_onObserve(void *context, cis_uri_t *uri, bool flag, cis_mid_t mid)
{
    LOGI("cis_onObserve mid:%d uri:(%d/%d/%d)",
         mid,
         uri->objectId,
         CIS_URI_IS_SET_INSTANCE(uri) ? uri->instanceId : -1,
         CIS_URI_IS_SET_RESOURCE(uri) ? uri->resourceId : -1);

    struct st_callback_info *newNode = (struct st_callback_info *)cis_malloc(sizeof(struct st_callback_info));
    newNode->next                    = NULL;
    newNode->flag                    = SAMPLE_CALLBACK_OBSERVE;
    newNode->mid                     = mid;
    newNode->uri                     = *uri;
    newNode->param.asObserve.flag    = flag;

    g_callbackList = (struct st_callback_info *)CIS_LIST_ADD(g_callbackList, newNode);

    return CIS_CALLBACK_CONFORM;
}

static cis_coapret_t cis_onParams(void *context, cis_uri_t *uri, cis_observe_attr_t parameters, cis_mid_t mid)
{
    if (CIS_URI_IS_SET_RESOURCE(uri))
    {
        LOGI("cis_on_params_uri:(%d/%d/%d)", uri->objectId, uri->instanceId, uri->resourceId);
        LOGI("set:%x,clr:%x", parameters.toSet, parameters.toClear);
        LOGI("min:%d,max:%d,gt:%f,lt:%f,st:%f",
             parameters.minPeriod,
             parameters.maxPeriod,
             parameters.greaterThan,
             parameters.lessThan,
             parameters.step);
    }

    if (!CIS_URI_IS_SET_INSTANCE(uri))
    {
        return CIS_CALLBACK_BAD_REQUEST;
    }

    struct st_callback_info *newNode     = (struct st_callback_info *)cis_malloc(sizeof(struct st_callback_info));
    newNode->next                        = NULL;
    newNode->flag                        = SAMPLE_CALLBACK_SETPARAMS;
    newNode->mid                         = mid;
    newNode->uri                         = *uri;
    newNode->param.asObserveParam.params = parameters;
    g_callbackList                       = (struct st_callback_info *)CIS_LIST_ADD(g_callbackList, newNode);

    return CIS_CALLBACK_CONFORM;
}

static void cis_onEvent(void *context, cis_evt_t eid, void *param)
{
    /* st_context_t* ctx = (st_context_t*)context; */

    LOGI("cis_on_event(%d):%s", eid, STR_EVENT_CODE(eid));
    switch (eid)
    {
    case CIS_EVENT_RESPONSE_FAILED:
        LOGI("cis_on_event response failed mid:%d", (int32_t)param);
        break;
    case CIS_EVENT_NOTIFY_FAILED:
        LOGI("cis_on_event notify failed mid:%d", (int32_t)param);
        break;
    case CIS_EVENT_UPDATE_NEED:
        LOGI("cis_on_event need to update,reserve time:%ds", (int32_t)param);
        cis_update_reg(g_context,
                       LIFETIME_INVALID,
                       true); /* true: updates object while update lifetime; false: only update lifetime */
        break;
    case CIS_EVENT_REG_SUCCESS:
    {
        struct st_observe_info *delnode;
        while (g_observeList != NULL)
        {
            g_observeList = (struct st_observe_info *)CIS_LIST_RM((cis_list_t *)g_observeList,
                                                                  g_observeList->mid,
                                                                  (cis_list_t **)&delnode);
            cis_free(delnode);
        }
    }
    break;
    case CIS_EVENT_UNREG_DONE:
        g_shutdown = true;
        LOGI("cis_on_event unregister done");
        break;
    default:
        break;
    }
}

int sample_delete_object(void)
{
    st_context_t *contextP = (st_context_t *)g_context;
    cis_oid_t     objectid;
    st_object_t  *targetP;

    objectid = SAMPLE_OID_B;

    targetP = (st_object_t *)CIS_LIST_FIND(contextP->objectList, objectid);
    if (targetP == NULL)
    {
        return CIS_RET_NOT_FOUND;
    }

    cis_delobject(g_context, objectid);

    return CIS_RET_OK;
}

int sample_add_object(void)
{
    int               index = 1;
    cis_inst_bitmap_t bitmap;
    cis_res_count_t   rescount;
    cis_instcount_t   instCount, instBytes;
    const char       *instAsciiPtr;
    uint8_t          *instPtr;
    cis_oid_t         oid;
    int16_t           i;
    st_sample_object *obj = &g_objectList[index];

    oid       = obj->oid;
    instCount = utils_strlen(obj->instBitmap);
    instBytes = (instCount - 1) / 8 + 1;
    instAsciiPtr = obj->instBitmap;
    instPtr      = (uint8_t *)cis_malloc(instBytes);
    cissys_assert(instPtr != NULL);
    cis_memset(instPtr, 0, instBytes);

    for (i = 0; i < instCount; i++)
    {
        cis_instcount_t instBytePos    = i / 8;
        cis_instcount_t instByteOffset = 7 - (i % 8);
        if (instAsciiPtr[i] == '1')
        {
            instPtr[instBytePos] += 0x01 << instByteOffset;
        }
    }

    bitmap.instanceCount  = instCount;
    bitmap.instanceBitmap = instPtr;
    bitmap.instanceBytes  = instBytes;

    rescount.attrCount = obj->attrCount;
    rescount.actCount  = obj->actCount;

    cis_addobject(g_context, oid, &bitmap, &rescount);
    cis_free(instPtr);

    return CIS_RET_OK;
}

void cis_sample_exit(void)
{
    g_doUnregister = true;
}
