/*******************************************************************************
 *
 * Copyright (c) 2017 China Mobile and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Bai Jie & Long Rong, China Mobile - initial API and implementation
 *
 *******************************************************************************/

/*
 Copyright (c) 2017 Chinamobile

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.


 Bai Jie <baijie@chinamobile.com>
 Long Rong <longrong@chinamobile.com>

*/
/************************************************************************/
/* nb-iot middle software of china mobile api                           */
/************************************************************************/
#include "cis_config.h"
#include "cis_def.h"
#include "cis_log.h"
#include "cis_if_sys.h"
#include "cis_internals.h"



#define NBCFG_USE_BIGENDIAN                 1

#define NBCFG_SUPPORT_VERSION               0x01
static const uint8_t prvVersionUnsupportList[] = {0x00,};

#define NBCFG_HeaderSize                    (0x03)
//#define NBCFG_MAKE_U16(p)                   (prvMakeU16(p))
//#define NBCFG_MAKE_U32(p)                   (prvMakeU32(p))


#define NBCFG_GetVersion(data)              ((data >> 4) & 0x0F)
#define NBCFG_GetCC(data)                   (data & 0x0F)

#define NBCFG_GetConfigId(data)             (cis_cfgid_t)(data & 0x0F)
#define NBCFG_GetConfigFlag(data)           (data & 0x80)
//#define NBCFG_GetConfigSize(data)           (NBCFG_MAKE_U16(data))

#define NBCFG_Getconfig_LogEnabled(data)    ((data >> 7) & 0x01)
#define NBCFG_Getconfig_LogExtoutput(data)  ((data >> 6) & 0x01)
#define NBCFG_Getconfig_LogType(data)       ((data >> 4) & 0x03)
#define NBCFG_Getconfig_LogOutputlevel(data)     (data  & 0x0F)
#define NBCFG_GetConfig_Istcount(data)      ((data >> 4) & 0x0F)

#define NBCFG_Getconfig_BSEnabled(data)     ((data >> 7) & 0x01)
#define NBCFG_Getconfig_DTLSEnabled(data)     ((data >> 6) & 0x01)

#define NBCFG_GetByteHi(p)                  (((uint8_t)(*p) >> 4) & 0x0F)
#define NBCFG_GetByteLow(p)                 ((uint8_t)(*p) & 0x0F)


//#define NBCFG_DEFAULT_HOST                  ("183.230.40.40") //("183.230.40.171")//
//#define NBCFG_DEFAULT_BOOTSTRAP                  ("183.230.40.39")

#define NBCFG_DEFAULT_MTU                   (1024)




#pragma pack(1)


struct st_cfg_header
{
    uint8_t version;
    uint8_t cfgcount;
    uint16_t bytes;
};

typedef struct st_cfg_header  ciscfg_header_t;

struct st_cfg_context
{
    ciscfg_header_t cfgHeader;
    cis_cfg_init_t cfgInit;
    cis_cfg_net_t cfgNet;
    cis_cfg_sys_t cfgSys;
};
typedef struct st_cfg_context ciscfg_context_t;

//static uint16_t prvMakeU16(uint8_t* p);
//static uint32_t prvMakeU32(uint8_t* p);
#if 0
static bool prvCheckSum(uint8_t* config,uint16_t size,uint32_t checksum);
static bool prvInitParser(ciscfg_context_t* context,uint8_t* config,uint16_t size);
static bool prvSysParser(ciscfg_context_t* context,uint8_t* config,uint16_t size);
static bool prvNetParser(ciscfg_context_t* context,uint8_t* config,uint16_t size);
#endif
static void prvDefaultConfig(ciscfg_context_t* context);
static void prvUserConfig(ciscfg_context_t* context, cis_user_cfg_t* cis_user_cfg);


cis_ret_t cis_config_init(void** config_context, cis_user_cfg_t* cis_user_cfg)
{
    ciscfg_context_t* context = NULL;

    ciscfg_header_t* cfgHeader = NULL;
    
    if((*config_context) == NULL)
    {
        context = (ciscfg_context_t*)cis_malloc(sizeof(ciscfg_context_t));
    }else
    {
        context = (ciscfg_context_t*)(*config_context);
    }

    if(context == NULL)
    {
        return CIS_RET_ERROR;
    }

    *config_context = context;

    if(cis_user_cfg == NULL)
    {
        prvDefaultConfig(context);
        return CIS_RET_NO_ERROR;
    }else
    {
        prvUserConfig(context, cis_user_cfg);
    }
    
    cfgHeader = &context->cfgHeader;
    cfgHeader->version= 0x01;
    cfgHeader->cfgcount = 3;
    cfgHeader->bytes = 0;
    

    LOGD("Config Header Info:");
    LOGD("Version:[0x%02x]",cfgHeader->version);
    LOGD("Version Support:[0x%02x]",NBCFG_SUPPORT_VERSION);
    LOGD("Count:%d",cfgHeader->cfgcount);
    LOGD("Size:%d bytes",cfgHeader->bytes);
    LOGD("----------------");

    if(cfgHeader->version != NBCFG_SUPPORT_VERSION)
    {
        int i=0;
        for(i=0;i<sizeof(prvVersionUnsupportList)/sizeof(uint8_t);i++)
        {
            if(prvVersionUnsupportList[i] == cfgHeader->version)
            {
                LOGE("ERROR:config init failed.don't support version.");
                cis_config_destory(config_context);
                return CIS_RET_ERROR;
            }
        }
    }
    
    LOGI(">>Config Init");
    LOGI("----------------");
    LOGI(">>Config Sys");
    LOGD("Log Enabled:%u",context->cfgSys.log_enabled);
    LOGD("Log Extend Output Type:%u",context->cfgSys.log_ext_output);
    LOGD("Log Output Type:%u",context->cfgSys.log_output_type);
    LOGD("Log Output Level:%u",context->cfgSys.log_output_level);
    LOGD("Log Buffer Size:%u",context->cfgSys.log_buffer_size);
    LOGD("Userdata Len:%u",context->cfgSys.user_data.len);
    LOGI("----------------");
    LOGI(">>Config Net");
    LOGI("Net Mtu:%u",context->cfgNet.mtu);
    LOGI("Net Linktype:%u",context->cfgNet.linktype);
    LOGI("Net Bandtype:%u",context->cfgNet.bandtype);
    if(context->cfgNet.username.len > 0){
        LOGI("Net Username:%s",context->cfgNet.username.data);
    }
    if(context->cfgNet.password.len > 0){
        LOGI("Net Password:%s",context->cfgNet.password.data);
    }
    if(context->cfgNet.apn.len > 0){
        LOGI("Net Apn:%s",context->cfgNet.apn.data);
    }
    if(context->cfgNet.host.len > 0){
        LOGI("Net Host:%s",context->cfgNet.host.data);
    }
    LOGD("Net Userdata data:%s",context->cfgNet.user_data.data);
    LOGD("Net Userdata Len:%u",context->cfgNet.user_data.len);
    LOGI("----------------");

    LOGI("The config init success.");
    return CIS_RET_OK;
}

cis_ret_t cis_config_get(void* config_context,cis_cfgid_t config_id,cis_cfgret_t* ret)
{
    ciscfg_context_t* context = (ciscfg_context_t*)config_context;
    switch(config_id){
    case cis_cfgid_init:
        {
            ret->data.cfg_init = &context->cfgInit;
        break;
        }
    case cis_cfgid_net:
        {
            ret->data.cfg_net = &context->cfgNet;
            break;
        }
    case cis_cfgid_sys:
        {
            ret->data.cfg_sys = &context->cfgSys;
            break;
        }
    default:
        return CIS_RET_ERROR;
    }
    
    return CIS_RET_OK;
}


void cis_config_destory(void** config_context)
{     
    cis_free(*config_context);
    *config_context = NULL;
}


//////////////////////////////////////////////////////////////////////////
//private function

#if 0
uint16_t prvMakeU16(uint8_t* p)
{
    uint8_t data[2]={0};
#if NBCFG_USE_BIGENDIAN
    if(utils_checkBigendian()){
        data[0] = *((uint8_t*)p);
        data[1] = *((uint8_t*)p + 1);
    }else
    {
        data[1] = *((uint8_t*)p);
        data[0] = *((uint8_t*)p + 1);
    }
#else
    if(utils_checkBigendian()){
        data[1] = *((uint8_t*)p + 0);
        data[0] = *((uint8_t*)p + 1);
    }else{
        data[0] = *((uint8_t*)p + 0);
        data[1] = *((uint8_t*)p + 1);
    }
#endif//NBCFG_USE_BIGENDIAN


    return *(uint16_t*)data;
}

uint32_t prvMakeU32(uint8_t* p)
{
    uint8_t data[4]={0};
#if NBCFG_USE_BIGENDIAN
    if(utils_checkBigendian()){
        data[0] = *((uint8_t*)p + 0);
        data[1] = *((uint8_t*)p + 1);
        data[2] = *((uint8_t*)p + 2);
        data[3] = *((uint8_t*)p + 3);

    }else{
        data[3] = *((uint8_t*)p + 0);
        data[2] = *((uint8_t*)p + 1);
        data[1] = *((uint8_t*)p + 2);
        data[0] = *((uint8_t*)p + 3);
    }
#else
    if(utils_checkBigendian()){
        data[3] = *((uint8_t*)p + 0);
        data[2] = *((uint8_t*)p + 1);
        data[1] = *((uint8_t*)p + 2);
        data[0] = *((uint8_t*)p + 3);
    }else{
        data[0] = *((uint8_t*)p + 0);
        data[1] = *((uint8_t*)p + 1);
        data[2] = *((uint8_t*)p + 2);
        data[3] = *((uint8_t*)p + 3);
    }
#endif//NBCFG_USE_BIGENDIAN
    return *(uint32_t*)data;
}


bool prvCheckSum(uint8_t* config,uint16_t size,uint32_t checksum)
{
    return TRUE;
}


bool prvInitParser(ciscfg_context_t* context,uint8_t* config,uint16_t size)
{
    LOGI(">>Config Init");
    LOGI("----------------");
    return TRUE;
}

bool prvSysParser(ciscfg_context_t* context,uint8_t* config,uint16_t size)
{
    cis_cfg_sys_t* cfgSys = &context->cfgSys;
    uint8_t* cfg_ptr = config;
    cfgSys->log_enabled = NBCFG_Getconfig_LogEnabled(*cfg_ptr);
    cfgSys->log_ext_output = NBCFG_Getconfig_LogExtoutput(*cfg_ptr);
    cfgSys->log_output_type = NBCFG_Getconfig_LogType(*cfg_ptr);
    cfgSys->log_output_level = NBCFG_Getconfig_LogOutputlevel(*cfg_ptr);
    cfgSys->log_buffer_size = NBCFG_MAKE_U16(cfg_ptr + 1);
    cfg_ptr += 3;
    cfg_ptr += sizeof(uint8_t);//reserve

    cfgSys->user_data.len = NBCFG_MAKE_U16(cfg_ptr);
    cfg_ptr += 2;
    cfgSys->user_data.data = NULL;
    if(cfgSys->user_data.len > 0)
    {
        cfgSys->user_data.data = cfg_ptr;
    }
    cfg_ptr += cfgSys->user_data.len;
    
    LOGI(">>Config Sys");
    LOGD("Log Enabled:%u",cfgSys->log_enabled);
    LOGD("Log Extend Output Type:%u",cfgSys->log_ext_output);
    LOGD("Log Output Type:%u",cfgSys->log_output_type);
    LOGD("Log Output Level:%u",cfgSys->log_output_level);
    LOGD("Log Buffer Size:%u",cfgSys->log_buffer_size);
    LOGD("Userdata Len:%u",cfgSys->user_data.len);
    LOGI("----------------");

    return TRUE;
}

bool prvNetParser(ciscfg_context_t* context,uint8_t* config,uint16_t size)
{
    uint8_t* cfg_ptr = config;
    cis_cfg_net_t* cfgNet =& context->cfgNet;
    cfgNet->mtu = NBCFG_MAKE_U16(cfg_ptr);
    cfg_ptr += sizeof(uint16_t);
    cfgNet->linktype = NBCFG_GetByteHi(cfg_ptr);
    cfgNet->bandtype = NBCFG_GetByteLow(cfg_ptr);
    cfg_ptr += sizeof(uint8_t);
    cfgNet->bs_enabled = NBCFG_Getconfig_BSEnabled(*cfg_ptr);
	cfgNet->dtls_enabled = NBCFG_Getconfig_DTLSEnabled(*cfg_ptr);
    cfg_ptr += sizeof(uint8_t);//reserve

    cfgNet->apn.len = NBCFG_MAKE_U16(cfg_ptr);
    cfg_ptr += sizeof(uint16_t);
    cfgNet->apn.data = (uint8_t*)cfg_ptr;
    cfg_ptr += cfgNet->apn.len;

    cfgNet->username.len = NBCFG_MAKE_U16(cfg_ptr);
    cfg_ptr += sizeof(uint16_t);
    cfgNet->username.data = (uint8_t*)cfg_ptr;
    cfg_ptr += cfgNet->username.len;

    cfgNet->password.len = NBCFG_MAKE_U16(cfg_ptr);
    cfg_ptr += sizeof(uint16_t);
    cfgNet->password.data = (uint8_t*)cfg_ptr;
    cfg_ptr += cfgNet->password.len;


    cfgNet->host.len = NBCFG_MAKE_U16(cfg_ptr);
    cfg_ptr += sizeof(uint16_t);
    cfgNet->host.data = (uint8_t*)cfg_ptr;
    cfg_ptr += cfgNet->host.len;


    cfgNet->user_data.len = NBCFG_MAKE_U16(cfg_ptr);
    cfg_ptr += sizeof(uint16_t);
    cfgNet->user_data.data = (uint8_t*)cfg_ptr;
    cfg_ptr += cfgNet->user_data.len;

    
    LOGI(">>Config Net");
    LOGD("Net Mtu:%u",cfgNet->mtu);
    LOGD("Net Linktype:%u",cfgNet->linktype);
    LOGD("Net Bandtype:%u",cfgNet->bandtype);
    if(cfgNet->username.len > 0){
        LOGD("Net Username:%s",cfgNet->username.data);
    }
    if(cfgNet->password.len > 0){
        LOGD("Net Password:%s",cfgNet->password.data);
    }
    if(cfgNet->apn.len > 0){
        LOGD("Net Apn:%s",cfgNet->apn.data);
    }
    if(cfgNet->host.len > 0){
        LOGD("Net Host:%s",cfgNet->host.data);
    }
    
    LOGD("Userdata Len:%u",cfgNet->user_data.len);
    LOGI("----------------");

    return TRUE;
}
#endif

void prvDefaultConfig(ciscfg_context_t* context)
{
    cis_cfg_net_t *cfgNet = &context->cfgNet;
    cis_cfg_sys_t *cfgSys = &context->cfgSys;

#if CIS_ENABLE_LOG
    cfgSys->log_enabled = true;
#else
    cfgSys->log_enabled = false;
#endif//CIS_ENABLE_LOG
    cfgSys->log_ext_output = 0;
    cfgSys->log_output_type = 0;
    cfgSys->log_output_level = 3;
    cfgSys->log_buffer_size = 200;
    cfgSys->user_data.len = 0;
    cfgSys->user_data.data = NULL;


    cfgNet->mtu = NBCFG_DEFAULT_MTU;
    cfgNet->linktype = 0;
    cfgNet->bandtype = 0;
    cfgNet->apn.len = 0;
    cfgNet->apn.data = NULL;
    cfgNet->username.len = 0;
    cfgNet->username.data = NULL;
    cfgNet->password.len = 0;
    cfgNet->password.data = NULL;
    cfgNet->bs_enabled = true;
	cfgNet->dtls_enabled = false;
    cfgNet->host.len = (uint16_t)utils_strlen(NBCFG_DEFAULT_BOOTSTRAP);
    cfgNet->host.data = (uint8_t*)NBCFG_DEFAULT_BOOTSTRAP;
    cfgNet->user_data.len = 0;
    cfgNet->user_data.data = NULL;


    LOGI(">>Default Config Sys");
    LOGD("Log Enabled:%u",cfgSys->log_enabled);
    LOGD("Log Extend Output Type:%u",cfgSys->log_ext_output);
    LOGD("Log Output Type:%u",cfgSys->log_output_type);
    LOGD("Log Output Level:%u",cfgSys->log_output_level);
    LOGD("Log Buffer Size:%u",cfgSys->log_buffer_size);
    LOGD("Userdata Len:%u",cfgSys->user_data.len);

    LOGI(">>Default Config Net");
    LOGI("Net Mtu:%u",cfgNet->mtu);
    LOGI("Net Linktype:%u",cfgNet->linktype);
    LOGI("Net Bandtype:%u",cfgNet->bandtype);
    if(cfgNet->username.len > 0){
        LOGI("Net Username:%s",cfgNet->username.data);
    }
    if(cfgNet->password.len > 0){
        LOGI("Net Password:%s",cfgNet->password.data);
    }
    if(cfgNet->apn.len > 0){
        LOGI("Net Apn:%s",cfgNet->apn.data);
    }
    if(cfgNet->host.len > 0){
        LOGI("Net Host:%s",cfgNet->host.data);
    }
    
    LOGD("Net Userdata data:%s",cfgNet->user_data.data);
    LOGD("Net Userdata Len:%u",cfgNet->user_data.len);
}


static void prvUserConfig(ciscfg_context_t* context, cis_user_cfg_t* cis_user_cfg)
{
    cis_cfg_net_t *cfgNet = &context->cfgNet;
    cis_cfg_sys_t *cfgSys = &context->cfgSys;
    char *host = NULL;
#if CIS_ENABLE_AUTH
    char *net_userdata = NULL;
#endif
    
#if CIS_ENABLE_LOG
    cfgSys->log_enabled = true;
    cfgSys->log_output_level = CIS_LOG_LEVEL;
    cfgSys->log_buffer_size = CIS_LOG_BUFFER_SIZE;
#else
    cfgSys->log_enabled = false;
    cfgSys->log_output_level = 3;
    cfgSys->log_buffer_size = 200;
#endif
    cfgSys->log_ext_output = 0;
    cfgSys->log_output_type = 0;
    cfgSys->user_data.len = 0;
    cfgSys->user_data.data = NULL;


    cfgNet->mtu = NBCFG_DEFAULT_MTU;
    cfgNet->linktype = 0;
    cfgNet->bandtype = 0;
    cfgNet->apn.len = 0;
    cfgNet->apn.data = NULL;
    cfgNet->username.len = 0;
    cfgNet->username.data = NULL;
    cfgNet->password.len = 0;
    cfgNet->password.data = NULL;
    cfgNet->bs_enabled = cis_user_cfg->bs_enabled;
	cfgNet->dtls_enabled = cis_user_cfg->dtls_enabled;
    host = cis_malloc(21);
    if (host != NULL){
        cis_memset(host, 0, 21);
        snprintf(host, 21, "%s:%s", cis_user_cfg->ip, cis_user_cfg->port);
        cfgNet->host.len = (uint16_t)utils_strlen(host);
        cfgNet->host.data = (uint8_t*)host;
    }else{
        cfgNet->host.len = 0;
        cfgNet->host.data = NULL;
    }
#if CIS_ENABLE_AUTH
    net_userdata = cis_malloc(strlen((char *)cis_user_cfg->auth_code) + strlen((char *)cis_user_cfg->psk) + 16);
    if (net_userdata != NULL ){
        cis_memset(net_userdata, 0 , strlen((char *)cis_user_cfg->auth_code) + strlen((char *)cis_user_cfg->psk) + 16);
        sprintf(net_userdata, "AuthCode:%s;PSK:%s;", cis_user_cfg->auth_code, cis_user_cfg->psk);
        cfgNet->user_data.len = strlen((char *)cis_user_cfg->auth_code) + strlen((char *)cis_user_cfg->psk) + 16;
        cfgNet->user_data.data = (uint8_t*)net_userdata;
    }else{
        cfgNet->user_data.len = 0;
        cfgNet->user_data.data = NULL;
    }
#else
    cfgNet->user_data.len = 0;
    cfgNet->user_data.data = NULL;
#endif
}
