#ifndef _DEV_RESET_INTERNAL_H_
#define _DEV_RESET_INTERNAL_H_

#include <stdio.h>
#include <string.h>
#include "infra_config.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_report.h"
#include "dev_reset_internal.h"
#include "dev_reset_api.h"
#include "mqtt_api.h"
#include "wrappers.h"

#ifdef INFRA_LOG
    #include "infra_log.h"
    #define devrst_err(...)               log_err("devrst", __VA_ARGS__)
    #define devrst_info(...)              log_info("devrst", __VA_ARGS__)
    #define devrst_debug(...)             log_debug("devrst", __VA_ARGS__)
#else
    #define devrst_info(...)              do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
    #define devrst_err(...)               do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
    #define devrst_debug(...)             do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#endif

#ifdef INFRA_MEM_STATS
    #include "infra_mem_stats.h"
    #define devrst_malloc(size)            LITE_malloc(size, MEM_MAGIC, "devrst")
    #define devrst_free(ptr)               LITE_free(ptr)
#else
    #define devrst_malloc(size)            HAL_Malloc(size)
    #define devrst_free(ptr)               {HAL_Free((void *)ptr);ptr = NULL;}
#endif

#endif

