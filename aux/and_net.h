#ifndef AND_NET_H
#define AND_NET_H
#include <vector>
#include <stdexcept>
#include <boost/asio.hpp>


using namespace boost::asio;

namespace and_net
{

class net_one
{
public:
	net_one() = delete;
	net_one(const net_one &source) = delete;
	net_one(net_one &&source) = delete;
	const net_one& operator=(const net_one&) = delete;
	const net_one& operator=(net_one&&) = delete;

	explicit net_one(ip::tcp::socket *sock, size_t size = 512);
	~net_one();
	size_t unreaded_size();
	size_t free_size();
	void move_tail();
	std::string read_str(size_t to_read_size = 1);
	std::string read_str(std::string end_marker = "\r\n");
	std::string remains_str();
	bool is_EOS();

private:
	std::vector<char> m_data;
	size_t m_max_buf_size;
	std::vector<char>::iterator m_get_pos,
		m_ins_pos, m_mid_pos, m_end_pos;
	ip::tcp::socket *m_sock;
	bool m_EOF_reached;
	bool m_EOS; // end of service
};


namespace errors
{

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

} //end namespace and

#endif //end #ifndef AND_NET_H