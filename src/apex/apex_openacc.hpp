#include <openacc.h>
#if defined(__PGI)
#include "pgi_acc_prof.h"
#define APEX_OPENACC_LOOKUP_FUNC acc_prof_lookup
#define acc_device_radeon 8
#else
#include "acc_prof.h"
#define APEX_OPENACC_LOOKUP_FUNC acc_prof_lookup_func
#endif

// strings for OpenACC parent constructs; based on enum acc_construct_t
const char* acc_constructs[] = {
    "parallel",
    "kernels",
    "loop",
    "data",
    "enter data",
    "exit data",
    "host data",
    "atomic",
    "declare",
    "init",
    "shutdown",
    "set",
    "update",
    "routine",
    "wait",
    "runtime api",
    "serial",
};

// strings for device APIs
const char* acc_device_api_names[] = {
    "none",
    "CUDA",
    "OpenCL",
    "COI",
    "other"
};

/* Callback functions */
extern "C" {
    void acc_register_library(acc_prof_reg reg,
        acc_prof_reg unreg, APEX_OPENACC_LOOKUP_FUNC lookup);

    void apex_openacc_launch_callback(acc_prof_info* prof_info,
        acc_event_info* event_info, acc_api_info* api_info);

    void apex_openacc_other_callback(acc_prof_info* prof_info,
        acc_event_info* event_info, acc_api_info* api_info);

    void apex_openacc_data_callback(acc_prof_info* prof_info,
        acc_event_info* event_info, acc_api_info* api_info);
}
