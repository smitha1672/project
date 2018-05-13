#ifndef _SENSORS_H_
#define _SENSORS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct SensorInfo {
  const char *sensorName; /* sensors.c code does not use this */

  /* Specify a list of rates supported in sensorSetRate, using a 0 to mark the
     end of the list.

     If SENSOR_RATE_ONCHANGE is included in this list, then sensor events
     should only be sent on data changes, regardless of any underlying
     sampling rate. In this case, the sensorSendOneDirectEvt callback will be
     invoked on each call to sensorRequest() to send new clients initial data.

     If SENSOR_RATE_ONDEMAND is included in this list, then the
     sensorTriggerOndemand callback must be implemented.

     If this list contains only explicit rates in Hz, then sensorRequests with
     SENSOR_RATE_ONCHANGE or ONDEMAND will be rejected.

     If NULL, the expectation is that rate is not applicable/configurable, and
     only SENSOR_RATE_ONCHANGE or SENSOR_RATE_ONDEMAND will be accepted, but
     neither on-change semantics or on-demand support is implied. */
  const uint32_t *supportedRates;

  uint8_t sensorType;
  uint8_t numAxis; /* enum NumAxis */
  uint8_t interrupt; /* interrupt to generate to AP */
  uint8_t flags1; /* enum SensorInfoFlags1 */
  uint16_t minSamples; /* minimum host fifo size (in # of samples) */
  uint8_t biasType;
  uint8_t rawType;
  float rawScale;
};

#endif
