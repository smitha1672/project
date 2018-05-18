#ifndef _SENSOR_MANAGER_H_
#define _SENSOR_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct SensConfigInfo {
  uint32_t sensorType;
  uint32_t supportedRate;
  uint64_t latency;
};

struct SensOrderInfo {
  uint32_t tid;
  struct SensConfigInfo server;
  struct SensConfigInfo client;
};


int smRequestSensorOrder(struct SensOrderInfo info);

int smListServerSensors(void);
int smAllocServerSensor(uint32_t sensorType);

#endif
