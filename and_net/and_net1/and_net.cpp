#ifndef MY_BUFF
#define MY_BUFF
#include "and_net.h"
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <boost/asio.hpp>

using namespace boost;

namespace and_net
{

namespace errors
{

and_net_exception::and_net_exception(std::string err_str)
	: std::runtime_error(err_str)
{
}

last_msg_no_EM::last_msg_no_EM()
	: and_net_exception("Cant find end marker in the remaining message.")
{
}

last_msg_shorter::last_msg_shorter()
	: and_net_exception("The last message is shorter that required.")
{
}

no_EM_buffer_overflowing::no_EM_buffer_overflowing()
	: and_net_exception("EM not found in all overflowing buffer.")
{
}

msg_longer_buffer::msg_longer_buffer()
	: and_net_exception("Requested message longer that buffer size.")
{
}

} //end namespace errors


net_one::net_one(ip::tcp::socket *sock, size_t size)
	: m_max_buf_size(size), m_sock(sock), m_EOF_reached(false), m_EOS(false)
{
	// BUFFER
	// > tail data space <---- |m_mid_pos| ----> main data space <
	m_data.resize(size * 2);
	m_mid_pos = m_get_pos = m_ins_pos = m_data.begin() + size;
}

net_one::~net_one()
{
}

size_t net_one::unreaded_size()
{
	return m_get_pos < m_ins_pos ? std::distance(m_get_pos, m_ins_pos) : 0;
}

size_t net_one::free_size()
{
	return m_ins_pos < m_data.end() ? std::distance(m_ins_pos, m_data.end()) : 0;
}

void net_one::move_tail()
{
	size_t to_move_bytes = unreaded_size();
	std::copy(m_get_pos, m_ins_pos, m_mid_pos - to_move_bytes);
	m_ins_pos = m_mid_pos;
	m_get_pos = m_mid_pos - to_move_bytes;
}

std::string net_one::read_str(size_t to_read_size)
{
	size_t bytes = 0;
	system::error_code err_code;

	while (to_read_size > unreaded_size())
	{
		if ((ssize_t)to_read_size > std::distance(m_get_pos, m_data.end()))
		{
			if (to_read_size > m_max_buf_size)
			{
				throw errors::msg_longer_buffer();
			}
			move_tail();
		}
		if (m_EOF_reached) 
		{
			throw errors::last_msg_shorter();
		}
		bytes = m_sock->read_some(buffer(&(*m_ins_pos), free_size()), err_code);
		m_ins_pos += bytes;
		if (err_code.value())
		{
			if (err_code == asio::error::eof)
			{
				m_EOF_reached = true;
			} else {
				throw system::system_error(err_code);
			}
		}
	}
	std::string out(static_cast<char*>(&(*m_get_pos)), to_read_size);
	m_get_pos += to_read_size;
	m_EOS = (m_EOF_reached && unreaded_size() == 0);
	return out;
}

std::string net_one::read_str(std::string end_marker)
{
	size_t bytes;
	std::vector<char>::iterator search_pos;
	system::error_code err_code;

	while (true) 
	{
		search_pos = std::search(m_get_pos, m_ins_pos, end_marker.cbegin(), 
			end_marker.cend());
		// if end marker is finded break the loop
		if (search_pos < m_ins_pos)
		{
			break; //end while loop
		}
		
		// if end marker is not found, do following

		// chek that EOF not reached
		if (m_EOF_reached){
			// EOF reached and end market was not founded
			throw errors::last_msg_no_EM();
		}

		// check that the tail is empty and buffer has data
		if (m_get_pos > m_mid_pos)
		{
			move_tail();
		}

		if (free_size() == 0) {
			throw errors::no_EM_buffer_overflowing();
		}
		bytes = m_sock->read_some(buffer(&(*m_ins_pos), free_size()), err_code);
		m_ins_pos += bytes;
		// }
		if (err_code.value() != 0)
		{
			if (err_code == asio::error::eof)
			{
				m_EOF_reached = true;
			} else {
				throw system::system_error(err_code);
			}
		}
	}
	size_t result_size = std::distance(m_get_pos, search_pos);
	std::string result_string(static_cast<char*>(&(*m_get_pos)), result_size);
	m_get_pos = search_pos + end_marker.length();
	m_EOS = (m_get_pos == m_ins_pos && m_EOF_reached);
	return result_string;
}

std::string net_one::remains_str()
{
	return read_str(unreaded_size());
}

bool net_one::is_EOS()
{
	return m_EOS;
}


} //end namespace and


#endif