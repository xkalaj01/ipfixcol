//
// Created by jan on 18.6.18.
//

#include <inttypes.h>
#include "Reader.h"

void read_packet(ipx_msg_ipfix_t *msg) {

    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr = (const struct fds_ipfix_msg_hdr*)ipx_msg_ipfix_get_packet(msg);

    //Read packet header
    printf("\n## Message header\n");
    printf("\tversion:\t%"PRIu16"\n",ntohs(ipfix_msg_hdr->version));
    printf("\tlength:\t\t%"PRIu16"\n",ntohs(ipfix_msg_hdr->length));
    printf("\texport time:\t%"PRIu32"\n",ntohl(ipfix_msg_hdr->export_time));
    printf("\tsequence no.:\t%"PRIu32"\n",ntohl(ipfix_msg_hdr->seq_num));
    printf("\tODID:\t\t%"PRIu32"\n", ntohl(ipfix_msg_hdr->odid));

    //Read all records from packet
    const uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(msg);
    for (uint32_t i = 0; i < rec_cnt; ++i) {

        //Print record number
        printf("\n-- Record header [n.%"PRIu32" of %"PRIu32"]\n",i+1,rec_cnt);

        //Get the specific record and read all the fields
        struct ipx_ipfix_record *ipfix_rec = ipx_msg_ipfix_get_drec(msg,i);
        read_record(ipfix_rec);
    }
}

void read_record(struct ipx_ipfix_record *rec) {
    //Write info from header about the record template
    printf("\ttemplate id:%"PRIu16"\n",rec->rec.tmplt->id);
    printf("\tfield count:%"PRIu16"\n",rec->rec.tmplt->fields_cnt_total);

    //iterate through all the fields in record
    struct fds_drec_iter iter;
    fds_drec_iter_init(&iter, &(rec->rec), 0);

    printf("\n\tfields:\n");
    while (fds_drec_iter_next(&iter) != FDS_ERR_NOTFOUND) {
        struct fds_drec_field field = iter.field;

        read_field(&field);
    }
}

void read_field(struct fds_drec_field *field) {
    //Write info from header about field
    printf("\ten:\t%" PRIu32 "\tid:\t%" PRIu16"\t", field->info->en, field->info->id);

    enum fds_iemgr_element_type type;
    char *org;
    char *field_name;
    char *unit;

    //Get the organisation name and field name.
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

    if (res == FDS_ERR_BUFFER){
        //Buffer too small
        printf("*/Data is too long to show/*\n");
        return;
    }
    else if(res >= 0){
        //Conversion was successful
        printf("%s",buffer);
        if (unit != "")
            printf(" (%s)",unit);
        printf("\n");
        return;
    }
    else{
        //Any other error
        printf("ERROR PRINTING DATA\n");
        return;
    }
}



