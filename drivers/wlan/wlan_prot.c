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
 * @file        wlan_prot.c
 *
 * @brief       wlan_prot
 *
 * @details     wlan_prot
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_hw.h>
#include <os_task.h>
#include <wlan/wlan_dev.h>
#include <wlan/wlan_prot.h>
#include <os_errno.h>
#include <os_memory.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wlan.prot"
#include <drv_log.h>

#ifdef OS_WLAN_PROT_ENABLE

#if OS_WLAN_PROT_NAME_LEN < 4
#error "The name is too short"
#endif

struct os_wlan_prot_event_des
{
    os_wlan_prot_event_handler handler;
    struct os_wlan_prot *      prot;
};

static struct os_wlan_prot *_prot[OS_WLAN_PROT_MAX];

static struct os_wlan_prot_event_des prot_event_tab[OS_WLAN_PROT_EVT_MAX][OS_WLAN_PROT_MAX];

static void os_wlan_prot_event_handle(struct os_wlan_device *wlan,
                                      os_wlan_dev_event_t    event,
                                      struct os_wlan_buff   *buff,
                                      void                  *parameter)
{
    int                        i;
    struct os_wlan_prot *      wlan_prot;
    struct os_wlan_prot *      prot;
    os_wlan_prot_event_handler handler;
    os_wlan_prot_event_t       prot_event;

    LOG_EXT_D("F:%s L:%d event:%d", __FUNCTION__, __LINE__, event);

    wlan_prot = wlan->prot;
    handler   = OS_NULL;
    prot      = OS_NULL;
    switch (event)
    {
    case OS_WLAN_DEV_EVT_INIT_DONE:
    {
        LOG_EXT_D("L%d event: INIT_DONE", __LINE__);
        prot_event = OS_WLAN_PROT_EVT_INIT_DONE;
        break;
    }
    case OS_WLAN_DEV_EVT_CONNECT:
    {
        LOG_EXT_D("L%d event: CONNECT", __LINE__);
        prot_event = OS_WLAN_PROT_EVT_CONNECT;
        break;
    }
    case OS_WLAN_DEV_EVT_DISCONNECT:
    {
        LOG_EXT_D("L%d event: DISCONNECT", __LINE__);
        prot_event = OS_WLAN_PROT_EVT_DISCONNECT;
        break;
    }
    case OS_WLAN_DEV_EVT_AP_START:
    {
        LOG_EXT_D("L%d event: AP_START", __LINE__);
        prot_event = OS_WLAN_PROT_EVT_AP_START;
        break;
    }
    case OS_WLAN_DEV_EVT_AP_STOP:
    {
        LOG_EXT_D("L%d event: AP_STOP", __LINE__);
        prot_event = OS_WLAN_PROT_EVT_AP_STOP;
        break;
    }
    case OS_WLAN_DEV_EVT_AP_ASSOCIATED:
    {
        LOG_EXT_D("L%d event: AP_ASSOCIATED", __LINE__);
        prot_event = OS_WLAN_PROT_EVT_AP_ASSOCIATED;
        break;
    }
    case OS_WLAN_DEV_EVT_AP_DISASSOCIATED:
    {
        LOG_EXT_D("L%d event: AP_DISASSOCIATED", __LINE__);
        prot_event = OS_WLAN_PROT_EVT_AP_DISASSOCIATED;
        break;
    }
    default:
    {
        return;
    }
    }
    for (i = 0; i < OS_WLAN_PROT_MAX; i++)
    {
        if ((prot_event_tab[prot_event][i].handler != OS_NULL) &&
            (prot_event_tab[prot_event][i].prot->id == wlan_prot->id))
        {
            handler = prot_event_tab[prot_event][i].handler;
            prot    = prot_event_tab[prot_event][i].prot;
            break;
        }
    }

    if (handler != OS_NULL)
    {
        handler(prot, wlan, prot_event);
    }
}

static struct os_wlan_device *os_wlan_prot_find_by_name(const char *name)
{
    os_device_t *device;

    if (name == OS_NULL)
    {
        LOG_EXT_E("F:%s L:%d Parameter Wrongful", __FUNCTION__, __LINE__);
        return OS_NULL;
    }
    device = os_device_find(name);
    if (device == OS_NULL)
    {
        LOG_EXT_E("F:%s L:%d not find wlan dev!! name:%s", __FUNCTION__, __LINE__, name);
        return OS_NULL;
    }
    return (struct os_wlan_device *)device;
}

os_err_t os_wlan_prot_attach(const char *dev_name, const char *prot_name)
{
    struct os_wlan_device *wlan;

    wlan = os_wlan_prot_find_by_name(dev_name);
    if (wlan == OS_NULL)
    {
        return OS_ERROR;
    }
    return os_wlan_prot_attach_dev(wlan, prot_name);
}

os_err_t os_wlan_prot_detach(const char *name)
{
    struct os_wlan_device *wlan;

    wlan = os_wlan_prot_find_by_name(name);
    if (wlan == OS_NULL)
    {
        return OS_ERROR;
    }
    return os_wlan_prot_detach_dev(wlan);
}

os_err_t os_wlan_prot_attach_dev(struct os_wlan_device *wlan, const char *prot_name)
{
    int                       i       = 0;
    struct os_wlan_prot *     prot    = wlan->prot;
    os_wlan_dev_event_handler handler = os_wlan_prot_event_handle;

    if (wlan == OS_NULL)
    {
        LOG_EXT_E("F:%s L:%d wlan is null", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }

    if (prot != OS_NULL && (strcmp(prot->name, prot_name) == 0))
    {
        LOG_EXT_D("prot is register");
        return OS_EOK;
    }

    /* if prot not NULL */
    if (prot != OS_NULL)
        os_wlan_prot_detach_dev(wlan);

#ifdef OS_WLAN_PROT_LWIP_PBUF_FORCE
    if (strcmp(OS_WLAN_PROT_LWIP_NAME, prot_name) != 0)
    {
        return OS_ERROR;
    }
#endif
    /* find prot */
    for (i = 0; i < OS_WLAN_PROT_MAX; i++)
    {
        if ((_prot[i] != OS_NULL) && (strcmp(_prot[i]->name, prot_name) == 0))
        {
            /* attach prot */
            wlan->prot = _prot[i]->ops->dev_reg_callback(_prot[i], wlan);
            break;
        }
    }

    if (i >= OS_WLAN_PROT_MAX)
    {
        LOG_EXT_E("F:%s L:%d not find wlan protocol", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }

    os_wlan_dev_register_event_handler(wlan, OS_WLAN_DEV_EVT_CONNECT, handler, OS_NULL);
    os_wlan_dev_register_event_handler(wlan, OS_WLAN_DEV_EVT_DISCONNECT, handler, OS_NULL);
    os_wlan_dev_register_event_handler(wlan, OS_WLAN_DEV_EVT_AP_START, handler, OS_NULL);
    os_wlan_dev_register_event_handler(wlan, OS_WLAN_DEV_EVT_AP_STOP, handler, OS_NULL);
    os_wlan_dev_register_event_handler(wlan, OS_WLAN_DEV_EVT_AP_ASSOCIATED, handler, OS_NULL);
    os_wlan_dev_register_event_handler(wlan, OS_WLAN_DEV_EVT_AP_DISASSOCIATED, handler, OS_NULL);

    return OS_EOK;
}

os_err_t os_wlan_prot_detach_dev(struct os_wlan_device *wlan)
{
    struct os_wlan_prot *prot = wlan->prot;
    os_wlan_dev_event_t  event;

    if (prot == OS_NULL)
        return OS_EOK;

    for (event = OS_WLAN_DEV_EVT_INIT_DONE; event < OS_WLAN_DEV_EVT_MAX; event++)
    {
        os_wlan_dev_unregister_event_handler(wlan, event, os_wlan_prot_event_handle);
    }

    /* detach prot */
    prot->ops->dev_unreg_callback(prot, wlan);
    wlan->prot = OS_NULL;

    return OS_EOK;
}

os_err_t os_wlan_prot_regisetr(struct os_wlan_prot *prot)
{
    int               i;
    os_uint32_t       id;
    static os_uint8_t num;

    /* Parameter checking */
    if ((prot == OS_NULL) || (prot->ops->prot_recv == OS_NULL) || (prot->ops->dev_reg_callback == OS_NULL))
    {
        LOG_EXT_E("F:%s L:%d Parameter Wrongful", __FUNCTION__, __LINE__);
        return OS_EINVAL;
    }

    /* save prot */
    for (i = 0; i < OS_WLAN_PROT_MAX; i++)
    {
        if (_prot[i] == OS_NULL)
        {
            id       = (OS_LWAN_ID_PREFIX << 16) | num;
            prot->id = id;
            _prot[i] = prot;
            num++;
            break;
        }
        else if (strcmp(_prot[i]->name, prot->name) == 0)
        {
            break;
        }
    }

    /* is full */
    if (i >= OS_WLAN_PROT_MAX)
    {
        LOG_EXT_E("F:%s L:%d Space full", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }

    return OS_EOK;
}

os_err_t os_wlan_prot_event_register(struct os_wlan_prot *prot, os_wlan_prot_event_t event, os_wlan_prot_event_handler handler)
{
    int i;

    if ((prot == OS_NULL) || (handler == OS_NULL))
    {
        return OS_EINVAL;
    }

    for (i = 0; i < OS_WLAN_PROT_MAX; i++)
    {
        if (prot_event_tab[event][i].handler == OS_NULL)
        {
            prot_event_tab[event][i].handler = handler;
            prot_event_tab[event][i].prot    = prot;
            return OS_EOK;
        }
    }

    return OS_ERROR;
}

os_err_t os_wlan_prot_event_unregister(struct os_wlan_prot *prot, os_wlan_prot_event_t event)
{
    int i;

    if (prot == OS_NULL)
    {
        return OS_EINVAL;
    }

    for (i = 0; i < OS_WLAN_PROT_MAX; i++)
    {
        if ((prot_event_tab[event][i].handler != OS_NULL) && (prot_event_tab[event][i].prot == prot))
        {
            memset(&prot_event_tab[event][i], 0, sizeof(struct os_wlan_prot_event_des));
            return OS_EOK;
        }
    }

    return OS_ERROR;
}

os_err_t os_wlan_prot_transfer_dev(struct os_wlan_device *wlan, void *buff, int len)
{
    if (wlan->ops->wlan_send != OS_NULL)
    {
        return wlan->ops->wlan_send(wlan, buff, len);
    }
    return OS_ERROR;
}

os_err_t os_wlan_dev_transfer_prot(struct os_wlan_device *wlan, void *buff, int len)
{
    struct os_wlan_prot *prot = wlan->prot;

    if (prot != OS_NULL)
    {
        return prot->ops->prot_recv(wlan, buff, len);
    }
    return OS_ERROR;
}

extern int os_wlan_prot_ready_event(struct os_wlan_device *wlan, struct os_wlan_buff *buff);

int os_wlan_prot_ready(struct os_wlan_device *wlan, struct os_wlan_buff *buff)
{
    return os_wlan_prot_ready_event(wlan, buff);
}

void os_wlan_prot_dump(void)
{
    int i;

    os_kprintf("  name       id \n");
    os_kprintf("--------  --------\n");
    for (i = 0; i < OS_WLAN_PROT_MAX; i++)
    {
        if (_prot[i] != OS_NULL)
        {
            os_kprintf("%-8.8s  ", _prot[i]->name);
            os_kprintf("%08x\n", _prot[i]->id);
        }
    }
}
#endif
