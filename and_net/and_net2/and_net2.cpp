#include <iostream>
#include <iterator>
#include <stdexcept>
#include <boost/asio.hpp>
#include "and_net2.h"
#include "uuid.h"

#define NOEXTRAOUT

using namespace boost;

namespace and_net
{

net_two::net_two(ip::tcp::socket *sock, size_t size)
	: m_sock(sock), m_data(size * 2),m_max_buf_size(size),
	m_EOF_reached(false), m_EOS(false),  m_to_read_size(0), 
	m_on_read_str_callback(nullptr)
{
	// BUFFER
	// > tail data space <---- |m_mid_pos| ----> main data space <
	// m_data.resize(size * 2);
	m_mid_pos = m_get_pos = m_ins_pos = m_data.begin() + size;
}

net_two::~net_two()
{
}

size_t net_two::unreaded_size()
{
	return m_get_pos < m_ins_pos ? std::distance(m_get_pos, m_ins_pos) : 0;
}

size_t net_two::free_size()
{
	return m_ins_pos < m_data.cend() ? std::distance(m_ins_pos, m_data.end()) : 0;
}

size_t net_two::move_tail()
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

std::string net_two::read_str(size_t to_read_size)
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

std::string net_two::read_str(std::string end_marker)
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

std::string net_two::remains_str()
{
	return read_str(unreaded_size());
}

bool net_two::is_EOS()
{
	return m_EOS;
}

// async read data by the size
void net_two::async_read_str_sz(size_t to_read_size, on_read_str_f_t on_read_str_f)
{
	#ifdef EXTRAOUT
	std::string from = "From AND_NET::async_read_str_sz(" + new_uuid() + "): ";
	std::cout << from << "Begin >>>>>>>" << std::endl;
	#endif
	m_to_read_size = to_read_size;
	m_on_read_str_callback = on_read_str_f;

	on_read_more_data_by_sz(system::error_code(), 0);
	#ifdef EXTRAOUT
	std::cout << from << "Finish <<<<<<<" << std::endl;
	#endif
}

// async continue read data by the size
void net_two::on_read_more_data_by_sz(const boost::system::error_code &err, size_t bytes)
{
	#ifdef EXTRAOUT
	std::string from = "From AND_NET::on_read_more_data_by_sz(" + new_uuid() + "): ";
	std::cout << from << "Begin. bytes = '" << bytes << "', err = '" << err.value() << "' >>>>>>>" << std::endl;
	#endif
	m_ins_pos += bytes;
	op_status_t op_status;
	// std::shared_ptr<std::string> out_str_ptr;
	std::string out_str_ptr;
	m_EOF_reached = (err == asio::error::eof);
	#ifdef EXTRAOUT
	if (m_EOF_reached)
	{
		std::cout << from << "EOF reached" << std::endl;
	}
	#endif
	if (m_to_read_size <= unreaded_size())
	{
		// out_str_ptr.reset(new std::string(m_get_pos, m_get_pos + m_to_read_size));
		out_str_ptr = std::string(m_get_pos, m_get_pos + m_to_read_size);
		m_get_pos += m_to_read_size;
	}
	else
	{
		if (!err) {
			if (m_to_read_size <= m_max_buf_size)
			{
				move_tail();
				#ifdef EXTRAOUT
				std::cout << from << "Start invoke async_read_some" << std::endl;
				std::cout << from << "free_size()=" << free_size() << std::endl;
				#endif
				m_sock->async_read_some
				(
					buffer(&*m_ins_pos, free_size()), 
					std::bind
					(
						&net_two::on_read_more_data_by_sz, shared_from_this(), 
						std::placeholders::_1, std::placeholders::_2
					)
				);
				#ifdef EXTRAOUT
				std::cout << from << "End invoke async_read_some" << std::endl;
				#endif
				#ifdef EXTRAOUT
				std::cout << from << "Finish <<<<<<<" << std::endl;
				#endif
				return;
			}
			else
			{
				op_status.read_status = READ_FAILED;
				op_status.reason = errors::BUFF_IS_SMALL;
			}	
		}
		else
		{
			op_status.read_status = READ_FAILED;
			op_status.reason = err == error::eof ? errors::LAST_MSG_SHORTER : errors::NET_ERROR;
		}
	}

	m_EOS = (m_EOF_reached && unreaded_size() == 0);
	#ifdef EXTRAOUT
	std::cout << from << "run m_on_read_str_callback " << std::endl;
	#endif
	// m_on_read_str_callback(shared_from_this(), op_status, out_str_ptr);
	m_on_read_str_callback(shared_from_this(), op_status, std::move(out_str_ptr));
	#ifdef EXTRAOUT
	std::cout << from << "after run m_on_read_str_callback " << std::endl;
	#endif
	#ifdef EXTRAOUT
	std::cout << from << "Finish <<<<<<<" << std::endl;
	#endif
}

// async read string until the marker is not found
void net_two::async_read_str(/* std::shared_ptr<std::string> out_str_ptr, */ std::string end_marker, 
	on_read_str_f_t on_read_str_f)
{
	// m_out_str_ptr = out_str_ptr;
	m_end_marker = end_marker;
	m_on_read_str_callback = on_read_str_f;

/* 	std::vector<char>::iterator search_pos = std::search(m_get_pos, 
		m_ins_pos, end_marker.cbegin(), end_marker.cend());
	if (search_pos < m_ins_pos) 
	{
		size_t str_sz = std::distance(m_get_pos, search_pos);
		*m_out_str_ptr = std::string(m_get_pos, m_get_pos + str_sz);
		m_get_pos += str_sz + m_end_marker.length();
		m_on_read_str_callback(READ_OK, m_out_str_ptr);
	}
	else
	{ */
		on_read_more_data_by_EM(system::error_code(), 0);
	// }
}



void net_two::on_read_more_data_by_EM(const boost::system::error_code &err, size_t bytes)
{
	#ifdef EXTRAOUT
	std::cout << "on_read_more_data_by_EM. bytes = '" << bytes << "', err = '" << err.value() << "'\n";
	#endif
	m_ins_pos += bytes;

	if (err.value())
	{
		if (err.value() == asio::error::eof)
		{
			m_EOF_reached = true;
		} 
		else
		{
			throw system::error_code(err);
		}
	}

	std::vector<char>::iterator search_pos = std::search(m_get_pos, 
		m_ins_pos, m_end_marker.cbegin(), m_end_marker.cend());
	if (search_pos < m_ins_pos) 
	{
		size_t str_sz = std::distance(m_get_pos, search_pos);
		// *m_out_str_ptr = std::string(m_get_pos, m_get_pos + str_sz);
		std::shared_ptr<std::string> str(new std::string(m_get_pos, m_get_pos + str_sz));
		m_get_pos += str_sz + m_end_marker.length();
		m_EOS = (m_EOF_reached && unreaded_size() == 0);
		// m_on_read_str_callback(shared_from_this(), op_status_t(), str);
	}
	else
	{
		if (m_EOF_reached)
		{
			throw errors:: last_msg_no_EM();
		}
		
		move_tail();

		if (free_size() == 0)
		{
			throw errors::no_EM_buffer_overflowing();
		}

		#ifdef EXTRAOUT
		std::cout << "read_data. start async_read_some'\n";
		#endif
		m_sock->async_read_some
		(
			buffer(&*m_ins_pos/*static_cast<char*>(m_ins_pos)*/, free_size()), 
			std::bind
			(
				&net_two::on_read_more_data_by_EM, shared_from_this(), 
				std::placeholders::_1, std::placeholders::_2
			)
		);
	}
}

// void net_two::return_str_by_EM()
// {
// 	std::cout << "return_str_by_EM start\n";
// 	*m_out_str_ptr = std::string(m_get_pos, m_get_pos + m_to_read_size);
// 	m_get_pos += m_to_read_size;
// 	m_on_read_str_callback(READ_OK, m_out_str_ptr);	
// }

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

} //end namespace and_net
