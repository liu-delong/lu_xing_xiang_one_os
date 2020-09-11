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
#include "cis_log.h"
#include "cis_api.h"
#include "cis_internals.h"

#include <ctype.h>


#if CIS_ENABLE_LOG
//#if LOG_OUTPUT_EXTINFO
//log_create_module(cis_onenet, PRINT_LEVEL_INFO);
//#endif
#endif

bool     gLogEnabled = true;
uint8_t  gLogLevel = LOG_LEVEL_DEBUG;
uint8_t  gLogExoutput = 0x00;
uint16_t gLogBufferLength = LOG_TEXT_SIZE_MIN;

#if 0
static void prvPrintIndent(int num)
{
    int i;
    for ( i = 0 ; i < num ; i++)LOG_PRINT("    ");
}
#endif

void log_config(bool enable,uint8_t exoutput,uint8_t level,uint16_t bufsize)
{
    gLogEnabled = enable;
    gLogLevel = level;
    gLogExoutput = exoutput;
    bufsize =  CIS_MAX(bufsize,LOG_TEXT_SIZE_MIN);
    bufsize =  CIS_MIN(bufsize,LOG_TEXT_SIZE_MAX);
    gLogBufferLength = bufsize;
}

#if LOG_DUMP_ENABLED
uint32_t onenet_at_bin_to_hex(char *dest, const uint8_t *source, uint32_t max_dest_len)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    uint32_t i = 0, j = 0;
    uint8_t ch1, ch2;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    while (j + 1 < max_dest_len)
    {
        ch1 = (source[i] & 0xF0) >> 4;
        ch2 = source[i] & 0x0F;

        if (ch1 <= 9) {
            *(dest + j) = ch1 + '0';
        } else {
            *(dest + j) = ch1 + 'A' - 10;
        }

        if (ch2 <= 9) {
            *(dest + j + 1) = ch2 + '0';
        } else {
            *(dest + j + 1) = ch2 + 'A' - 10;
        }

        i++;
        j += 2;
    }

    *(dest + j) = '\0';
    return j;
}
void log_dump(const char* title,const uint8_t * buffer,int length,int indent)
{
    int i;

    if(!gLogEnabled)
    {
        return;
    }

    if (length <= 0)
    {
		 LOG_PRINT("\n");
		 return;
	}

    if(title != NULL)
    {
        LOG_PRINT("->[%s]>>---\r\n",title);
    }
    else
    {
	    LOG_PRINT("-----\r\n");
    }

    i = 0;
    if (i < length)
    {
        char *array = cissys_malloc(2 * length + 1);
        onenet_at_bin_to_hex(array, buffer, 2 * length);
        LOG_PRINT("%s", array);
        cissys_free(array);
    }
    if(title != NULL){
        LOG_PRINT("<-[%s]<<---\r\n",title);
    }else{
        LOG_PRINT("-----\r\n");
    }
}
#endif//LOG_DUMP_ENABLED
