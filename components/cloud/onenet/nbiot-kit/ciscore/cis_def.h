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
#ifndef _CIS_DEFINITION_HEADER_
#define _CIS_DEFINITION_HEADER_

//some pre-defined macro
//#define  LWM2M_LITTLE_ENDIAN
#define  LWM2M_BIG_ENDIAN

#define  CIS_EMBED 

//////////////////////////////////////
#if defined( CIS_WIN32 )
#    if defined( CIS_EXPORT )
#        define CIS_API __declspec(dllexport)
#    endif
#    if defined( CIS_IMPORT )
#        define CIS_API __declspec(dllimport)
#    endif  // defined(NB_EXPORTS)
#elif defined( CIS_LINUX )
// Do not upstream, this will be added later upstream,
// after this file is renamed to base_export.h. Not
// cherry-picking that since the #define's also change
#define CIS_API __attribute__((visibility("default")))

#elif defined( CIS_EMBED )
#include <stdbool.h>
#include <time.h>
#else
#error "Please select the target platform used in your application."
#endif


#if !defined( CIS_API )
#define     CIS_API
#endif

#include <stdint.h>
#include <stdlib.h>
#include <oneos_config.h>

#ifndef TRUE
#define  TRUE (1)
#endif

#ifndef FALSE
#define  FALSE (0)
#endif

typedef int32_t      cis_evt_t;
typedef int32_t      cis_ret_t;
typedef uint8_t      cis_coapret_t;
typedef uint16_t     cis_oid_t;
typedef uint16_t     cis_iid_t;
typedef uint16_t     cis_rid_t;
typedef uint16_t     cis_coapid_t;//coap message id
typedef uint32_t     cis_mid_t;   //callback message id

typedef cis_iid_t    cis_instcount_t;
typedef cis_rid_t    cis_attrcount_t;
typedef cis_rid_t    cis_actcount_t;
typedef cis_rid_t    cis_rescount_t;
typedef int32_t      cis_time_t;  //normally in second unit

/*
 *
 */
#ifdef ONENET_NBIOT_USING_AUTH_CODE
#define     CIS_ENABLE_AUTH                       1
#define     CIS_AUTH_CODE                         NBIOT_DEVICE_AUTH_CODE
#else
#define     CIS_AUTH_CODE                         ""
#endif

#ifdef ONENET_NBIOT_ENABLE_LOG
#define     CIS_ENABLE_LOG                        1
#define     CIS_LOG_LEVEL                         ONENET_NBIOT_LOG_LEVEL
#define     CIS_LOG_BUFFER_SIZE                   ONENET_NBIOT_LOG_BUFFER_SIZE
#endif

#ifdef ONENET_NBIOT_ENABLE_MEMTRACE
#define     CIS_ENABLE_MEMORYTRACE                1        
#endif 

#ifdef ONENET_NBIOT_USING_DTLS
#define     CIS_ENABLE_DTLS                       1
#define     CIS_ENABLE_PSK                        1
#define     CIS_PSK_CODE                          NBIOT_DEVICE_PSK_CODE   
#else
#define     CIS_PSK_CODE                          ""
#endif

//#define     CIS_ENABLE_DM                       1            
//#define     CIS_ENABLE_UPDATE                   1
//#define     CIS_ENABLE_UPDATE_MCU               1

//#define     CIS_ENABLE_AUTH                     1
//#define     CIS_ENABLE_MONITER                  1
//#define     CIS_ENABLE_CMIOT_OTA                1
//#define     CIS_ENABLE_DTLS                     1
//#define     CIS_ENABLE_PSK                      1
//#define     CIS_LITTLE_ENDIAN                   1
//#define     CIS_ENABLE_LOG                      1
//#define     CIS_DTLS_LOG                        1

//#define     CIS_ENABLE_MEMORYTRACE              1

#ifndef     CIS_VERSION_MAJOR
#define     CIS_VERSION_MAJOR                     2
#endif
#ifndef     CIS_VERSION_MINOR
#define     CIS_VERSION_MINOR                     3
#endif
#ifndef     CIS_VERSION_MICRO
#define     CIS_VERSION_MICRO                     0
#endif

#ifndef     DM_VERSION_MAJOR
#define     DM_VERSION_MAJOR                      2
#endif
#ifndef     DM_VERSION_MINOR
#define     DM_VERSION_MINOR                      0
#endif
#ifndef     DM_VERSION_MICRO
#define     DM_VERSION_MICRO                      3
#endif


#ifndef     CIS_ENABLE_UPDATE_MCU
#define     CIS_ENABLE_UPDATE_MCU                 0
#else
#undef      CIS_ENABLE_UPDATE_MCU
#define     CIS_ENABLE_UPDATE_MCU                 1
#endif


#ifndef     CIS_ENABLE_UPDATE
#define     CIS_ENABLE_UPDATE                     0           
#else
#undef      CIS_ENABLE_UPDATE
#define     CIS_ENABLE_UPDATE                     1
#endif

#ifndef     CIS_ENABLE_MONITER
#define     CIS_ENABLE_MONITER                    0
#else
#undef      CIS_ENABLE_MONITER
#define     CIS_ENABLE_MONITER                    1
#endif

#ifndef     CIS_ENABLE_DM
#define     CIS_ENABLE_DM                         0
#else
#undef      CIS_ENABLE_DM
#define     CIS_ENABLE_DM                         1                   
#endif

#ifndef     OTA_VERSION_MAJOR
#define     OTA_VERSION_MAJOR                     0
#endif
#ifndef     OTA_VERSION_MINOR
#define     OTA_VERSION_MINOR                     0
#endif
#ifndef     OTA_VERSION_MICRO
#define     OTA_VERSION_MICRO                     3
#endif



#ifndef		CIS_ENABLE_CMIOT_OTA
#define		CIS_ENABLE_CMIOT_OTA				  0
#else
#undef		CIS_ENABLE_CMIOT_OTA
#define		CIS_ENABLE_CMIOT_OTA			      1
#endif

#ifndef     CIS_ENABLE_AUTH
#define     CIS_ENABLE_AUTH                       0
#else
#undef      CIS_ENABLE_AUTH
#define     CIS_ENABLE_AUTH                       1
#endif


//default
#define     CIS_ENABLE_BLOCK

#ifndef     CIS_ENABLE_BLOCK
#define     CIS_ENABLE_BLOCK                      0
#else
#undef      CIS_ENABLE_BLOCK
#define     CIS_ENABLE_BLOCK                      1
#endif

#ifndef     CIS_ENABLE_MEMORYTRACE
#define     CIS_ENABLE_MEMORYTRACE                0
#else
#undef      CIS_ENABLE_MEMORYTRACE
#define     CIS_ENABLE_MEMORYTRACE                1
#endif

#ifndef     CIS_ENABLE_LOG
#define     CIS_ENABLE_LOG                        0
#else
#undef      CIS_ENABLE_LOG
#define     CIS_ENABLE_LOG                        1
#endif



#ifndef     CIS_ENABLE_DTLS
#define     CIS_ENABLE_DTLS                       0
#else
#undef      CIS_ENABLE_DTLS
#define     CIS_ENABLE_DTLS                       1
#endif

#ifndef     CIS_DTLS_LOG
#define     CIS_DTLS_LOG                          0
#else
#undef      CIS_DTLS_LOG
#define     CIS_DTLS_LOG                          1
#endif

#ifndef     CIS_LITTLE_ENDIAN
#define     CIS_LITTLE_ENDIAN                     0     
#else
#undef      CIS_LITTLE_ENDIAN
#define     CIS_LITTLE_ENDIAN                     1
#endif


#ifndef     CIS_ENABLE_PSK
#define     CIS_ENABLE_PSK                        0
#else
#undef      CIS_ENABLE_PSK
#define     CIS_ENABLE_PSK                        1
#endif


#define     CIS_CONFIG_LOCK_INFINITY              0xFFFFFFFF
#define     CIS_CONFIG_MEMORYTRACE_TIMEOUT        (5)
#define     CIS_CONFIG_CALLBACK_TIMEOUT           (300)
#define     CIS_CONFIG_BOOTSTRAP_TIMEOUT          (60)

#define     CIS_CONFIG_CONNECT_RETRY_TIME         (2)
#define     CIS_CONFIG_REG_INTERVAL_TIME          (5)

#define     CIS_COFNIG_REG_QUERY_SIZE             (256)
#define     CIS_COFNIG_REG_PAYLOAD_SIZE           (512)

// the maximum payload transfered by block1 we accumulate per server
#define     CIS_CONFIG_BLOCK1_SIZE_MAX            (2048)


#if CIS_ENABLE_UPDATE_MCU

#define  sota_event_unknow    0
#define	 sota_erase_success   1        //for erase, handle /5/0/26501
#define  sota_erase_fail      2  
#define	 sota_write_success   3        //for write
#define	 sota_write_fail      4
#define	 sota_validate_success 5       //for validate
#define	 sota_validate_fail    6                
#define	 sota_update_success   7       //for update
#define	 sota_update_fail      8

#endif


#endif //_CIS_DEFINITION_HEADER_

