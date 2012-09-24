#include <iostream>

#include <assert.h>
#include <boost/bind.hpp>

#include "tcp_connection.hpp"

namespace paxos { namespace detail {


tcp_connection::tcp_connection (
   boost::asio::io_service & io_service)
   : socket_ (io_service)
{
}

/*! static */ tcp_connection::pointer 
tcp_connection::create (
   boost::asio::io_service &    io_service)
{
   return pointer (
      new tcp_connection (io_service));
}

boost::asio::ip::tcp::socket &
tcp_connection::socket ()
{
   return socket_;
}

void
tcp_connection::write (
   std::string const &  message)
{
   /*!
     The problem with Boost.Asio is that it assumes that the buffer we send to async_write ()
     must stay alive until the callback to handle_write () has been called. Since there is no
     way we can guarantee that if we use the message object we get as a reference to this function,
     we have no alternative but to copy the data into our local buffer.

     But wait! What happends when write() is called when we are not yet done writing another
     block? In that case, we will just append that data onto the queue. The handle_write () function
     checks whether any data is still pending on the queue, and if so, ensures another async_write ()
     is called.
    */

   if (buffer_.empty () == true)
   {
      buffer_ = message;

      start_write ();
   }
   else
   {
      buffer_ += message;
   }
}

void
tcp_connection::start_write ()
{
   assert (buffer_.empty () == false);

   boost::asio::async_write (socket_,
                             boost::asio::buffer (buffer_),
                             boost::bind (&tcp_connection::handle_write, 
                                          shared_from_this(),
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
}

void
tcp_connection::handle_write (
   boost::system::error_code const &    error,
   size_t                               bytes_transferred)
{
   buffer_ = buffer_.substr (bytes_transferred);

   /*!
     As discussed in the write () function, if we still have data on the buffer, ensure
     that data is also written.
    */
   if (buffer_.empty () == false)
   {
      start_write ();
   }
}




}; };
