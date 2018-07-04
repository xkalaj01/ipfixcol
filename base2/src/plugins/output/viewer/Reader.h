//
// Created by jan on 18.6.18.
//

#ifndef IPFIXCOL_READER_H
#define IPFIXCOL_READER_H

#include <ipfixcol2.h>

void read_packet(ipx_msg_ipfix_t *msg);

void read_record(struct ipx_ipfix_record *rec);

void read_field(struct fds_drec_field *field);

#endif //IPFIXCOL_READER_H
