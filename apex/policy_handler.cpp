//  Copyright (c) 2014 University of Oregon
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>
#include <hpx/include/runtime.hpp>

#include "apex.hpp"
#include "policy_handler.hpp"
#include "thread_instance.hpp"

#include <iostream>

#include <boost/make_shared.hpp>

namespace apex {

policy_handler::policy_handler (void) : handler() {
  //_init();
}

/*
template <typename Rep, typename Period>
policy_handler::policy_handler (duration<Rep, Period> const& period) : handler(period)
{
    _init();
}
*/

policy_handler::policy_handler (uint64_t period_microseconds) : handler(period_microseconds)
{
    _init();
}


void policy_handler::_handler(void) {
#ifdef APEX_HAVE_HPX3
  static bool _registered = false;
  if(!_registered) {
     hpx::runtime * runtime = get_hpx_runtime_ptr();
     if(runtime != nullptr) {
         bool status = runtime->register_thread("policy_handler", next_id);
         if(!status){
            std::cerr << "Error: failed to register thread." << std::endl;
         }
         std::cerr << "Periodic handler thread registered with HPX." << std::endl;
         _registered = true;
     } else {
        this->reset();
        return;
     }
  }
#endif
  periodic_event_data data;
  this->on_periodic(data);
  this->reset();
}

void policy_handler::_init(void) {
  next_id = 0;
  _timer.async_wait(boost::bind(&policy_handler::_handler, this));
  run();
  return;
}

inline void policy_handler::reset(void) {
  if (!_terminate) {
    _timer.expires_at(_timer.expires_at() + boost::posix_time::microseconds(_period));
    _timer.async_wait(boost::bind(&policy_handler::_handler, this));
  }
}

int policy_handler::register_policy(const apex_event_type & when,
    std::function<bool(apex_context const&)> f) {
  int id = next_id++;
  boost::shared_ptr<policy_instance> instance(
    boost::make_shared<policy_instance>(id, f));
    switch(when) {
      case STARTUP: {
        boost::unique_lock<mutex_type> l(startup_mutex);
        startup_policies.push_back(instance);
        break;
      }
      case SHUTDOWN: {
        boost::unique_lock<mutex_type> l(shutdown_mutex);
        shutdown_policies.push_back(instance);
        break;
      }
      case NEW_NODE: {
        boost::unique_lock<mutex_type> l(new_node_mutex);
        new_node_policies.push_back(instance);
        break;
      }
      case NEW_THREAD: {
        boost::unique_lock<mutex_type> l(new_thread_mutex);
        new_thread_policies.push_back(instance);
        break;
      }
      case START_EVENT: {
        boost::unique_lock<mutex_type> l(start_event_mutex);
        start_event_policies.push_back(instance);
        break;
      }
      case RESUME_EVENT: {
        boost::unique_lock<mutex_type> l(resume_event_mutex);
        resume_event_policies.push_back(instance);
        break;
      }
      case STOP_EVENT: {
        boost::unique_lock<mutex_type> l(stop_event_mutex);
        stop_event_policies.push_back(instance);
        break;
      }
      case SAMPLE_VALUE: {
        boost::unique_lock<mutex_type> l(sample_value_mutex);
        sample_value_policies.push_back(instance);
        break;
      }
      case PERIODIC: {
        boost::unique_lock<mutex_type> l(periodic_mutex);
        periodic_policies.push_back(instance);
        break;
      }
  }
  return id;

}

inline void policy_handler::call_policies(
    const std::list<boost::shared_ptr<policy_instance> > & policies,
    event_data &event_data) {
  for(const boost::shared_ptr<policy_instance>& policy : policies) {
    apex_context my_context;
    my_context.event_type = event_data.event_type_;
    my_context.policy_handle = NULL;
    const bool result = policy->func(my_context);
    if(!result) {
      printf("Warning: registered policy function failed!\n");
    }
  }
}

void policy_handler::on_startup(startup_event_data &event_data) {
  if (_terminate) return;
            if (startup_policies.empty())
                return;
        call_policies(startup_policies, event_data);
}

void policy_handler::on_shutdown(shutdown_event_data &event_data) {
  if (_terminate) return;
        _terminate = true;
            if (shutdown_policies.empty())
                return;
        call_policies(shutdown_policies, event_data);
}

void policy_handler::on_new_node(node_event_data &event_data) {
  if (_terminate) return;
            if (new_node_policies.empty())
                return;
        call_policies(new_node_policies, event_data);
}

void policy_handler::on_new_thread(new_thread_event_data &event_data) {
  if (_terminate) return;
            if (new_thread_policies.empty())
                return;
        call_policies(new_thread_policies, event_data);
}

void policy_handler::on_start(apex_function_address function_address, string *timer_name) {
  if (_terminate) return;
  if (start_event_policies.empty()) return;
        //call_policies(start_event_policies, event_data);
  for(const boost::shared_ptr<policy_instance>& policy : start_event_policies) {
    apex_context my_context;
    my_context.event_type = START_EVENT;
    my_context.policy_handle = NULL;
    const bool result = policy->func(my_context);
    if(!result) {
      printf("Warning: registered policy function failed!\n");
    }
  }
}

void policy_handler::on_stop(profiler *p) {
  if (_terminate) return;
            if (stop_event_policies.empty())
                return;
        //call_policies(stop_event_policies, event_data);
  for(const boost::shared_ptr<policy_instance>& policy : stop_event_policies) {
    apex_context my_context;
    my_context.event_type = STOP_EVENT;
    my_context.policy_handle = NULL;
    const bool result = policy->func(my_context);
    if(!result) {
      printf("Warning: registered policy function failed!\n");
    }
  }
}

void policy_handler::on_resume(profiler * p) {
  if (_terminate) return;
}

void policy_handler::on_sample_value(sample_value_event_data &event_data) {
  if (_terminate) return;
            if (sample_value_policies.empty())
                return;
        call_policies(sample_value_policies, event_data);
}

void policy_handler::on_periodic(periodic_event_data &event_data) {
  if (_terminate) return;
            if (periodic_policies.empty())
                return;
        call_policies(periodic_policies, event_data);
}

} // end namespace apex

