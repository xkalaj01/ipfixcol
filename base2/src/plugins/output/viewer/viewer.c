/**
 * \file src/plugins/output/viewer/viewer.c
 * \author Lukas Hutak <lukas.hutak@cesnet.cz>
 * \brief Example output plugin for IPFIXcol 2
 * \date 2018
 */

/* Copyright (C) 2018 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include <ipfixcol2.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "config.h"
#include "../../input/dummy/config.h"
#include "../../../core/message_ipfix.h"
//Reader.h

/** Plugin description */
IPX_API struct ipx_plugin_info ipx_plugin_info = {
    // Plugin type
    .type = IPX_PT_OUTPUT,
    // Plugin identification name
    .name = "viewer",
    // Brief description of plugin
    .dsc = "Output plugin for ptinting information about incoming IPFIX messages.",
    // Configuration flags (reserved for future use)
    .flags = 0,
    // Plugin version string (like "1.2.3")
    .version = "0.1.0",
    // Minimal IPFIXcol version string (like "1.2.3")
    .ipx_min = "2.0.0"
};

/** Instance */
struct instance_data {
    /** Parsed configuration of the instance  */
    struct instance_config *config;
};

int
ipx_plugin_init(ipx_ctx_t *ctx, const char *params)
{
    // Create a private data
    struct instance_data *data = calloc(1, sizeof(*data));
    if (!data) {
        return IPX_ERR_DENIED;
    }

    if ((data->config = config_parse(ctx, params)) == NULL) {
        free(data);
        return IPX_ERR_DENIED;
    }

    ipx_ctx_private_set(ctx, data);

    // Subscribe to receive IPFIX messages and Transport Session events
    uint16_t new_mask = IPX_MSG_IPFIX | IPX_MSG_SESSION;
    ipx_ctx_subscribe(ctx, &new_mask, NULL);
    return IPX_OK;
}

void
ipx_plugin_destroy(ipx_ctx_t *ctx, void *cfg)
{
    (void) ctx; // Suppress warnings

    struct instance_data *data = (struct instance_data *) cfg;
    config_destroy(data->config);
    free(data);
}

int
ipx_plugin_process(ipx_ctx_t *ctx, void *cfg, ipx_msg_t *msg)
{
    int type = ipx_msg_get_type(msg);
    if (type != IPX_MSG_IPFIX) {
        return IPX_OK;
    }

    ipx_msg_ipfix_t *ipfix_msg = ipx_msg_base2ipfix(msg);

    struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr = (struct fds_ipfix_msg_hdr*)ipx_msg_ipfix_get_packet(ipfix_msg);

    //read packet header
    printf("\n");
    printf("VERSION:[%"PRIu16"] ",ntohs(ipfix_msg_hdr->version));
    printf("LENGTH:[%"PRIu16"] ",ntohs(ipfix_msg_hdr->length));
    printf("EXPORT_TIME:[%"PRIu32"] ",ntohl(ipfix_msg_hdr->export_time));
    printf("SEQ_NUM:[%"PRIu32"] ",ntohl(ipfix_msg_hdr->seq_num));
    printf("ODID:[%"PRIu32"] \n", ntohl(ipfix_msg_hdr->odid));


    //read record
    const uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(ipfix_msg);
    for(uint32_t i = 0; i < rec_cnt; ++i){

        //print info about the record
        printf("\t Record n.%"PRIu32" of %"PRIu32"\n",i+1,rec_cnt);

        //get the record and read all the fields
        struct ipx_ipfix_record *ipfix_rec = ipx_msg_ipfix_get_drec(ipfix_msg,i);

        struct fds_drec_iter iter;
        fds_drec_iter_init(&iter, &(ipfix_rec->rec), 0);

        //iterate through all the fields in record
        while (fds_drec_iter_next(&iter) != FDS_ERR_NOTFOUND) {
            const struct fds_tfield *field = iter.field.info;

            printf("\t\tEN: [%" PRIu32 "] \t\t ID: [%" PRIu16"]\t\t", field->en, field->id);
            printf("%s : %s-> \n",field->def->scope->name, field->def->name);
        }
    }
    return IPX_OK;

}
