#ifndef AND_NET2_H
#define AND_NET2_H
#include <vector>
#include <stdexcept>
#include <memory>
#include <functional>
#include <boost/asio.hpp>


using namespace boost::asio;

namespace and_net
{

namespace errors
{

enum reason_t
{
	NO_ERROR,
	NET_ERROR,
	BUFF_IS_SMALL,
	LAST_MSG_SHORTER
};

class and_net_exception : public std::runtime_error
{
public:
	and_net_exception(std::string err_str);
};

class last_msg_no_EM : public and_net_exception
{
public:
	last_msg_no_EM();
};

class no_EM_buffer_overflowing: public and_net_exception
{
public:
	no_EM_buffer_overflowing();
};

class last_msg_shorter : public and_net_exception
{
public:
	last_msg_shorter();
};

class msg_longer_buffer : public and_net_exception
{
public:
	msg_longer_buffer();
};

} //end namespace errors

enum read_status_t {
	READ_OK,
	READ_FAILED
};

struct op_status_t
{
	read_status_t read_status = READ_OK;
	errors::reason_t reason = errors::NO_ERROR;
};


class net_two : public std::enable_shared_from_this<net_two>
{
public:
	typedef std::shared_ptr<net_two> ptr_t;
	// typedef std::function<void(ptr_t, op_status_t, /* std::string */ std::shared_ptr<std::string>)> on_read_str_f_t;
	typedef std::function<void(ptr_t, op_status_t, std::string )> on_read_str_f_t;
	net_two() = delete;
	net_two(const net_two &source) = delete;
	net_two(net_two &&source) = delete;
	const net_two& operator=(const net_two&) = delete;
	const net_two& operator=(net_two&&) = delete;

	explicit net_two(ip::tcp::socket *sock, size_t size = 512);
	~net_two();
	size_t unreaded_size();
	size_t free_size();
	size_t move_tail();
	std::string read_str(size_t to_read_size = 1);
	std::string read_str(std::string end_marker = "\r\n");
	std::string remains_str();
	bool is_EOS();

	// async section --------------------------------
	void async_read_str_sz(size_t to_read_size , on_read_str_f_t on_read_str_f);
	void on_read_more_data_by_sz(const boost::system::error_code &err, size_t bytes);

	void async_read_str(std::string end_marker, on_read_str_f_t on_read_str_f);
	void on_read_more_data_by_EM(const boost::system::error_code &err, size_t bytes);

private:
	ip::tcp::socket *m_sock;
	std::vector<char> m_data;
	size_t m_max_buf_size;
	std::vector<char>::iterator m_get_pos,
		m_ins_pos, m_mid_pos, m_end_pos;
	bool m_EOF_reached;
	bool m_EOS; // end of service

	// async section=======================================
	// std::shared_ptr<std::string> m_out_str_ptr;
	size_t m_to_read_size;
	on_read_str_f_t m_on_read_str_callback;

	std::string m_end_marker;
};




} //end namespace and

#endif //end #ifndef AND_NET2_H