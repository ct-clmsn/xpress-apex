//  Copyright (c) 2014 University of Oregon
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef CONCURRENCYHANDLER_HPP
#define CONCURRENCYHANDLER_HPP

#include "handler.hpp"
#include "event_listener.hpp"
#include <stack>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <boost/thread/mutex.hpp>
#include "task_identifier.hpp"

#ifdef SIGEV_THREAD_ID
#ifndef sigev_notify_thread_id
#define sigev_notify_thread_id _sigev_un._tid
#endif /* ifndef sigev_notify_thread_id */
#endif /* ifdef SIGEV_THREAD_ID */

namespace apex {

class concurrency_handler : public handler, public event_listener {
private:
  void _init(void);
  // vectors and mutex
  std::vector<std::stack<task_identifier>* > _event_stack;
  boost::mutex _vector_mutex;
  // periodic samples of stack top states
  std::vector<std::map<task_identifier, unsigned int>* > _states;
  // vector of power samples
  std::vector<double> _power_samples;
  // vector of thread cap values
  std::vector<int> _thread_cap_samples;
  std::map<std::string, std::vector<long>> _tunable_param_samples;
  // functions and mutex
  std::set<task_identifier> _functions;
  boost::mutex _function_mutex;
  int _option;
public:
  concurrency_handler (void);
  concurrency_handler (int option);
  concurrency_handler (unsigned int period);
  concurrency_handler (unsigned int period, int option);
  ~concurrency_handler (void) { };
  void on_startup(startup_event_data &data) { APEX_UNUSED(data); };
  void on_shutdown(shutdown_event_data &data);
  void on_new_node(node_event_data &data) { APEX_UNUSED(data); };
  void on_new_thread(new_thread_event_data &data);
  void on_exit_thread(event_data &data);
  bool on_start(apex_function_address function_address);
  bool on_start(std::string *timer_name);
  void on_stop(std::shared_ptr<profiler> p);
  void on_yield(std::shared_ptr<profiler> p);
  bool on_resume(apex_function_address function_address);
  bool on_resume(std::string *timer_name);
  void on_new_task(apex_function_address function_address, void * task_id) 
       { APEX_UNUSED(function_address); APEX_UNUSED(task_id); };
  void on_new_task(std::string *timer_name, void * task_id) 
       { APEX_UNUSED(timer_name); APEX_UNUSED(task_id); };
  void on_sample_value(sample_value_event_data &data) { APEX_UNUSED(data); };
  void on_periodic(periodic_event_data &data) { APEX_UNUSED(data); };
  void on_custom_event(custom_event_data &data) { APEX_UNUSED(data); };

  bool _handler(void);
  std::stack<task_identifier>* get_event_stack(unsigned int tid);
  void add_thread(unsigned int tid) ;
  void _reset(void);
  void output_samples(int node_id);
};

}

#endif // CONCURRENCYHANDLER_HPP
