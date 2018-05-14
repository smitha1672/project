#ifndef _SENSOR_MANAGER_H_
#define _SENSOR_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


struct SensServeInfo {
  uint32_t supportedRate;
  uint64_t latency;
};

struct SensClientInfo {
  uint32_t sensorType;
};

struct SensOrderInfo {
  uint32_t tid;
  struct SensServeInfo serve;
  struct SensClientInfo client;
};


int smRequestSensorOrder(struct SensOrderInfo info);

int smAllocServerSensor(uint32_t sensorType);

#endif
