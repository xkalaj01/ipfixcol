//
// Created by jan on 18.6.18.
//

#ifndef IPFIXCOL_READER_H
#define IPFIXCOL_READER_H

#include <ipfixcol2.h>

void read_packet(ipx_msg_ipfix_t *msg);

void read_record(struct ipx_ipfix_record *rec);

void read_field(struct fds_drec_field field);

// Conversion functions
void print_octet(const struct fds_drec_field field);
void print_int(const struct fds_drec_field field);
void print_uint(const struct fds_drec_field field);
void print_float(const struct fds_drec_field field);
void print_bool(const struct fds_drec_field field);
void print_mac(const struct fds_drec_field field);
//void to_ip(const struct fds_drec_field &field);
//void to_string(const struct fds_drec_field &field);
//void to_datetime(const struct fds_drec_field &field);
//void to_flags(const struct fds_drec_field &field);
//void to_proto(const struct fds_drec_field &field);

#endif //IPFIXCOL_READER_H
