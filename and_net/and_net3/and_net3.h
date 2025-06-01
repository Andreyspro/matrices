#ifndef AND_NET3_H
#define AND_NET3_H

#include <vector>
#include <stdexcept>
#include <memory>
#include <functional>
#include <string>
#include <utility>
#include <boost/asio.hpp>


using namespace boost::asio;
#define AND_NET3_EXTRAOUT
#undef AND_NET3_EXTRAOUT

namespace and_net
{

enum status_read_t
{
	READ_OK,
	READ_FAILED
};

enum status_read_reason_t
{
	NO_ERROR,
	BUFF_IS_SMALL,
	LAST_MSG_SHORTER,
	NEED_MORE_DATA
};

struct op_read_status_t
{
	 status_read_t read_result = READ_OK;
	 status_read_reason_t reason = NO_ERROR;
};

enum status_fill_t
{
	FILL_OK,
	FILL_FAILED
};

enum status_fill_reason_t
{
	FILL_NO_ERROR,
	FILL_EOF,
	FILL_NET_ERROR
};

struct op_fill_status_t
{
	 status_fill_t fill_result;
	 status_fill_reason_t reason;
};
// struct op_fill_status_t
// {
// 	 status_fill_t fill_result = FILL_OK;
// 	 status_fill_reason_t reason = FILL_NO_ERROR;
// };

typedef std::pair<std::string, op_read_status_t> result_read_str_t;

class net_three : public std::enable_shared_from_this<net_three>
{
	explicit net_three(ip::tcp::socket *sock, size_t size = 512);
	net_three() = delete;
	net_three(const net_three &source) = delete;
	net_three(net_three &&source) = delete;
	const net_three& operator=(const net_three&) = delete;
	const net_three& operator=(net_three&&) = delete;
public:
	typedef std::shared_ptr<net_three> ptr_t;
	// typedef std::function<void(ptr_t, op_status_t, /* std::string */ std::shared_ptr<std::string>)> on_read_str_f_t;
	// typedef std::function<void(ptr_t, op_status_t, std::string )> on_read_str_f_t;
	// typedef std::function<void(std::shared_ptr<net_three>, op_fill_status_t)> on_fill_buff_callback_t;
	typedef std::function<void(op_fill_status_t)> on_fill_buff_callback_t;

	static ptr_t get_new(ip::tcp::socket *sock, size_t size = 512);
	ptr_t get_ptr();
	~net_three();
	
	std::string remains_str();
	bool is_EOS() const;

	std::string read_str(size_t to_read_size = 1);
	std::string read_str(std::string end_marker = "\r\n");
	size_t get_net_data();
	result_read_str_t try_read_str(size_t);
	op_read_status_t try_read_str_sz(std::string &, size_t);
	op_read_status_t try_read_str_em(std::string &, const std::string &end_marker); // EM - END MARKER

	// async section --------------------------------
	void async_fill_buff(on_fill_buff_callback_t);
	void cleancallback();

private:
	void on_fill_data(on_fill_buff_callback_t, const boost::system::error_code &, size_t bytes);
	size_t unreaded_size() const;
	size_t free_size() const;
	size_t move_tail();

	ip::tcp::socket *m_sock;
	std::vector<char> m_data;
	size_t m_max_buf_size;
	std::vector<char>::iterator m_get_pos,
		m_ins_pos, m_mid_pos, m_end_pos;
	bool m_EOF_reached;
	bool m_EOS; // end of service
	#ifdef AND_NET3_EXTRAOUT
	std::string m_id;
	#endif
	// async section=======================================
	on_fill_buff_callback_t m_fill_callback;
	// size_t m_to_read_size;
	// on_read_str_f_t m_on_read_str_callback;
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

#endif //end #ifndef AND_NET3_H