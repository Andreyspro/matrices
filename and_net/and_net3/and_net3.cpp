#include <iostream>
#include <iterator>
#include <stdexcept>
#include <boost/asio.hpp>
#include "and_net3.h"
#include "uuid.h"


using namespace boost;

namespace and_net
{

net_three::net_three(ip::tcp::socket *sock, size_t size)
	: m_sock(sock), m_data(size * 2),m_max_buf_size(size),
	m_EOF_reached(false), m_EOS(false)
{
	#ifdef AND_NET3_EXTRAOUT
	m_id = new_uuid();
	std::string prefix = "net_three (" + m_id + ") net_three. ";
	std::cout << prefix << "CTOR." << std::endl;
	#endif
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
	#ifdef AND_NET3_EXTRAOUT
	std::string prefix = "net_three (" + m_id + ") ~net_three. ";
	std::cout << prefix << "DTOR." << std::endl;
	#endif
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

std::string net_three::read_str(size_t to_read_size)
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

std::string net_three::read_str(std::string end_marker)
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
	// m_fill_callback = fill_callback;
	move_tail();
	m_sock->async_read_some
	(
		buffer(&*m_ins_pos, free_size()),
		std::bind
		(
			&net_three::on_fill_data, shared_from_this(),
			fill_callback,
			std::placeholders::_1, std::placeholders::_2
		)
	);
}

void net_three::cleancallback()
{
	m_fill_callback = NULL;
}

void net_three::on_fill_data(on_fill_buff_callback_t fill_callback,
		const boost::system::error_code &err, size_t bytes)
{
	#ifdef AND_NET3_EXTRAOUT2
	std::cout << "++" << shared_from_this().use_count() << std::endl;
	#endif
	op_fill_status_t status{and_net::FILL_OK, and_net::FILL_NO_ERROR};
	m_ins_pos += bytes;
	if (err.value()) {
		if (err == asio::error::eof)
		{
			m_EOF_reached = true;
		} 
		else
		{
			#ifdef AND_NET3_EXTRAOUT
			std::string prefix = "net_three (" + m_id + ") on_fill_data. ";
			std::cout << prefix << "Error code - '" << err.value()
				<< "',error message - '" << err.message() << "'" << std::endl;
			#endif
			status.fill_result = FILL_FAILED;
			status.reason = FILL_NET_ERROR;
		}
	}
	// m_fill_callback(shared_from_this(), status);
	fill_callback(status);

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

} // end namespace and_net
