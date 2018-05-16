#include <sensors.h>

#include <stdio.h>

#define SIZE_ARRAY(array) (sizeof(array)/sizeof(array[0]))

#define SENS_TYPE_INVALID         0
#define SENS_TYPE_ACCEL           1
#define SENS_TYPE_ANY_MOTION      2 //provided by ACCEL, nondiscardable edge trigger
#define SENS_TYPE_NO_MOTION       3 //provided by ACCEL, nondiscardable edge trigger


const struct SensorInfo* sensorFind(uint32_t sensorType,
    uint32_t idx, uint32_t *handleP) {

  const static uint32_t AccRates[] = {
    SENSOR_HZ(12.5f),
    SENSOR_HZ(25.0f),
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    SENSOR_HZ(200.0f),
    0,
  };

  const static struct SensorInfo info[] = {
    {"Accelerometer", AccRates, SENS_TYPE_ACCEL, 0, 0, 0, 0, 0, 0, 1024.0f},
    {"Any Motion", NULL, SENS_TYPE_ANY_MOTION, 0, 0, 0, 0, 0, 0, 1024.0f},
    {"No Motion", NULL, SENS_TYPE_NO_MOTION, 0, 0, 0, 0, 0, 0, 1024.0f},
  };

  for (int i = 0; i < SIZE_ARRAY(info); i++) {
    if (info[i].sensorType == sensorType) {
      if (handleP)
        *handleP = sensorGetMyEventType(info[i].sensorType);
      return &info[i];
    }
  }
  return NULL;
}
