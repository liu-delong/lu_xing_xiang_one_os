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
 * \@file        cis_if_net.c
 *
 * \@brief       network port file for cis
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <cis_sock.h>
#include <cis_if_net.h>
#include <cis_list.h>
#include <cis_log.h>
#include <cis_if_sys.h>
#include <cis_internals.h>
#include <libc_errno.h>
#include "os_task.h"
#include "os_sem.h"
#ifdef __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__
#include "modem_switch.h"
#endif
#define MAX_PACKET_SIZE (1024)

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN IP4ADDR_STRLEN_MAX
#endif

static void *g_lockPacketlist;
static int   prvCreateSocket(uint16_t localPort, int ai_family);

void callbackRecvThread(void *lpParam);
void callbackDMThread(void *lpParam);

bool cisnet_attached_state(void *ctx)
{
    return ((struct st_cis_context *)(ctx))->netAttached;
}

#if defined(__NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__) && defined(MTK_SOCKET_AGENT_SUPPORT)
static modem_switch_network_type_enum_t g_onenet_nw_type = MODEM_SWITCH_NETWORK_TYPE_NO_SERVICE;
static bool                             g_onenet_nw_switch;
static modem_switch_network_type_enum_t g_onenet_dm_type = MODEM_SWITCH_NETWORK_TYPE_NO_SERVICE;
static bool                             g_onenet_dm_switch;

void cisnet_event_callback(modem_switch_event_t *evt)
{
    if (evt && evt->evt_id == MODEM_SWITCH_EVENT_ID_NETWORK_TYPE_UPDATE_IND)
    {
        modem_switch_event_network_type_update_ind_t *ind = (modem_switch_event_network_type_update_ind_t *)evt;
        LOGI("modem switch callback nw_type: %d, g_onenet_nw_type: %d", ind->nw_type, g_onenet_nw_type);
        if ((ind->nw_type == MODEM_SWITCH_NETWORK_TYPE_NBIOT || ind->nw_type == MODEM_SWITCH_NETWORK_TYPE_2G) &&
            ind->nw_type != g_onenet_nw_type)
        {
            /* switch current onenet session */
            g_onenet_nw_switch = true;
        }
    }
}

void cisnet_dm_event_callback(modem_switch_event_t *evt)
{
    if (evt && evt->evt_id == MODEM_SWITCH_EVENT_ID_NETWORK_TYPE_UPDATE_IND)
    {
        modem_switch_event_network_type_update_ind_t *ind = (modem_switch_event_network_type_update_ind_t *)evt;
        LOGI("modem switch callback nw_type: %d, g_onenet_nw_type: %d", ind->nw_type, g_onenet_dm_type);
        if ((ind->nw_type == MODEM_SWITCH_NETWORK_TYPE_NBIOT || ind->nw_type == MODEM_SWITCH_NETWORK_TYPE_2G) &&
            ind->nw_type != g_onenet_dm_type)
        {
            /* switch current dm session */
            g_onenet_dm_switch = true;
        }
    }
}
#endif /* __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__ */

cis_ret_t cisnet_init(void *context, const cisnet_config_t *config, cisnet_callback_t cb)
{
    net_Init();
    LOGI("fall in cisnet_init");
    cis_memcpy(&((struct st_cis_context *)context)->netConfig, config, sizeof(cisnet_config_t));
    ((struct st_cis_context *)context)->netCallback.onEvent = cb.onEvent;
    cissys_sleepms(1000);
    ((struct st_cis_context *)context)->netAttached = 1;
    if (g_lockPacketlist == NULL)
        cissys_lockcreate(&g_lockPacketlist);
#if defined(__NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__) && defined(MTK_SOCKET_AGENT_SUPPORT)
    modem_switch_result_code_enum_t result_code;
    if (((struct st_cis_context *)context)->isDM)
    {
        result_code = modem_switch_register_event_callback(cisnet_dm_event_callback, context);
    }
    else
    {
        result_code = modem_switch_register_event_callback(cisnet_event_callback, context);
    }
    LOGI("modem switch register result_code: %d", result_code);
#endif /* __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__ */
    return CIS_RET_OK;
}

cis_ret_t cisnet_create(cisnet_t *netctx, const char *host, void *context)
{
    st_context_t *ctx = (struct st_cis_context *)context;

    if (ctx->netAttached != true)
        return CIS_RET_ERROR;

    (*netctx)               = (cisnet_t)cis_malloc(sizeof(struct st_cisnet_context));
    (*netctx)->sock         = 0;
    (*netctx)->port         = atoi((const char *)(ctx->serverPort));
    (*netctx)->state        = 0;
    (*netctx)->quit         = 0;
    (*netctx)->g_packetlist = NULL;
    (*netctx)->context      = context;
    strncpy((*netctx)->host, host, sizeof((*netctx)->host));

    return CIS_RET_OK;
}

void cisnet_destroy(cisnet_t netctx)
{
    LOGD("cisnet_destroy");
    netctx->quit = 1;
    if (((struct st_cis_context *)(netctx->context))->recv_taskhandle != NULL)
    {
        cissys_lock(((struct st_cis_context *)(netctx->context))->lockSocket, CIS_CONFIG_LOCK_INFINITY);
        cissys_lock(((struct st_cis_context *)(netctx->context))->lockSocket, CIS_CONFIG_LOCK_INFINITY);
        cissys_unlock(((struct st_cis_context *)(netctx->context))->lockSocket);
    }
    net_Close(netctx->sock);
    cissys_lock(g_lockPacketlist, CIS_CONFIG_LOCK_INFINITY);
    while (netctx->g_packetlist != NULL)
    {
        struct st_net_packet* delNode;
        delNode              = netctx->g_packetlist;
        netctx->g_packetlist = netctx->g_packetlist->next;
        cis_free(delNode->buffer);
        cis_free(delNode);
    }
    cissys_unlock(g_lockPacketlist);
    cis_free(netctx);
}

cis_ret_t cisnet_connect(cisnet_t netctx)
{
    int        sock            = -1;
    os_task_t *recv_taskhandle = NULL;
#if defined(__NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__) && defined(MTK_SOCKET_AGENT_SUPPORT)
    if (((struct st_cis_context *)(netctx->context))->isDM)
    {
        modem_switch_network_type_enum_t network_type = modem_switch_get_available_service();
        LOGD("mode switch network type: %d", network_type);
        if (network_type == MODEM_SWITCH_NETWORK_TYPE_NBIOT)
        {
            sock             = prvCreateSocket(0, AF_INET);
            g_onenet_dm_type = MODEM_SWITCH_NETWORK_TYPE_NBIOT;
        }
        else if (network_type == MODEM_SWITCH_NETWORK_TYPE_2G)
        {
            sock             = prvCreateSocket(0, AF_BRIDGE_INET);
            g_onenet_dm_type = MODEM_SWITCH_NETWORK_TYPE_2G;
        }
        else
        {
            return CIS_RET_ERROR;
        }
    }
    else
    {
        modem_switch_network_type_enum_t network_type = modem_switch_get_available_service();
        LOGD("mode switch network type: %d", network_type);
        if (network_type == MODEM_SWITCH_NETWORK_TYPE_NBIOT)
        {
            sock             = prvCreateSocket(0, AF_INET);
            g_onenet_nw_type = MODEM_SWITCH_NETWORK_TYPE_NBIOT;
        }
        else if (network_type == MODEM_SWITCH_NETWORK_TYPE_2G)
        {
            sock             = prvCreateSocket(0, AF_BRIDGE_INET);
            g_onenet_nw_type = MODEM_SWITCH_NETWORK_TYPE_2G;
        }
        else
        {
            return CIS_RET_ERROR;
        }
    }
#else  /* __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__ */
    sock = prvCreateSocket(0, AF_INET);
#endif /* __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__ */
    if (sock < 0)
    {
        LOGD("Failed to open socket: %d %s", errno, strerror(errno));
        return CIS_RET_ERROR;
    }
    netctx->sock  = sock;
    netctx->state = 1;
    ((struct st_cis_context *)(netctx->context))->netCallback.onEvent(netctx, 
                                                                      cisnet_event_connected, 
                                                                      NULL, 
                                                                      netctx->context);

    cissys_assert(((struct st_cis_context *)(netctx->context))->recv_taskhandle == NULL);
    if (((struct st_cis_context *)(netctx->context))->isDM)
    {

        recv_taskhandle = os_task_create("dm_recv", callbackDMThread, netctx, 3072, OS_TASK_PRIORITY_MAX / 3, 10);
        if (recv_taskhandle)
        {
            ((struct st_cis_context *)(netctx->context))->recv_taskhandle = recv_taskhandle;
            os_task_startup(recv_taskhandle);
            return CIS_RET_OK;
        }
    }
    else
    {

        recv_taskhandle = os_task_create("cis_recv", callbackRecvThread, netctx, 3072, OS_TASK_PRIORITY_MAX / 3, 10);
        LOGI("callbackRecvThread created success.");

        if (recv_taskhandle)
        {
            ((struct st_cis_context *)(netctx->context))->recv_taskhandle = recv_taskhandle;
            os_task_startup(recv_taskhandle);
            return CIS_RET_OK;
        }
    }

    return CIS_RET_ERROR;
}

cis_ret_t cisnet_disconnect(cisnet_t netctx)
{
    netctx->state = 0;
    ((struct st_cis_context *)(netctx->context))->netCallback.onEvent(netctx, 
                                                                      cisnet_event_disconnect, 
                                                                      NULL, 
                                                                      netctx->context);
    return 1;
}

cis_ret_t cisnet_write(cisnet_t netctx, const uint8_t *buffer, uint32_t length)
{
    int                nbSent;
    size_t             addrlen;
    size_t             offset;
    struct sockaddr_in saddr;
    saddr.sin_family      = AF_INET;
    saddr.sin_port        = htons(netctx->port);
    saddr.sin_addr.s_addr = inet_addr(netctx->host);

    addrlen = sizeof(saddr);
    offset  = 0;
    while (offset != length)
    {
        nbSent = sendto(netctx->sock, 
                        (const char *)buffer + offset, 
                        length - offset, 
                        0, 
                        (struct sockaddr *)&saddr, 
                        addrlen);

        if (nbSent == -1)
        {
            LOGE("socket sendto [%s:%d] failed.", netctx->host, ntohs(saddr.sin_port));
            return -1;
        }
        else
        {
            LOGI("socket sendto [%s:%d] %d bytes", netctx->host, ntohs(saddr.sin_port), nbSent);
        }
        offset += nbSent;
    }

    return CIS_RET_OK;
}

cis_ret_t cisnet_read(cisnet_t netctx, uint8_t **buffer, uint32_t *length)
{
    cissys_lock(g_lockPacketlist, CIS_CONFIG_LOCK_INFINITY);
    if (netctx->g_packetlist != NULL)
    {
        struct st_net_packet *delNode;
        *buffer              = netctx->g_packetlist->buffer;
        *length              = netctx->g_packetlist->length;
        delNode              = netctx->g_packetlist;
        netctx->g_packetlist = netctx->g_packetlist->next;
        cis_free(delNode);
        cissys_unlock(g_lockPacketlist);
        return CIS_RET_OK;
    }

    cissys_unlock(g_lockPacketlist);
    return CIS_RET_ERROR;
}

cis_ret_t cisnet_free(cisnet_t netctx, uint8_t *buffer, uint32_t length)
{
    cis_free(buffer);
    return CIS_RET_ERROR;
}

static int prvCreateSocket(uint16_t localPort, int ai_family)
{
    int sock = net_Socket(ai_family, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        return -1;
    }
    return sock;
}

void callbackRecvThread(void *lpParam)
{
    cisnet_t netctx = (cisnet_t)lpParam;
    int      sock   = netctx->sock;

    LOGI("callbackRecvThread start.");
    while (0 == netctx->quit && netctx->state == 1)
    {
        struct timeval tv = {5, 0};
        fd_set         readfds;
        int            result;

#if defined(__NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__) && defined(MTK_SOCKET_AGENT_SUPPORT)
        if (g_onenet_nw_switch == true)
        {
            modem_switch_network_type_enum_t network_type = modem_switch_get_available_service();
            LOGD("mode switch network type: %d", network_type);
            if (network_type == MODEM_SWITCH_NETWORK_TYPE_NBIOT)
            {
                net_Close(netctx->sock);
                sock             = prvCreateSocket(0, AF_INET);
                netctx->sock     = sock;
                g_onenet_nw_type = MODEM_SWITCH_NETWORK_TYPE_NBIOT;
            }
            else if (network_type == MODEM_SWITCH_NETWORK_TYPE_2G)
            {
                net_Close(netctx->sock);
                sock             = prvCreateSocket(0, AF_BRIDGE_INET);
                netctx->sock     = sock;
                g_onenet_nw_type = MODEM_SWITCH_NETWORK_TYPE_2G;
            }
            g_onenet_nw_switch = false;
        }
#endif /* __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__ */

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        /*
         * This part will set up an interruption until an event happen on SDTIN or the socket until "tv" timed out (set
         * with the precedent function)
         */
        result = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);
        
        if (result < 0)
        {
            LOGE("Error in select(): %d %s", errno, strerror(errno));
        }
        else if (result > 0)
        {
            uint8_t buffer[MAX_PACKET_SIZE];
            int     numBytes;

            /*
             * If an event happens on the socket
             */
            if (FD_ISSET(sock, &readfds))
            {
                struct sockaddr_storage addr;
                socklen_t               addrLen;

                addrLen = sizeof(addr);

                /*
                 * We retrieve the data received
                 */
                numBytes = recvfrom(sock, (char *)buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&addr, &addrLen);

                if (numBytes < 0)
                {
                    if (errno == 0)
                    {
                        continue;
                    }

                    LOGE("Error in recvfrom(): numBytes = %d, errno = %d %s", numBytes, errno, strerror(errno));
                }
                else if (numBytes > 0)
                {
                    char     s[INET_ADDRSTRLEN];
                    uint16_t port;

                    struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
                    inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET_ADDRSTRLEN);
                    port = saddr->sin_port;

                    LOGI("%d bytes received from [%s]:%hu", numBytes, inet_ntoa(saddr->sin_addr), ntohs(port));
                    uint8_t *data = (uint8_t *)cis_malloc(numBytes);
                    cis_memcpy(data, buffer, numBytes);

                    struct st_net_packet *packet = (struct st_net_packet *)cis_malloc(sizeof(struct st_net_packet));
                    packet->next                 = NULL;
                    packet->dummy_id             = 0;
                    packet->buffer               = data;
                    packet->length               = numBytes;
                    cissys_lock(g_lockPacketlist, CIS_CONFIG_LOCK_INFINITY);
                    netctx->g_packetlist = (struct st_net_packet *)CIS_LIST_ADD(netctx->g_packetlist, packet);
                    cissys_unlock(g_lockPacketlist);
                }
            }
        }
    }
    LOGE("Socket recv thread exit.");
    ((struct st_cis_context *)(netctx->context))->recv_taskhandle = NULL;
    cissys_unlock(((struct st_cis_context *)(netctx->context))->lockSocket);
}

void callbackDMThread(void *lpParam)
{

    cisnet_t netctx = (cisnet_t)lpParam;
    int      sock   = netctx->sock;
    while (0 == netctx->quit && netctx->state == 1)
    {
        struct timeval tv = {5, 0};
        fd_set         readfds;
        int            result;

#if defined(__NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__) && defined(MTK_SOCKET_AGENT_SUPPORT)
        if (g_onenet_dm_switch == true)
        {
            modem_switch_network_type_enum_t network_type = modem_switch_get_available_service();
            LOGD("mode switch network type: %d", network_type);
            if (network_type == MODEM_SWITCH_NETWORK_TYPE_NBIOT)
            {
                net_Close(netctx->sock);
                sock             = prvCreateSocket(0, AF_INET);
                netctx->sock     = sock;
                g_onenet_dm_type = MODEM_SWITCH_NETWORK_TYPE_NBIOT;
            }
            else if (network_type == MODEM_SWITCH_NETWORK_TYPE_2G)
            {
                net_Close(netctx->sock);
                sock             = prvCreateSocket(0, AF_BRIDGE_INET);
                netctx->sock     = sock;
                g_onenet_dm_type = MODEM_SWITCH_NETWORK_TYPE_2G;
            }
            g_onenet_dm_switch = false;
        }
#endif /* __NBIOT_ONLY_LOW_POWER_MODE_SUPPORT__ */

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        /*
         * This part will set up an interruption until an event happen on SDTIN or the socket until "tv" timed out (set
         * with the precedent function)
         */
        result = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);

        if (result < 0)
        {
            LOGE("Error in select(): %d %s", errno, strerror(errno));
        }
        else if (result > 0)
        {
            uint8_t buffer[MAX_PACKET_SIZE];
            int     numBytes;

            /*
             * If an event happens on the socket
             */
            if (FD_ISSET(sock, &readfds))
            {
                struct sockaddr_storage addr;
                socklen_t               addrLen;

                addrLen = sizeof(addr);

                /*
                 * We retrieve the data received
                 */
                numBytes = recvfrom(sock, (char *)buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&addr, &addrLen);

                if (numBytes < 0)
                {
                    if (errno == 0)
                    {
                        continue;
                    }
                    LOGE("Error in recvfrom(): %d %s", errno, strerror(errno));
                }
                else if (numBytes > 0)
                {
                    char     s[INET_ADDRSTRLEN];
                    uint16_t port;

                    struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
                    inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET_ADDRSTRLEN);
                    port = saddr->sin_port;

                    LOGI("%d bytes received from [%s]:%hu", numBytes, s, ntohs(port));

                    uint8_t *data = (uint8_t *)cis_malloc(numBytes);
                    cis_memcpy(data, buffer, numBytes);

                    struct st_net_packet *packet = (struct st_net_packet *)cis_malloc(sizeof(struct st_net_packet));
                    packet->next                 = NULL;
                    packet->dummy_id             = 0;
                    packet->buffer               = data;
                    packet->length               = numBytes;
                    cissys_lock(g_lockPacketlist, CIS_CONFIG_LOCK_INFINITY);
                    netctx->g_packetlist = (struct st_net_packet *)CIS_LIST_ADD(netctx->g_packetlist, packet);
                    cissys_unlock(g_lockPacketlist);
                }
            }
        }
    }
    LOGE("Error in socket recv thread exit..");
    ((struct st_cis_context *)(netctx->context))->recv_taskhandle = NULL;
    cissys_unlock(((struct st_cis_context *)(netctx->context))->lockSocket);
    os_task_destroy(NULL);
}
