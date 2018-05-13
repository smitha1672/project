#ifndef _SENSOR_MANAGER_H_
#define _SENSOR_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct SensServeInfo {
  char *name;
  uint32_t rate;
  uint64_t latency;
  uint32_t handle;
  uint32_t evtType;
};

struct SensClientInfo {
  char *name;
  uint32_t handle;
  uint32_t tid;
};

struct SensOrderInfo {
  struct SensServeInfo serve;
  struct SensClientInfo client;
};

#endif
