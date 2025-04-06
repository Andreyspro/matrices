#include <iostream>
#include <iterator>
#include <stdexcept>
#include <boost/asio.hpp>
// #include "uuid.h"
#include "and_net3.h"

#define NOEXTRAOUT

using namespace boost;

namespace and_net
{

net_three::net_three(ip::tcp::socket *sock, size_t size)
	: m_sock(sock), m_data(size * 2),m_max_buf_size(size),
	m_EOF_reached(false), m_EOS(false)
{
	// BUFFER
	// > tail data space <---- |m_mid_pos| ----> main data space <
	// m_data.resize(size * 2);
	m_mid_pos = m_get_pos = m_ins_pos = m_data.begin() + size;
}

net_three::ptr_t net_three::get_new(ip::tcp::socket *sock, size_t size)
{
	return ptr_t(new  net_three(sock, size));
}

net_three::ptr_t net_three::get_ptr()
{
	return shared_from_this();
}

net_three::~net_three()
{
}

size_t net_three::unreaded_size() const
{
	return m_get_pos < m_ins_pos ? std::distance(m_get_pos, m_ins_pos) : 0;
}

size_t net_three::free_size() const
{
	return m_ins_pos < m_data.cend() ? 
		std::distance<std::vector<char>::const_iterator>(m_ins_pos, m_data.cend()) : 0;
}

size_t net_three::move_tail()
{
	size_t to_move_bytes = unreaded_size();
	if (to_move_bytes)
	{
		std::copy(m_get_pos, m_ins_pos, m_mid_pos - to_move_bytes);
	}
	m_ins_pos = m_mid_pos;
	m_get_pos = m_mid_pos - to_move_bytes;
	return to_move_bytes;
}

std::string net_three::remains_str()
{
	std::string result_str;
	try_read_str_sz(result_str, unreaded_size());
	return result_str;
}

bool net_three::is_EOS() const
{
	return m_EOS;
}

result_read_str_t net_three::try_read_str(size_t to_read_size)
{
	result_read_str_t result;
	if (to_read_size <= unreaded_size())
	{
		result.first = std::string(&*m_get_pos, to_read_size);
		result.second.read_result = READ_OK;
		result.second.reason = NO_ERROR;
	}
	return result;
}

op_read_status_t net_three::try_read_str_sz(std::string &result_str, size_t to_read_size)
{
	op_read_status_t status;
	if (to_read_size <= unreaded_size())
	{
		result_str = std::string(&*m_get_pos, to_read_size);
		m_get_pos += to_read_size;
	}
	else
	{
		status.read_result = READ_FAILED;
		if (m_EOF_reached)
		{
			status.reason = LAST_MSG_SHORTER;
		}
		else
		{
			status.reason = NEED_MORE_DATA;
		}
	}
	m_EOS = ((unreaded_size() == 0) && m_EOF_reached);
	return status;
}

op_read_status_t net_three::try_read_str_em(std::string & result_str, const std::string &end_marker)
{
	op_read_status_t status;
	std::vector<char>::iterator search_pos;
	search_pos = std::search(m_get_pos, m_ins_pos, end_marker.cbegin(), end_marker.cend());
	if (search_pos < m_ins_pos)
	{
		size_t result_size = std::distance(m_get_pos, search_pos);
		result_str = std::string(&*m_get_pos, result_size);
		m_get_pos += result_size + end_marker.length();
	} 
	else
	{
		status.read_result = READ_FAILED;
		if (m_EOF_reached)
		{
			status.reason = LAST_MSG_SHORTER;
		}
		else
		{
			status.reason = NEED_MORE_DATA;
		}
	}
	m_EOS = ((unreaded_size() == 0) && m_EOF_reached);
	return status;

}

void net_three::async_fill_buff(on_fill_buff_callback_t fill_callback)
{
	m_fill_callback = fill_callback;
	move_tail();
	m_sock->async_read_some
	(
		buffer(&*m_ins_pos, free_size()),
		std::bind
		(
			&net_three::on_fill_data, shared_from_this(),
			std::placeholders::_1, std::placeholders::_2
		)
	);
}

void net_three::cleancallback()
{
	m_fill_callback = NULL;
}

void net_three::on_fill_data(const system::error_code &err, size_t bytes)
{
	#ifdef EXTRAOUT
	std::cout << "++" << shared_from_this().use_count() << std::endl;
	#endif
	op_fill_status_t status;
	m_ins_pos += bytes;
	if (err.value()) {
		if (err == asio::error::eof)
		{
			m_EOF_reached = true;
		} 
		else
		{
			status.fill_result = FILL_FAILED;
			status.reason = FILL_NET_ERROR;
		}
	}
	// m_fill_callback(shared_from_this(), status);
	m_fill_callback(status);

}

size_t net_three::get_net_data()
{
	size_t bytes;
	system::error_code err_code;

	bytes = m_sock->read_some(buffer(&*m_ins_pos, free_size()), err_code);
	m_ins_pos += bytes;
	if (err_code.value())
	{
		if (err_code == asio::error::eof)
		{
			m_EOF_reached = true;
		}
	}
	return bytes;
}

} // end namespace and_net
