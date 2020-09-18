/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        cis_if_sys.c
 *
 * @brief       system port file for cis
 *
 * @details     
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <cis_if_sys.h>
#include <cis_def.h>
#include <cis_internals.h>
#include <cis_config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "os_kernel.h"
#include "os_sem.h"
#include "cis_log.h"
#include "cis_list.h"
#include <sys/time.h>

#if defined(NET_USING_MODULES)
#include <arpa/inet.h>
#endif

#ifdef __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__
#include "modem_switch.h"
#endif
#if CIS_ENABLE_CMIOT_OTA
#include "std_object/std_object.h"
#endif

#if defined(NET_USING_MOLINK) && !defined(MOLINK_USING_ESP8266)
    #include <mo_api.h>
#endif

void testThread(void *lpParam);

static char cissys_imsi[20] = {0};
static char cissys_imei[20] = {0};

extern void *g_lockImei;
extern void *g_lockImsi;

int get_module_imsi()
{
#if defined(NET_USING_MOLINK) && !defined(MOLINK_USING_ESP8266)

    mo_object_t  *defmo_obj  = OS_NULL;

    defmo_obj = mo_get_default(); /* get atintf obj and ops interface */
    if (mo_get_imsi(defmo_obj, cissys_imsi, sizeof(cissys_imsi)) != OS_EOK) /* get IMSI number */
    {
        LOGD("AT tool: get IMSI number fail\n");
        return OS_ERROR;
    }
#elif defined(NET_USING_LWIP)
    strncpy(cissys_imsi, "460041912306456", 15);
#endif
    LOGI("get module imsi: %s", cissys_imsi);

    return OS_EOK;
}

int get_module_imei()
{
#if defined(NET_USING_MOLINK) && !defined(MOLINK_USING_ESP8266)

    mo_object_t  *defmo_obj  = OS_NULL;

    defmo_obj = mo_get_default(); /* get atintf obj and ops interface */
    if (mo_get_imei(defmo_obj, cissys_imei, sizeof(cissys_imei)) != OS_EOK) /* get IMEI number */
    {
        LOGD("AT tool: get IMEI number fail\n");
        return OS_ERROR;
    }
#elif defined(NET_USING_LWIP)
    strncpy(cissys_imei, "862521040003107", 15);
#endif
    LOGI("get module imei: %s", cissys_imei);

    return OS_EOK;
}

cis_ret_t cissys_request_endpoint_name(void)
{
    cissys_lock(g_lockImsi, CIS_CONFIG_LOCK_INFINITY);
#if defined(__NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__) && defined(MTK_SOCKET_AGENT_SUPPORT)
    modem_switch_network_type_enum_t network_type = modem_switch_get_available_service();
    LOGD("mode switch network type: %d", network_type);
    if (network_type == MODEM_SWITCH_NETWORK_TYPE_2G)
    {
#if defined(__RIL_GSM_CHANNEL_ENABLE__)
        ril_request_gsm_imsi(RIL_ACTIVE_MODE, cissys_cimi_callback, NULL);
#else
        ril_request_imsi(RIL_ACTIVE_MODE, cissys_cimi_callback, NULL);
#endif
    }
    else
    {
        ril_request_imsi(RIL_ACTIVE_MODE, cissys_cimi_callback, NULL);
    }
#else  /* __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__ */

    get_module_imsi();
    cissys_unlock(g_lockImsi);
#endif /* __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__ */

    cissys_lock(g_lockImei, CIS_CONFIG_LOCK_INFINITY);
    get_module_imei();
    cissys_unlock(g_lockImei);

    return 1;
}

#if CIS_ENABLE_CMIOT_OTA
#define CIS_CMIOT_OTA_WAIT_UPLINK_FINISHED 1000
#define CIS_CMIOT_OTA_START_QUERY_INTERVAL 10000
#define CIS_CMIOT_OTA_START_QUERY_LOOP     6
#define CIS_CMIOT_NEWIMSI_STR              "460040983303979"
extern uint8_t cissys_changeIMSI(char *buffer, uint32_t len);

void                  testCmiotOta(void *lpParam);
uint8_t               std_poweruplog_ota_notify(st_context_t *contextP, cis_oid_t *objectid, uint16_t mid);
uint8_t               cissys_remove_psm(void);
cis_ota_history_state cissys_quary_ota_finish_state(void);
uint8_t               cissys_change_ota_finish_state(uint8_t flag);
uint8_t               cissys_recover_psm(void);

extern cis_ota_history_state  g_cmiot_otafinishstate_flag;
extern void                  *g_context;

void testCmiotOta(void *lpParam)
{
    cis_oid_t           test_obj_id            = CIS_POWERUPLOG_OBJECT_ID;
    cis_cmiot_ota_state test_monitor_ota_state = CMIOT_OTA_STATE_IDIL;
    cissys_assert(g_context != NULL);
    st_context_t      *ctx = (st_context_t *)g_context;
    poweruplog_data_t *targetP;
    char              *NewImsiStr;

    while (1)
    {
        uint8_t ota_queri_loop;
        bool    ota_imsi_change_stat = FALSE;
        test_monitor_ota_state       = ctx->cmiotOtaState;
        uint8_t len_imsi             = 0;
        switch (test_monitor_ota_state)
        {
        case CMIOT_OTA_STATE_IDIL:
        {
            Sleep(5000);
        }
        break;

        case CMIOT_OTA_STATE_START:
        {
            targetP = (poweruplog_data_t *)(std_object_get_poweruplog(ctx, 0));
            if (NULL != targetP)
            {
                printf("UE is trigged to OTA start state and waiting the OTA procedule finished. \r\n");

                printf("OTA started with current IMSI: %s . \r\n", targetP->IMSI);

                NewImsiStr = (char *)cis_malloc(NBSYS_IMSI_MAXLENGTH);
                if (NewImsiStr == NULL)
                {
                    cis_free(NewImsiStr);
                }
                cis_memset(NewImsiStr, 0, NBSYS_IMSI_MAXLENGTH);

                Sleep(CIS_CMIOT_OTA_WAIT_UPLINK_FINISHED);

                cis_unregister(ctx);

                cissys_remove_psm();

                Sleep(CIS_CMIOT_OTA_START_QUERY_INTERVAL);

                for (ota_queri_loop = 0; ota_queri_loop < CIS_CMIOT_OTA_START_QUERY_LOOP; ota_queri_loop++)
                {
                    Sleep(CIS_CMIOT_OTA_START_QUERY_INTERVAL);
                    cis_memset(NewImsiStr, 0, NBSYS_IMSI_MAXLENGTH);
                    len_imsi = cissys_getIMSI(NewImsiStr, NBSYS_IMSI_MAXLENGTH);

                    if (cis_memcmp(targetP->IMSI, NewImsiStr, len_imsi) != 0)
                    {
                        ota_imsi_change_stat = TRUE;
                        cis_memset(targetP->IMSI, 0, NBSYS_IMSI_MAXLENGTH);
                        cis_memcpy(targetP->IMSI, NewImsiStr, len_imsi);
                        break;
                    }
                    /* change the IMSI for test */
                    if (ota_queri_loop == 3)
                    {
                        cis_memcpy(NewImsiStr, CIS_CMIOT_NEWIMSI_STR, NBSYS_IMSI_MAXLENGTH);
                        cissys_changeIMSI(NewImsiStr, NBSYS_IMSI_MAXLENGTH);
                    }
                }
                if (ota_imsi_change_stat == TRUE)
                {
                    core_callbackEvent(ctx, CIS_EVENT_CMIOT_OTA_SUCCESS, NULL);
                    printf("OTA success detected. \r\n");
                }
                else
                {
                    core_callbackEvent(ctx, CIS_EVENT_CMIOT_OTA_FAIL, NULL);
                    printf("OTA fail detected. \r\n");
                }
                printf("OTA finished with current IMSI: %s . \r\n", targetP->IMSI);
                cis_free(NewImsiStr);
            }

            ctx->registerEnabled = true;
            printf("Reregister to ONENET to update OTA result. \r\n");
        }
        break;

        case CMIOT_OTA_STATE_SUCCESS:
        case CMIOT_OTA_STATE_FAIL:
        {
        }
        break;

        case CMIOT_OTA_STATE_FINISH:
        {
            cis_unregister(g_context);
            printf("OTA Finish detected.Unresgister from OneNET \r\n");
            cis_deinit(&g_context);
            return 0;
        }
        break;

        default:
            Sleep(5000);
        }

        /* std_poweruplog_ota_notify((st_context_t *)g_context,&test_obj_id, cmiot_ota_observe_mid); */
    }
}

#endif    /* CIS_ENABLE_CMIOT_OTA */

cis_ret_t cissys_init(void *context, const cis_cfg_sys_t *cfg, cissys_callback_t *event_cb)
{
    st_context_t *ctx = (struct st_cis_context *)context;
    cis_memcpy(&(ctx->g_sysconfig), cfg, sizeof(cis_cfg_sys_t));

#if CIS_ENABLE_CMIOT_OTA
    /* if(TRUE!=ctx->isDM) */
    {
        xTaskCreate(testCmiotOta, "cis_ota", 2048 / sizeof(portSTACK_TYPE), NULL, TASK_PRIORITY_NORMAL, NULL);
    }
#endif    /* CIS_ENABLE_CMIOT_OTA */

#if CIS_ENABLE_UPDATE
    if (event_cb != NULL)
    {
        ctx->g_fotacallback.onEvent  = event_cb->onEvent;
        ctx->g_fotacallback.userData = event_cb->userData;
#if CIS_ENABLE_DM
        if (TRUE != ctx->isDM)
#endif
        {
            xTaskCreate(testThread,
                        "cis_fota",
                        2048 / sizeof(portSTACK_TYPE),
                        &(ctx->g_fotacallback),
                        TASK_PRIORITY_NORMAL,
                        NULL);
        }
    }
#endif
    return 1;
}

uint32_t cissys_gettime()
{
    struct timeval tv = {0};

    gettimeofday(&tv, NULL);
    if (0 == tv.tv_sec)
    {
        return (uint32_t)-1;
    }

    return tv.tv_sec * 1000;
}

void cissys_sleepms(uint32_t ms)
{
    os_task_mdelay(ms);
}

clock_t cissys_tick(void)
{
    return os_tick_get();
}

void cissys_logwrite(uint8_t *buffer, uint32_t length)
{
    os_kprintf((const char *)buffer);
}

void *cissys_malloc(size_t length)
{
    return os_malloc(length);
}

void cissys_free(void *buffer)
{
    os_free(buffer);
}

void *cissys_memset(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

void *cissys_memcpy(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

void *cissys_memmove(void *dst, const void *src, size_t n)
{
    return memmove(dst, src, n);
}

int cissys_memcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

void cissys_fault(uint16_t id)
{
    LOGE("fall in cissys_fault.id=%d", id);
    OS_ASSERT(0);
}

void cissys_assert(bool flag)
{
    OS_ASSERT(flag);
}

uint32_t cissys_rand()
{
    uint32_t random_seed;

#ifdef OS_USING_HWCRYPTO
#include "hwcrypto/crypto.h"
    random_seed = os_hwcrypto_rng_update();
#else
    random_seed = rand();
#endif

    return random_seed;
}

/*
#if CIS_ENABLE_PSK
uint8_t	cissys_getPSK(char* buffer, uint32_t maxlen)
{
    char* pBuffer = NULL;
    uint32_t readlen = 0;
    uint8_t len = 0;
    FILE* f = fopen("psk", "r");
    if (f == NULL)
    {
        return 0;
    }

    pBuffer = (char*)cissys_malloc(maxlen + 1);
    readlen = fread(pBuffer, 1, maxlen, f);
    if (readlen <= 0)
    {
        cissys_free(pBuffer);
        fclose(f);
        return 0;
    }
    pBuffer[readlen] = '\0';

    len = strlen(pBuffer);
    if (maxlen < len)return 0;
    cissys_memcpy(buffer, pBuffer, len);
    cissys_free(pBuffer);
    return len;
}
#endif
*/
uint8_t cissys_getIMEI(char *buffer, uint32_t maxlen)
{
    const char *str = cissys_imei;
    uint8_t     len = 0;

    if (!str[0])
    {
        str = "imeiwb2";
    }
    len = strlen(str);

    if (maxlen < len)
    {
        return 0;
    }
    memcpy(buffer, str, len);

    return len;
}
uint8_t cissys_getIMSI(char *buffer, uint32_t maxlen)
{
    const char *str = cissys_imsi;
    uint8_t     len = 0;

    if (!str[0])
    {
        str = "imsiwb1";
    }
    len = strlen(str);

    if (maxlen < len)
    {
        return 0;
    }
    memcpy(buffer, str, len);

    return len;
}

#if CIS_ENABLE_CMIOT_OTA

uint8_t cissys_changeIMSI(char *buffer, uint32_t len)
{
    FILE *f = fopen("imsi", "wb");
    if (f != NULL)
    {
        fwrite(buffer, 1, len, f);
        fclose(f);
        return true;
    }
    return false;
}

uint8_t cissys_getEID(char *buffer, uint32_t maxlen)
{
    char    *pBuffer = NULL;
    uint32_t readlen = 0;
    uint8_t  len     = 0;
    FILE    *f       = fopen("eid", "r");
    if (f == NULL)
    {
        return 0;
    }

    pBuffer = (char *)malloc(maxlen + 1);
    readlen = fread(pBuffer, 1, maxlen, f);
    if (readlen <= 0)
    {
        free(pBuffer);
        fclose(f);
        return 0;
    }
    pBuffer[readlen] = '\0';

    len = strlen(pBuffer);
    if (maxlen < len)
    {
        return 0;
    }
    memcpy(buffer, pBuffer, len);
    free(pBuffer);
    return len;
}

uint8_t cissys_remove_psm(void)
{
    return 1;
}

uint8_t cissys_recover_psm(void)
{
    return 1;
}

cis_ota_history_state cissys_quary_ota_finish_state(void)
{
    return g_cmiot_otafinishstate_flag;
}
uint8_t cissys_change_ota_finish_state(cis_ota_history_state flag)
{
    g_cmiot_otafinishstate_flag = flag;
    return 1;
}
#endif

void cissys_lockcreate(void **mutex)
{
    OS_ASSERT(mutex != NULL);

    os_sem_t *mutex_handle;
    mutex_handle = os_sem_create("onenetnb_lock", 1, OS_IPC_FLAG_FIFO);
    (*mutex)     = mutex_handle;
}

void cissys_lockdestory(void *mutex)
{
    os_sem_destroy((os_sem_t *)mutex);
}

cis_ret_t cissys_lock(void *mutex, uint32_t ms)
{
    OS_ASSERT(mutex != NULL);
    os_sem_wait((os_sem_t *)mutex, OS_IPC_WAITING_FOREVER);

    return CIS_RET_OK;
}

void cissys_unlock(void *mutex)
{
    os_sem_post((os_sem_t *)mutex);
}

bool cissys_save(uint8_t *buffer, uint32_t length)
{
    FILE *f = NULL;
    f       = fopen("d:\\cis_serialize.bin", "wb");
    if (f != NULL)
    {
        fwrite(buffer, 1, length, f);
        fclose(f);
        return true;
    }
    return false;
}

bool cissys_load(uint8_t *buffer, uint32_t length)
{
    uint32_t readlen;
    FILE    *f = fopen("d:\\cis_serialize.bin", "rb");
    if (f != NULL)
    {
        while (length)
        {
            readlen = fread(buffer, 1, length, f);
            if (readlen == 0)
            {
                break;
            }
            length -= readlen;
        }
        if (length == 0)
        {
            return true;
        }
    }
    return false;
}
char     VERSION[16] = "2.2.0";
uint32_t cissys_getFwVersion(uint8_t **version)
{
    int length = strlen(VERSION) + 1;
    cis_memcpy(*version, VERSION, length);
    return length;
}

#define DEFAULT_CELL_ID               (95)
#define DEFAULT_RADIO_SIGNAL_STRENGTH (99)

uint32_t cissys_getCellId(void)
{
    return DEFAULT_CELL_ID;
}

uint32_t cissys_getRadioSignalStrength(void)
{
    return DEFAULT_RADIO_SIGNAL_STRENGTH;
}

#if CIS_ENABLE_UPDATE

#define DEFAULT_BATTERY_LEVEL   (99)
#define DEFAULT_BATTERY_VOLTAGE (3800)
#define DEFAULT_FREE_MEMORY     (554990)

#if CIS_ENABLE_UPDATE_MCU
bool isupdatemcu = false;
#endif

int LENGTH = 0;
int RESULT = 0;
int STATE  = 0;

#if 0
int  writeCallback    = 0;
int  validateCallback = 0;
int  eraseCallback    = 0;
void testThread(void *lpParam)
{
    cissys_callback_t *cb = (cissys_callback_t *)lpParam;
    while (1)
    {
        if (writeCallback == 1)    // write callback
        {
            cissys_sleepms(2000);
            // FILE* f = NULL;
            // int i = 0;
            // OutputDebugString("cissys_writeFwBytes\n");

            // cissys_sleepms(100);
            // f = fopen("cis_serialize_package.bin","a+b");
            // fseek(f, 0, SEEK_END);
            // if(f != NULL)
            //{
            // fwrite(buffer,1,size,f);
            // fclose(f);
            // return true;
            //}
            writeCallback = 0;
            cb->onEvent(cissys_event_write_success, NULL, cb->userData, NULL);
            // cb->onEvent(cissys_event_write_fail,NULL,cb->userData,NULL);
        }
        else if (eraseCallback == 1)    // erase callback
        {
            cissys_sleepms(5000);
            eraseCallback = 0;
            // cb->onEvent(cissys_event_fw_erase_fail,0,cb->userData,NULL);
            cb->onEvent(cissys_event_fw_erase_success, 0, cb->userData, NULL);
        }
        else if (validateCallback == 1)    // validate
        {
            cissys_sleepms(3000);
            validateCallback = 0;
            // cb->onEvent(cissys_event_fw_validate_fail,0,cb->userData,NULL);
            cb->onEvent(cissys_event_fw_validate_success, 0, cb->userData, NULL);
        }
        /*else if(nThreadNo == 3)
        {
            cissys_sleepms(3000);
            g_syscallback.onEvent(cissys_event_fw_update_success,0,g_syscallback.userData,NULL);
            nThreadNo = -1;
        }*/
    }
}

#endif

uint32_t cissys_getFwBatteryLevel()
{
    return DEFAULT_BATTERY_LEVEL;
}

uint32_t cissys_getFwBatteryVoltage()
{
    return DEFAULT_BATTERY_VOLTAGE;
}
uint32_t cissys_getFwAvailableMemory()
{
    return DEFAULT_FREE_MEMORY;
}

int cissys_getFwSavedBytes()
{
    /* FILE * pFile;
     long size = 0;
     pFile = fopen ("F:\\cissys_writeFwBytes.bin","rb");
     if (pFile==NULL)
    	perror ("Error opening file");
     else
    {
    	fseek (pFile, 0, SEEK_END);
    	size=ftell (pFile);
    	fclose (pFile);
    }
     return size; */
    return LENGTH;
}

bool cissys_checkFwValidation(cissys_callback_t *cb)
{
    validateCallback = 1;
    /* cissys_sleepms(1000);
     cb->onEvent(cissys_event_fw_validate_success,0,cb->userData,NULL); */
    return true;
}

bool cissys_eraseFwFlash(cissys_callback_t *cb)
{
    eraseCallback = 1;
    /* cissys_sleepms(8000);
     LENGTH=0;
     cb->onEvent(cissys_event_fw_erase_success,0,cb->userData,NULL); */
    return true;
}

void cissys_ClearFwBytes(void)
{
    LENGTH = 0;
}

uint32_t cissys_writeFwBytes(uint32_t size, uint8_t *buffer, cissys_callback_t *cb)
{
    cissys_sleepms(2000);
    cb->onEvent(cissys_event_write_success, NULL, cb->userData, (int *)&size);
    return 1;
}

void cissys_savewritebypes(uint32_t size)
{
    LENGTH += size;
}

bool cissys_updateFirmware(cissys_callback_t *cb)
{
    cissys_sleepms(10000);
    cis_memcpy(VERSION, "1801102", sizeof("1801102"));
    return true;
}

bool cissys_readContext(cis_fw_context_t *context)
{
    context->result    = RESULT;
    context->savebytes = LENGTH;
    context->state     = STATE;
    /*uint32_t readlen;
    char buffer[10];
    FILE* f = fopen("f:\\cis_setFwUpdateResult.bin","rb");
    if(f != NULL)
    {
        fread(buffer,1,sizeof(buffer),f);
        sscanf(buffer,"%d",&(context->result));
    }
    fclose(f);

    f = fopen("f:\\cis_setFwState.bin","rb");
    if(f != NULL)
    {
        fread(buffer,1,sizeof(buffer),f);
        sscanf(buffer,"%d",&(context->state));
    }
    fclose(f);

    f = fopen("f:\\cis_setFwSavedBytes.bin","rb");
    if(f != NULL)
    {
        fread(buffer,1,sizeof(buffer),f);
        sscanf(buffer,"%d",&(context->savebytes));
    }
    fclose(f);*/
    return 1;
}

bool cissys_setFwState(uint8_t state)
{
    /* FILE* f = NULL;
     int i = 0;
     char buffer[10];
     OutputDebugString("cissys_setFwState\n");
     sprintf(buffer,"%d",state);
     f = fopen("f:\\cis_setFwState.bin","wb");
    fseek(f, 0, SEEK_END);
     if(f != NULL)
    {
    	fwrite(buffer,1,1,f);
    	fclose(f);
    }*/
    STATE = state;
    return true;
}
bool cissys_setFwUpdateResult(uint8_t result)
{
    /* FILE* f = NULL;
     int i = 0;
     char buffer[10];
     OutputDebugString("cissys_setFwUpdateResult\n");
     sprintf(buffer,"%d",result);
     f = fopen("f:\\cis_setFwUpdateResult.bin","wb");
    fseek(f, 0, SEEK_END);
     if(f != NULL)
    {
    	fwrite(buffer,1,1,f);
    	fclose(f);
    }*/
    RESULT = result;
    return true;
}

#if CIS_ENABLE_UPDATE_MCU

char     SOTA_VERSION[16] = "0.0.0";
uint32_t MCU_SotaSize     = 100;

bool cissys_setSwState(bool ismcu)
{
    isupdatemcu = ismcu;
    return true;
}

bool cissys_getSwState(void)
{

    return isupdatemcu;
}

void cissys_setSotaMemory(uint32_t size)
{
    MCU_SotaSize = size;
}

uint32_t cissys_getSotaMemory(void)
{
    return MCU_SotaSize;
}

uint32_t cissys_getSotaVersion(uint8_t **version)
{
    int length = strlen(SOTA_VERSION) + 1;
    cis_memcpy(*version, SOTA_VERSION, length);
    return length;
}

void cissys_setSotaVersion(char *version)
{
    int length = strlen(version) + 1;
    cis_memcpy(SOTA_VERSION, version, length);
}

#endif

#endif    /* CIS_ENABLE_UPDATE */

#if CIS_ENABLE_MONITER
/*
int8_t      cissys_getRSRP()
{
    return 0;
}
*/
int8_t cissys_getSINR()
{
    return 0;
}
int8_t cissys_getBearer()
{
    return 0;
}

int8_t cissys_getUtilization()
{
    return 0;
}

int8_t cissys_getCSQ()
{
    return 0;
}

int8_t cissys_getECL()
{
    return 0;
}

int8_t cissys_getSNR()
{
    return 0;
}

int16_t cissys_getPCI()
{
    return 0;
}

int32_t cissys_getECGI()
{
    return 0;
}

int32_t cissys_getEARFCN()
{
    return 0;
}

int16_t cissys_getPCIFrist()
{
    return 0;
}

int16_t cissys_getPCISecond()
{
    return 0;
}

int8_t cissys_getRSRPFrist()
{
    return 0;
}

int8_t cissys_getRSRPSecond()
{
    return 0;
}

int8_t cissys_getPLMN()
{
    return 0;
}

int8_t cissys_getLatitude()
{
    return 0;
}

int8_t cissys_getLongitude()
{
    return 0;
}

int8_t cissys_getAltitude()
{
    return 0;
}
#endif    /* CIS_ENABLE_MONITER */
