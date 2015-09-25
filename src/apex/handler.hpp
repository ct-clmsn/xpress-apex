//  Copyright (c) 2014 University of Oregon
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef APEX_HANDLER_H
#define APEX_HANDLER_H

#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace apex {

class handler
{
private:
  static boost::asio::io_service _io;
  static const unsigned int default_period = 100000;
  void _threadfunc(void) {
      _io.run();
  };
protected:
  unsigned int _period;
  bool _terminate;
  boost::asio::deadline_timer _timer;
  boost::thread* _timer_thread;
  void reset(void) {
      if (!_terminate) {
        _timer.expires_at(_timer.expires_at() + boost::posix_time::microseconds(_period));
        _timer.async_wait(boost::bind(&handler::_handler, this));
      }
  };
  void run(void) {
    _timer_thread = new boost::thread(&handler::_threadfunc, this);
  };
public:
  handler() : 
      _period(default_period), 
      _terminate(false), 
      _timer(_io, boost::posix_time::microseconds(_period)),
      _timer_thread(nullptr)
    { }
  handler(unsigned int period) : 
      _period(period), 
      _terminate(false), 
      _timer(_io, boost::posix_time::microseconds(_period)), 
      _timer_thread(nullptr)
    { }
  // virtual destructor
  virtual ~handler() {
      _terminate = true; 
      if(_timer_thread != nullptr) {
        _timer.cancel();
        if (_timer_thread->try_join_for(boost::chrono::seconds(1))) {
            _timer_thread->interrupt();
        }
        delete(_timer_thread);
        _timer_thread = nullptr;
      }
  };
  // all methods in the interface that a handler has to override
  virtual bool _handler(void) {
      std::cout << "Default handler" << std::endl;
      this->reset();
      return true;
  };
};

}

#endif // APEX_HANDLER_H
