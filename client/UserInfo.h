/*
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef USERINFO_H
#define USERINFO_H

#include "ClientManager.h"
#include "TaskQueue.h"
#ifdef IRAINMAN_USE_NG_FAST_USER_INFO
#include "UserInfoColumns.h"
#endif

enum Tasks { UPDATE_USER_JOIN,
             UPDATE_USER,
             REMOVE_USER,
             ADD_CHAT_LINE,
             ADD_STATUS_LINE,
             ADD_SILENT_STATUS_LINE,
             SET_WINDOW_TITLE,
             GET_PASSWORD,
             PRIVATE_MESSAGE,
             STATS,
             CONNECTED,
             DISCONNECTED,
             CHEATING_USER,
             USER_REPORT,
             GET_SHUTDOWN,
             SET_SHUTDOWN,
             KICK_MSG,
#ifdef RIP_USE_CONNECTION_AUTODETECT
             DIRECT_MODE_DETECTED
#endif
           };

struct OnlineUserTask : public Task // [!] IRainman fix.
#ifdef _DEBUG
		, virtual NonDerivable<OnlineUserTask>
#endif
{
	explicit OnlineUserTask(const OnlineUserPtr& ouser) : m_ouser(ouser)
	{
	}
	GETC(OnlineUserPtr, m_ouser, OnlineUser); // [!] IRainman fix: is its online user!!!
};

struct MessageTask : public Task // [!] IRainman fix.
#ifdef _DEBUG
		, virtual NonDerivable<MessageTask>
#endif
{
	explicit MessageTask(const ChatMessage& message) : m_message(message)
	{
	}
	GETC(ChatMessage, m_message, Message);
};

class UserInfo : public UserInfoBase
#ifdef _DEBUG
	, virtual NonDerivable<UserInfo> // [+] IRainman fix.
#endif
{
	private:
		const OnlineUserPtr m_ou; // [!] IRainman fix: use online user here!
		Util::CustomNetworkIndex m_location; // [+] IRainman opt.
#ifdef SCALOLAZ_BRIGHTEN_LOCATION_WITH_LASTIP
		bool m_is_ip_from_sql;
#endif
	public:
	
		explicit UserInfo(const OnlineUserTask& u) :
#ifdef SCALOLAZ_BRIGHTEN_LOCATION_WITH_LASTIP
			m_is_ip_from_sql(false),
#endif
			m_ou(u.getOnlineUser())
		{
		}
		static int compareItems(const UserInfo* a, const UserInfo* b, int col);
		bool is_update(int sortCol)
		{
#ifdef IRAINMAN_USE_NG_FAST_USER_INFO
			return (m_ou->getIdentity().getChanges() & (1 << sortCol)) != 0; // [!] IRAINMAN_USE_NG_FAST_USER_INFO
#else
			return true;
#endif
		}
		tstring getText(int p_col) const;
#ifdef PPA_INCLUDE_LASTIP_AND_USER_RATIO
#ifdef SCALOLAZ_BRIGHTEN_LOCATION_WITH_LASTIP
		bool isIPFromSQL() const
		{
			return m_is_ip_from_sql;
		}
#endif // SCALOLAZ_BRIGHTEN_LOCATION_WITH_LASTIP
#endif // PPA_INCLUDE_LASTIP_AND_USER_RATIO
		bool isOP() const
		{
			return getIdentity().isOp();
		}
		string getIp() const
		{
			return getIdentity().getIp();
		}
		uint8_t getImageIndex() const
		{
			return UserInfoBase::getImage(*m_ou);
		}
		// [+] IRainman opt.
		const Util::CustomNetworkIndex& getLocation() const
		{
			return m_location;
		}
		const Util::CustomNetworkIndex& calcLocation()
		{
			auto& l_location = getLocation();
			if (l_location.isNew())
			{
				const auto& l_ip = getIp();
#ifdef SCALOLAZ_BRIGHTEN_LOCATION_WITH_LASTIP
				calcIpFromSQL(l_ip);
#endif
				if (!l_ip.empty())
					setLocation(Util::getIpCountry(l_ip));
// TODO             else
// TODO                     setLocation(Util::CustomNetworkIndex(0,0));
			}
			return l_location;
		}
#ifdef SCALOLAZ_BRIGHTEN_LOCATION_WITH_LASTIP
		void calcIpFromSQL(const string& p_ip)
		{
			m_is_ip_from_sql = p_ip.empty() && getUser()->isLastIP();
		}
#endif
		void setLocation(const Util::CustomNetworkIndex& p_location)
		{
			m_location = p_location;
		}
		// [~] IRainman opt.
		const string& getNick() const
		{
			return m_ou->getIdentity().getNick();
		}
#ifdef IRAINMAN_USE_HIDDEN_USERS
		bool isHidden() const
		{
			return m_ou->getIdentity().isHidden();
		}
#endif
		const OnlineUserPtr& getOnlineUser() const
		{
			return m_ou;
		}
		const UserPtr& getUser() const
		{
			return m_ou->getUser();
		}
		const Identity& getIdentity() const
		{
			return m_ou->getIdentity();
		}
		tstring getHubs() const
		{
			return m_ou->getIdentity().getHubs();
		}
		static tstring formatSpeedLimit(const uint32_t limit);
		tstring getLimit() const;
		tstring getDownloadSpeed() const;
		typedef std::unordered_map<OnlineUserPtr, UserInfo*, OnlineUser::Hash> OnlineUserMapBase; // [!] IRainman fix: use online user here.
		class OnlineUserMap : public OnlineUserMapBase
#ifdef _DEBUG
			, virtual NonDerivable<OnlineUserMap>, boost::noncopyable // [+] IRainman fix.
#endif
		{
			public:
				UserInfo* findUser(const OnlineUserPtr& p_user) const // [!] IRainman fix: use online user here.
				{
					const auto i = find(p_user);
					return i == end() ? nullptr : i->second;
				}
		};
};


#endif //USERINFO_H
