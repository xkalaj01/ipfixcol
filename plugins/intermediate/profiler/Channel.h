/**
 * \file Channel.h
 * \author Michal Kozubik <kozubik@cesnet.cz>
 * \brief intermediate plugin for profiling data
 *
 * Copyright (C) 2015 CESNET, z.s.p.o.
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

#ifndef CHANNEL_H
#define	CHANNEL_H

#include "Profile.h"
#include "profiler.h"

#include <set>

class Channel {
	/* Shortcuts */
	using channelsSet = std::set<Channel *>;
public:
	/**
	 * \brief Constructor
	 * 
     * \param[in] name Channel name
     */
	Channel(std::string name);
	
	/**
	 * \brief Destructor
     */
	~Channel();

	/**
	 * \brief Set channel's profile
	 * 
     * \param[in] profile profile
     */
	void setProfile(Profile *profile);
	
	/**
	 * \brief Set channel's sources
	 * 
     * \param[in] sources comma separated list of sources from parent profile
     */
	void setSources(std::string sources);
	
	/**
	 * \brief Set channel's filter
	 * 
     * \param[in] filter filter
     */
	void setFilter(filter_profile *filter);

	/**
	 * \brief Get channel's ID
	 * 
     * \return channel's ID (unique within all channels)
     */
	channel_id_t getId() { return m_id; }
	
	/**
	 * \brief Get channel's name
	 * 
     * \return channel's name from startup configuration
     */
	std::string getName() { return m_name; }
	
	/**
	 * \brief Get listening channels
	 * 
     * \return vector of channels that want to receive data from this channel
     */
	channelsSet getListeners() { return m_listeners; }

	/**
	 * \brief Add new listening channel
	 * 
     * \param[in] listener new listener
     */
	void addListener(Channel *listener);

	/* Match channel with data record */
	/**
	 * \brief Match channel with data record (using channel's filter)
	 * 
     * \param[in] msg IPFIX message
     * \param[in] mdata Data record's metadata
     * \param[out] profiles	list of matching profiles and channels
     */
	void match(struct ipfix_message *msg, struct metadata *mdata, std::vector<couple_id_t>& profiles);
private:

	channel_id_t m_id;			/**< Channel ID */
	std::string m_name;			/**< Channel name */

	filter_profile *m_filter{};	/**< Filter */
	Profile *m_profile{};		/**< Profile */

	channelsSet m_listeners{};	/**< Listening channels */
	
	static channel_id_t channels_cnt;	/**< Total number of channels */
};

#endif	/* CHANNEL_H */

