//
// Created by jan on 18.6.18.
//

#include <inttypes.h>
#include "Reader.h"

void read_packet(ipx_msg_ipfix_t *msg) {

    //

    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr = (const struct fds_ipfix_msg_hdr*)ipx_msg_ipfix_get_packet(msg);

    //Read packet header
    printf("\n");
    printf("VERSION:[%"PRIu16"] ",ntohs(ipfix_msg_hdr->version));
    printf("LENGTH:[%"PRIu16"] ",ntohs(ipfix_msg_hdr->length));
    printf("EXPORT_TIME:[%"PRIu32"] ",ntohl(ipfix_msg_hdr->export_time));
    printf("SEQ_NUM:[%"PRIu32"] ",ntohl(ipfix_msg_hdr->seq_num));
    printf("ODID:[%"PRIu32"] \n", ntohl(ipfix_msg_hdr->odid));

    //Read all records from packet
    const uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(msg);
    for (uint32_t i = 0; i < rec_cnt; ++i) {

        //Print record number
        printf("\t Record n.%"PRIu32" of %"PRIu32"\n",i+1,rec_cnt);

        //Get the specific record and read all the fields
        struct ipx_ipfix_record *ipfix_rec = ipx_msg_ipfix_get_drec(msg,i);
        read_record(ipfix_rec);
    }
}

void read_record(struct ipx_ipfix_record *rec) {

    struct fds_drec_iter iter;
    fds_drec_iter_init(&iter, &(rec->rec), 0);

    //iterate through all the fields in record
    while (fds_drec_iter_next(&iter) != FDS_ERR_NOTFOUND) {
        struct fds_drec_field field = iter.field;

        read_field(field);
    }
}

void read_field(struct fds_drec_field field) {
    const struct fds_tfield *info = field.info;

    if (info->def == NULL){
        printf("\t\tEN: [%" PRIu32 "] \t\t ID: [%" PRIu16"]\t\t", info->en, info->id);
        print_octet(field);
        return;
    }
    else {
        printf("%s : %s :: ", info->def->scope->name, info->def->name);
    }

    switch(info->def->data_type){
        case (FDS_ET_OCTET_ARRAY):
            print_octet(field);
            break;
        case (FDS_ET_UNSIGNED_8) :
        case (FDS_ET_UNSIGNED_16) :
        case (FDS_ET_UNSIGNED_32) :
        case (FDS_ET_UNSIGNED_64) :
            print_uint(field);
            break;
        case (FDS_ET_SIGNED_8) :
        case (FDS_ET_SIGNED_16) :
        case (FDS_ET_SIGNED_32) :
        case (FDS_ET_SIGNED_64) :
            print_int(field);
            break;
        case (FDS_ET_FLOAT_32) :
        case (FDS_ET_FLOAT_64) :
            print_float(field);
            break;
        case (FDS_ET_BOOLEAN) :
            print_bool(field);
        case (FDS_ET_MAC_ADDRESS) :

        case (FDS_ET_STRING) :

        case (FDS_ET_DATE_TIME_SECONDS) :
        case (FDS_ET_DATE_TIME_MILLISECONDS) :
        case (FDS_ET_DATE_TIME_MICROSECONDS) :
        case (FDS_ET_DATE_TIME_NANOSECONDS) :

        case (FDS_ET_IPV4_ADDRESS) :
        case (FDS_ET_IPV6_ADDRESS) :
            printf("\n");
            break;
        default:
            print_octet(field);
            break;
    }
}

void print_octet(const struct fds_drec_field field) {
    if (field.size <= 8) {
        // Print as unsigned integer
        print_uint(field);
        return;
    }

    const size_t mem_req = (2 * field.size) + 3U; // "0x" + 2 chars per byte + "\0"
    char buffer[mem_req];

    buffer[0] = '0';
    buffer[1] = 'x';

    size_t buffer_size_remain = sizeof(buffer) - 2;
    int res = fds_octet_array2str(field.data, field.size, &buffer[2], buffer_size_remain);
    if (res >= 0) {
        printf("%s\n",buffer);
    }
}

void print_int(const struct fds_drec_field field) {
    char buffer[FDS_CONVERT_STRLEN_INT];

    size_t buffer_size_remain = sizeof(buffer);
    int res = fds_int2str_be(field.data, field.size, &buffer[0], buffer_size_remain);
    if (res > 0) {
        printf("%s\n",buffer);
    }

    if (res != FDS_ERR_BUFFER) {
       //some code
    }

}

void print_uint(const struct fds_drec_field field) {
    char buffer[FDS_CONVERT_STRLEN_INT];

    size_t buffer_size_remain = sizeof(buffer);
    int res = fds_uint2str_be(field.data, field.size, &buffer[0], buffer_size_remain);
    if (res > 0) {
        printf("%s\n",buffer);
        return;
    }

    if (res != FDS_ERR_BUFFER) {
        //some code
    }

}

void print_float(const struct fds_drec_field field) {
// We cannot use default function because "nan" and "infinity" values
    double value;
    if (fds_get_float_be(field.data, field.size, &value) != FDS_OK) {
        //error message goes here
        return;
    }

    if (isfinite(value) != 0) {
        // Normal value
        char buffer[50]; //Magic constant - replace

        const char *fmt = (field.size == sizeof(float))
                          ? ("%." FDS_CONVERT_STRX(FLT_DIG) "g")  // float precision
                          : ("%." FDS_CONVERT_STRX(DBL_DIG) "g"); // double precision
        int str_real_len = snprintf(buffer, sizeof(buffer), fmt, value);
        if (str_real_len < 0) {
            //error message goes here
            return;
        }

        if ((size_t)str_real_len >= sizeof(buffer)) {
            // Reallocate the buffer and try again
            //buffer_reserve(2 * buffer_alloc()); // Just double it
            //to_float(field);
            return;
        }

        printf("%s\n", buffer);
        return;
    }

    // +/-Nan or +/-infinite
    const char *str;
    // Size 8 (double)
    if (isinf(value)) {
        str = "inf";
        //Checking -inf
//    } else if (value == -std::numeric_limits<double>::infinity()) {
//        str = "\"-inf\"";
    } else if (isnan(value)) {
        str = "nan";
        //Checking -nan
//    } else if (value == -std::numeric_limits<double>::quiet_NaN()) {
//        str = "\"-nan\"";
    } else {
        str = "null";
    }

    printf("%s\n",str);
}

void print_bool(const struct fds_drec_field field) {
    if (field.size != 1) {
//        error message goes here
        return;
    }
    char buffer[FDS_CONVERT_STRLEN_FALSE] //False has more letters than true

    int res = fds_bool2str(field.data, &buffer[0], sizeof(buffer));
    if (res > 0) {
        printf("%s\n",buffer);
        return;
    }

    if (res != FDS_ERR_BUFFER) {
        //error message
    }
}

void print_mac(const struct fds_drec_field field) {
    buffer_reserve(buffer_used() + FDS_CONVERT_STRLEN_MAC + 2); // MAC address + 2x '\"'

    *(record.write_begin++) = '"';
    int res = fds_mac2str(field.data, field.size, record.write_begin, buffer_remain());
    if (res > 0) {
        record.write_begin += res;
        *(record.write_begin++) = '"';
        return;
    }

}

