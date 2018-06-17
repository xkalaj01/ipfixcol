//
// Created by jan on 18.6.18.
//

#include <ipfixcol2/message_ipfix.h>
#include <inttypes.h>
#include "Reader.h"

void read_packet(ipx_msg_ipfix_t *msg) {

    //Here the header will be printed
    printf("I am the header of the IPFIX packet\n");

    //Read all records from packet
    read_records(msg);
}

void read_records(const ipx_msg_ipfix_t *msg) {
    //Get number of records and iterate through them
    const uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(msg);
    for(uint32_t i = 0; i < rec_cnt; ++i){

        //print info about the record
        printf("\t Record n.%"PRIu32" of %"PRIu32"\n",i,rec_cnt);

        //get the record and read all the fields
        struct ipx_ipfix_record *ipfix_rec = ipx_msg_ipfix_get_drec(msg,i);
        read_fields(ipfix_rec);
    }
}

void read_fields(struct ipx_ipfix_record *record) {
    struct fds_drec_iter iter;
    fds_drec_iter_init(&iter, &(record->rec), 0);

    //iterate through all the fields in record
    while (fds_drec_iter_next(&iter) != FDS_ERR_NOTFOUND) {
        const struct fds_tfield *field = iter.field.info;

        printf("\t\tEN: [%" PRIu32 "] \t\t ID: [%" PRIu16"]\t\t", field->en, field->id);
        printf("%s : %s->\n",field->def->scope->name, field->def->name);
    }
}
