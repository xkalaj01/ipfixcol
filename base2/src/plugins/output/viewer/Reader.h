//
// Created by jan on 18.6.18.
//

#ifndef IPFIXCOL_READER_H
#define IPFIXCOL_READER_H


void read_packet(ipx_msg_ipfix_t *msg);

void read_records(const ipx_msg_ipfix_t *msg);

void read_fields(struct ipx_ipfix_record *record);

#endif //IPFIXCOL_READER_H
