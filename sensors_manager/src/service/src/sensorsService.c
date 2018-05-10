#include <inttypes.h>
#include <stdio.h>
//#include <slab.h>

typedef enum {
  REAL_SENSOR_ACC = 0,
  REAL_SENSOR_GYRO,
  REAL_ANY_MOTION,
  REAL_NO_MOTION,
  REAL_SENSOR_UNAVAILABLE
}RealSensors;

int sensorsServiceClientRequest(RealSensors sensor) {
  return 0;
}

int sensorsServiceClientRelease(RealSensors sensor) {
  return 0;
}

