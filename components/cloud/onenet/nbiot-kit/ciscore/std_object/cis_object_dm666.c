/*******************************************************************************
 *
 * This object is single instance only, and provide dm info functionality.
 * Object ID is 666.
 * Object instance ID is 0.
 */

#include "../cis_api.h"
#include "../cis_internals.h"
#include "std_object.h"
#include "dm_endpoint.h" 


#if CIS_ENABLE_DM

#define RES_DM_DEV_INFO                  6601
#define RES_DM_APP_INFO                  6602
#define RES_DM_MAC                       6603
#define RES_DM_ROM                       6604
#define RES_DM_RAM                       6605
#define RES_DM_CPU                       6606
#define RES_DM_SYS_VERSION               6607
#define RES_DM_FIRM_VERSION              6608
#define RES_DM_FIRM_NAME                 6609
#define RES_DM_VOLTE                     6610
#define RES_DM_NET_TYPE                  6611
#define RES_DM_NET_ACCT                  6612
#define RES_DM_PHONE                     6613
#define RES_DM_LOCATION                  6614

#define RES_VALUE_BUFF_LEN  50

typedef struct _dm666_data_
{
    struct _dm666_data_ * next;        // matches st_list_t::next
    cis_listid_t             instanceId;  // matches st_list_t::id
    char dev_info[RES_VALUE_BUFF_LEN + 1];
    char app_info[RES_VALUE_BUFF_LEN + 1];
    char mac[RES_VALUE_BUFF_LEN + 1];
    char rom[RES_VALUE_BUFF_LEN + 1];
    char ram[RES_VALUE_BUFF_LEN + 1];
    char cpu[RES_VALUE_BUFF_LEN + 1];
    char sys_ver[RES_VALUE_BUFF_LEN + 1];
    char firm_ver[RES_VALUE_BUFF_LEN + 1];
    char firm_name[RES_VALUE_BUFF_LEN + 1];
    char volte[RES_VALUE_BUFF_LEN + 1];
    char net_type[RES_VALUE_BUFF_LEN + 1];
    char net_acct[RES_VALUE_BUFF_LEN + 1];
    char phone[RES_VALUE_BUFF_LEN + 1];
    char location[RES_VALUE_BUFF_LEN + 1];
} dm666_data_t;

void std_dm666_encode(char *encode_out, char *encode_in, int len)
{
    char *outbuff = NULL;
    to_encode(encode_in, &outbuff);
    cis_memcpy(encode_out, outbuff, len);
    if (outbuff != NULL) free(outbuff);
}

bool std_dm666_create(st_context_t * contextP,
                       int instanceId,
                       st_object_t * dm666Obj)
{
    dm666_data_t * instConn=NULL;
    dm666_data_t * targetP = NULL;
    uint8_t instBytes = 0;
    uint8_t instCount = 0;
    cis_iid_t instIndex;
    if (NULL == dm666Obj)
    {
        return false;   
    }

    // Manually create a hard-code instance
    targetP = (dm666_data_t *)cis_malloc(sizeof(dm666_data_t));
    if (NULL == targetP)
    {
        return false;
    }

    cis_memset(targetP, 0, sizeof(dm666_data_t));
    targetP->instanceId = instanceId;

    std_dm666_encode(targetP->app_info, "app_name|package|30000|1\r\n", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->mac, "mac", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->rom, "rom", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->ram, "ram", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->cpu, "cpu", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->sys_ver, "sys_ver", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->firm_ver, "firm_ver", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->firm_name, "firm_name", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->volte, "volte", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->net_type, "net_type", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->net_acct, "net_acct", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->phone, "phone", RES_VALUE_BUFF_LEN);
    std_dm666_encode(targetP->location, "39.9,116.3,1", RES_VALUE_BUFF_LEN);

    LOGD("dm666 std_object_put_dm666=%x, targetP=%x", contextP->dm666_inst, targetP);
    instConn = (dm666_data_t * )std_object_put_dm666(contextP,(cis_list_t*)targetP);
    LOGD("dm666 std_object_put_dm666=%x", contextP->dm666_inst);

    instCount = CIS_LIST_COUNT(instConn);
    if(instCount == 0)
    {
        cis_free(targetP);
        LOGD("dm666 instCount 0");
        return false;
    }

    if(instCount == 1)
    {
        LOGD("dm666 instCount 1");
        return true;
    }

    dm666Obj->instBitmapCount = instCount;
    instBytes = (instCount - 1) / 8 + 1;
    if(dm666Obj->instBitmapBytes < instBytes){
        if(dm666Obj->instBitmapBytes != 0 && dm666Obj->instBitmapPtr != NULL)
        {
            cis_free(dm666Obj->instBitmapPtr);
        }
        dm666Obj->instBitmapPtr = (uint8_t*)cis_malloc(instBytes);
        dm666Obj->instBitmapBytes = instBytes;
    }
    cissys_memset(dm666Obj->instBitmapPtr,0,instBytes);
    targetP = instConn;
    for (instIndex = 0;instIndex < instCount;instIndex++)
    {
        uint8_t instBytePos = targetP->instanceId / 8;
        uint8_t instByteOffset = 7 - (targetP->instanceId % 8);
        dm666Obj->instBitmapPtr[instBytePos] += 0x01 << instByteOffset;

        targetP = targetP->next;
    }
    return true;
}

static uint8_t prv_dm666_get_value(st_context_t * contextP,
                                    st_data_t * dataArrayP,
                                    int number,
                                    dm666_data_t * devDataP)
{
    uint8_t result;
    uint16_t resId;
    st_data_t *dataP;

    for (int i = 0; i<number;i++)
    {
        if(number == 1)
        {
            resId = dataArrayP->id;
            dataP = dataArrayP;
        }
        else
        {
            resId = dataArrayP->value.asChildren.array[i].id;
            dataP = dataArrayP->value.asChildren.array+i;
        }
        switch (resId)//need do error handle
        {
        case RES_DM_DEV_INFO:
            {
                data_encode_string(devDataP->dev_info, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_APP_INFO:
            {
                data_encode_string(devDataP->app_info, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_MAC:
            {
                data_encode_string(devDataP->mac, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_ROM:
            {
                data_encode_string(devDataP->rom, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_RAM:
            {
                data_encode_string(devDataP->ram, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_CPU:
            {
                data_encode_string(devDataP->cpu, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_SYS_VERSION:
            {
                data_encode_string(devDataP->sys_ver, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_FIRM_VERSION:
            {
                data_encode_string(devDataP->firm_ver, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_FIRM_NAME:
            {
                data_encode_string(devDataP->firm_name, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_VOLTE:
            {
                data_encode_string(devDataP->volte, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_NET_TYPE:
            {
                data_encode_string(devDataP->net_type, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_NET_ACCT:
            {
                data_encode_string(devDataP->net_acct, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_PHONE:
            {
                data_encode_string(devDataP->phone, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        case RES_DM_LOCATION:
            {
                data_encode_string(devDataP->location, dataP);
                result = COAP_205_CONTENT;
                break;
            }

        default:
            LOGD("dm666 error");
            return COAP_404_NOT_FOUND;
        }
    }
    return result;
}

static dm666_data_t * prv_dm666_find(st_context_t * contextP,cis_iid_t instanceId)
{
    dm666_data_t * targetP;
    LOGD("dm666 std_object_get_dm666=%x", contextP->dm666_inst);
    targetP = (dm666_data_t *)(std_object_get_dm666(contextP, instanceId));
    LOGD("dm666 std_object_get_dm666=%x", contextP->dm666_inst);

    if (NULL != targetP)
    {
        return targetP;
    }

    return NULL;
}

uint8_t std_dm666_read(st_context_t * contextP,
                        uint16_t instanceId,
                        int * numDataP,
                        st_data_t ** dataArrayP,
                        st_object_t * objectP)
{
    uint8_t result;
    int i;
    dm666_data_t * targetP;
    targetP = prv_dm666_find(contextP,instanceId);
    // this is a single instance object
    if (instanceId != 0||targetP==NULL)
    {
        LOGD("dm666 not found instanceId=%d", instanceId);
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
            //RES_DM_DEV_INFO,
            RES_DM_APP_INFO,
            RES_DM_MAC,
            RES_DM_ROM,
            RES_DM_RAM,
            RES_DM_CPU,
            RES_DM_SYS_VERSION,
            RES_DM_FIRM_VERSION,
            RES_DM_FIRM_NAME,
            RES_DM_VOLTE,
            RES_DM_NET_TYPE,
            RES_DM_NET_ACCT,
            RES_DM_PHONE,
            RES_DM_LOCATION
        };
        int nbRes = sizeof(resList) / sizeof(uint16_t);
        LOGD("dm666 nbRes=%d", nbRes);
        (*dataArrayP)->id = 0;
        (*dataArrayP)->type = DATA_TYPE_OBJECT_INSTANCE;
        (*dataArrayP)->value.asChildren.count = nbRes;
        (*dataArrayP)->value.asChildren.array = data_new(nbRes);
        cis_memset((*dataArrayP)->value.asChildren.array, 0, (nbRes) * sizeof(cis_data_t));
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;

        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0; i < nbRes; i++)
        {
            (*dataArrayP)->value.asChildren.array[i].id = resList[i];
        }

        if ((result = prv_dm666_get_value(contextP, (*dataArrayP), nbRes, targetP)) != COAP_205_CONTENT)
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
    }
    else
    {
        result = prv_dm666_get_value(contextP, (*dataArrayP), 1, targetP);
    }

    LOGD("dm666 result=%d", result);

    return result;
}

uint8_t std_dm666_discover(st_context_t * contextP,
                            uint16_t instanceId,
                            int * numDataP,
                            st_data_t ** dataArrayP,
                            st_object_t * objectP)
{
    uint8_t result;
    int i;

    // this is a single instance object
    if (instanceId != 0)
    {
        return COAP_404_NOT_FOUND;
    }

    result = COAP_205_CONTENT;

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
            //RES_DM_DEV_INFO,
            RES_DM_APP_INFO,
            RES_DM_MAC,
            RES_DM_ROM,
            RES_DM_RAM,
            RES_DM_CPU,
            RES_DM_SYS_VERSION,
            RES_DM_FIRM_VERSION,
            RES_DM_FIRM_NAME,
            RES_DM_VOLTE,
            RES_DM_NET_TYPE,
            RES_DM_NET_ACCT,
            RES_DM_PHONE,
            RES_DM_LOCATION
        };
        int nbRes = sizeof(resList) / sizeof(uint16_t);

        (*dataArrayP)->id = 0;
        (*dataArrayP)->type = DATA_TYPE_OBJECT_INSTANCE;
        (*dataArrayP)->value.asChildren.count = nbRes;
        (*dataArrayP)->value.asChildren.array =  data_new(nbRes);
        cis_memset((*dataArrayP)->value.asChildren.array,0,(nbRes)*sizeof(cis_data_t));
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0; i < nbRes; i++)
        {
            (*dataArrayP)->value.asChildren.array[i].id = resList[i];
            (*dataArrayP)->value.asChildren.array[i].type = DATA_TYPE_LINK;
            (*dataArrayP)->value.asChildren.array[i].value.asObjLink.objectId = CIS_DM666_OBJECT_ID;
            (*dataArrayP)->value.asChildren.array[i].value.asObjLink.instId = 0;
        }
    }
    else
    {
        for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
        {
            switch ((*dataArrayP)[i].id)
            {
            //case RES_DM_DEV_INFO:
            case RES_DM_APP_INFO:
            case RES_DM_MAC:
            case RES_DM_ROM:
            case RES_DM_RAM:
            case RES_DM_CPU:
            case RES_DM_SYS_VERSION:
            case RES_DM_FIRM_VERSION:
            case RES_DM_FIRM_NAME:
            case RES_DM_VOLTE:
            case RES_DM_NET_TYPE:
            case RES_DM_NET_ACCT:
            case RES_DM_PHONE:
            case RES_DM_LOCATION:
                break;
            default:
                result = COAP_404_NOT_FOUND;
            }
        }
    }

    return result;
}

void std_dm666_clean(st_context_t * contextP)
{
    dm666_data_t * deleteInst;
    dm666_data_t * dm666Instance = NULL;
    dm666Instance = (dm666_data_t *)(contextP->dm666_inst);
    while (dm666Instance != NULL)
    {
        deleteInst = dm666Instance;
        dm666Instance = dm666Instance->next;
        LOGD("dm666 std_object_remove_dm666=%x", contextP->dm666_inst);
        std_object_remove_dm666(contextP,(cis_list_t*)(deleteInst));
        LOGD("dm666 std_object_remove_dm666=%x", contextP->dm666_inst);
        cis_free(deleteInst);
    }
}
#endif
