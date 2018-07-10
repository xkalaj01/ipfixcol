/**
 * \file src/plugins/output/viewer/Reader.c
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz.cz>
 * \brief Output viewer module for showing basic data oon STDIN (header file)
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

#include <inttypes.h>
#include <libfds.h>
#include "Reader.h"
#include "../../../core/message_ipfix.h"

/**
 * \brief Reads the data inside of ipfix message
 *
 * Function reads and prints header of the packet and then iterates through the records.
 * Information printed from header of packet are:
 * version, length, export time, sequence number and Observation Domain ID
 * \param[in] msg ipfix message which will be printed
 */
void read_packet(ipx_msg_ipfix_t *msg, struct fds_iemgr *iemgr) {

    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr = (const struct fds_ipfix_msg_hdr*)ipx_msg_ipfix_get_packet(msg);

    if (ipfix_msg_hdr->length < FDS_IPFIX_MSG_HDR_LEN){
        return;
    }

    //Read packet header
    printf("\n## Message header\n");
    printf("\tversion:\t%"PRIu16"\n",ntohs(ipfix_msg_hdr->version));
    printf("\tlength:\t\t%"PRIu16"\n",ntohs(ipfix_msg_hdr->length));
    printf("\texport time:\t%"PRIu32"\n",ntohl(ipfix_msg_hdr->export_time));
    printf("\tsequence no.:\t%"PRIu32"\n",ntohl(ipfix_msg_hdr->seq_num));
    printf("\tODID:\t\t%"PRIu32"\n", ntohl(ipfix_msg_hdr->odid));

    struct fds_ipfix_msg_hdr *temp = (struct fds_ipfix_msg_hdr*) ipx_msg_ipfix_get_packet(msg);
    struct fds_sets_iter sets_iter;
    fds_sets_iter_init(&sets_iter,temp);

    struct ipx_ipfix_set *sets;
    size_t set_cnt;
    ipx_msg_ipfix_get_sets(msg,&sets,&set_cnt);

    //Read all records from packet
    const uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(msg);
    uint32_t rec_i = 0;

    printf("NUMBER OF SETS %zu\n",set_cnt);

    for (uint32_t i = 0; i < set_cnt; ++i){
        fds_sets_iter_next(&sets_iter);
        uint8_t *set_end = (uint8_t *)sets_iter.set + ntohs(sets_iter.set->length);
        uint16_t set_id = ntohs(sets_iter.set->flowset_id);

        printf("\n** Set header\n");
        printf("\tset ID: %"PRIu16"\n",set_id);
        printf("\tlength: %"PRIu16"\n",ntohs(sets_iter.set->length));

        //Template set
        if (set_id < 255){

            struct fds_tset_iter tset_iter;
            fds_tset_iter_init(&tset_iter,sets_iter.set);

            //Iteration through all templates in the set
            while (fds_tset_iter_next(&tset_iter) == FDS_OK){
                //setting the type of the Template
                enum fds_template_type type;
                void *ptr;
                switch (set_id){
                    case FDS_IPFIX_SET_TMPLT:
                        type = FDS_TYPE_TEMPLATE;
                        ptr = tset_iter.ptr.trec;
                        break;
                    case FDS_IPFIX_SET_OPTS_TMPLT:
                        type = FDS_TYPE_TEMPLATE_OPTS;
                        ptr = tset_iter.ptr.opts_trec;
                        break;
                    default:
                        printf("\tundefined template\n");
                        continue;
                }
                //Filling the template structure with data from raw packet
                uint16_t tmplt_size = tset_iter.size;
                struct fds_template *tmplt;
                fds_template_parse(type,ptr,&tmplt_size,&tmplt);

                //Printing out the header
                printf("\n-- Template header\n");
                printf("\ttemplate id: %"PRIu16"\n",tmplt->id);
                printf("\tfield count: %"PRIu16"\n",tmplt->fields_cnt_total);

                //Using IEManager to fill the definitions of the fields in the template
                fds_template_ies_define(tmplt, iemgr , false);

                //Iteration through the fields and printing them out
                for (uint16_t i = 0; i < tmplt->fields_cnt_total ; ++i) {
                    printf("++ %s\n",tmplt->fields[i].def->name);
                }
            }

        }
        else{
            //Data set
            struct ipx_ipfix_record *ipfix_rec = ipx_msg_ipfix_get_drec(msg,rec_i);

            //Check if the record belongs to current set
            while ((ipfix_rec != NULL) && (ipfix_rec->rec.data < set_end) && (rec_i < rec_cnt)){
                //Print record header
                printf("\n-- Record header [n.%"PRIu32" of %"PRIu32"]\n",rec_i+1,rec_cnt);

                //Get the specific record and read all the fields
                read_record(ipfix_rec);

                //Get the next record
                rec_i++;
                ipfix_rec = ipx_msg_ipfix_get_drec(msg,rec_i);
            }

        }
    }
}

/**
 * \brief Reads data inside the single record of IPFIX message
 *
 * Reads and prints the header of the record and then iterates through the fields.
 * Information printed from header of record are:
 * Template ID and number of the fields inside the record
 * \param[in] rec record which will be printed
 */
void read_record(struct ipx_ipfix_record *rec) {
    //Write info from header about the record template
    printf("\ttemplate id: %"PRIu16"\n",rec->rec.tmplt->id);
    printf("\tfield count: %"PRIu16"\n",rec->rec.tmplt->fields_cnt_total);
    printf("\tdata length: %"PRIu16"\n",rec->rec.tmplt->data_length);
    printf("\tsize       : %"PRIu16"\n",rec->rec.size);

    //iterate through all the fields in record
    struct fds_drec_iter iter;
    fds_drec_iter_init(&iter, &(rec->rec), 0);

    printf("\n\tfields:\n");
    while (fds_drec_iter_next(&iter) != FDS_ERR_NOTFOUND) {
        struct fds_drec_field field = iter.field;

        read_field(&field);
    }
}

/**
 * \brief Reads the data inside the field of IPFIX message
 *
 * Reads and prints all the information about the data in the field
 * if the detailed definition is known, data are printed in readable format
 * as well as the information about the data (organisation name and name of the data).
 * Otherwise data are printed in the raw format (hexadecimal)
 * In both cases Enterprise number and ID will be printed.
 * \param[in] field Field which will be printed
 */
void read_field(struct fds_drec_field *field) {
    //Write info from header about field
    printf("\ten:\t%" PRIu32 "\tid:\t%" PRIu16"\t", field->info->en, field->info->id);

    enum fds_iemgr_element_type type;
    char *org;
    char *field_name;
    char *unit;

    //Get the organisation name, field name and unit of the data.
    if (field->info->def == NULL){
        type = FDS_ET_OCTET_ARRAY;
        org = "unknown";
        field_name = "unknown";
        unit = "";
    }
    else {
        type = field->info->def->data_type;
        org = field->info->def->scope->name;
        field_name = field->info->def->name;

        switch(field->info->def->data_unit){
            case FDS_EU_NONE :
                unit = "";
                break;
            case FDS_EU_BITS :
                unit = "bits";
                break;
            case FDS_EU_OCTETS :
                unit = "octets";
                break;
            case FDS_EU_PACKETS :
                unit = "packets";
                break;
            case FDS_EU_FLOWS:
                unit = "flows";
                break;
            case FDS_EU_SECONDS :
                unit = "seconds";
                break;
            case FDS_EU_MILLISECONDS :
                unit = "milliseconds";
                break;
            case FDS_EU_MICROSECONDS :
                unit = "microseconds";
                break;
            case FDS_EU_NANOSECONDS :
                unit = "nanoseconds";
                break;
            case FDS_EU_4_OCTET_WORDS :
                unit = "4 octet word";
                break;
            case FDS_EU_MESSAGES :
                unit = "messages";
                break;
            case FDS_EU_HOPS :
                unit = "hops";
                break;
            case FDS_EU_ENTRIES :
                unit = "entries";
                break;
            case FDS_EU_FRAMES :
                unit = "frames";
                break;
            default:
                unit = "";
                break;
        }
    }
    printf("[%s] %s : ",org,field_name);

    //Read and write the data from the field
    char buffer[1024];
    int res = fds_field2str_be(field->data, field->size, type, buffer, sizeof(buffer));

    if(res >= 0){
        //Conversion was successful
        printf("%s",buffer);
        if (unit != "")
            printf(" (%s)",unit);
        printf("\n");
        return;
    }
    else if (res == FDS_ERR_BUFFER){
        //Buffer too small
        printf("<Data is too long to show>\n");
        return;
    }
    else{
        //Any other error
        printf("<Invalid value>\n");
        return;
    }
}

void read_set(struct fds_tset_iter *set_iter) {


}



