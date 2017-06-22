#ifndef COMM_LIBRARY_COMM_H
#define COMM_LIBRARY_COMM_H

#include <functional>
#include <list>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/function.hpp>

namespace comm_library
{

class Comm
{
public:
  Comm();
  virtual ~Comm();
  bool init();
  void close();

  void send_bytes(const uint8_t * src, size_t len);
  void register_receive_callback(std::function<void(uint8_t)> fun);

protected:
  static constexpr size_t READ_BUFFER_SIZE = 512;
  static constexpr size_t MAX_PACKET_LEN = 256;

  virtual bool is_open() = 0;
  virtual bool do_init() = 0;
  virtual void do_close() = 0;
  virtual void do_async_read(const boost::asio::mutable_buffers_1 &buffer, boost::function<void(const boost::system::error_code&, size_t)> handler) = 0;
  virtual void do_async_write(const boost::asio::const_buffers_1 &buffer, boost::function<void(const boost::system::error_code&, size_t)> handler) = 0;

  boost::asio::io_service io_service_;

private:

  struct WriteBuffer
  {
    uint8_t data[MAX_PACKET_LEN];
    size_t len;
    size_t pos;

    WriteBuffer() : len(0), pos(0) {}

    WriteBuffer(const uint8_t * buf, uint16_t len) : len(len), pos(0)
    {
      assert(len <= MAX_PACKET_LEN); //! \todo Do something less catastrophic here
      memcpy(data, buf, len);
    }

    const uint8_t * dpos() const { return data + pos; }

    size_t nbytes() const { return len - pos; }
  };

  typedef std::lock_guard<std::recursive_mutex> mutex_lock;
  void async_read();
  void async_read_end(const boost::system::error_code& error, size_t bytes_transferred);

  void async_write(bool check_write_state);
  void async_write_end(const boost::system::error_code& error, size_t bytes_transferred);

  std::thread io_thread_;
  std::recursive_mutex mutex_;

  uint8_t read_buffer_[READ_BUFFER_SIZE];
  std::list<WriteBuffer*> write_queue_;
  bool write_in_progress_;

  std::function<void(uint8_t)> receive_callback_;
};

} // namespace comm_library

#endif // COMM_LIBRARY_COMM_H
