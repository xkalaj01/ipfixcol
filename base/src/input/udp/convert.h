/**
 * \file convert.h
 * \author Michal Kozubik <kozubik.michal@gmail.com>
 * \brief Packet conversion from Netflow v5/v9 or sFlow to IPFIX format.
 *
 * Copyright (C) 2011 CESNET, z.s.p.o.
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
 * This software is provided ``as is, and any express or implied
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

#ifndef __CONVERT_H
#define __CONVERT_H

enum {
	UDP_PLUGIN,
	TCP_PLUGIN,
	SCTP_PLUGIN
};

/**
  * \brief Prepare static variables used for inserting template and data sets
  *  
  *  Also allocate memory for templates
  *
  *  \param[in] in_plugin Type of input plugin (UDP_PLUGIN...)
  *  \param[in] len Length of buff used in plugins
  *  \return 0 on success
  */
int convert_init(int in_plugin, int len);

/**
  * \brief Main function for packet conversion
  *
  * \param[in] packet Received packet
  * \param[in] len Packet length from function recvfrom etc.
  * \param[in] info_list info_list structure (used only in UDP_PLUGIN)
  */
void convert_packet(char **packet, ssize_t *len, char *input_info);

/**
  * \brief Reallocate memory for templates
  * 
  * \return 0 on success
  */

void convert_close();


#endif