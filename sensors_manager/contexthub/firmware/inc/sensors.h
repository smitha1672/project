#ifndef _SENSORS_H_
#define _SENSORS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

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

/*
 * Sensor rate is encoded as a 32-bit integer as number of samples it can
 * provide per 1024 seconds, allowing representations of all useful values
 * well. This define is to be used for static values only please, as GCC
 * will convert it into a const int at compile time. Do not use this at
 * runtime please. A few Magic values exist at both ends of the range
 * 0 is used as a list sentinel and high numbers for special abilities.
 */
#define SENSOR_RATE_ONDEMAND    0xFFFFFF00UL
#define SENSOR_RATE_ONCHANGE    0xFFFFFF01UL
#define SENSOR_RATE_ONESHOT     0xFFFFFF02UL
#define SENSOR_HZ(_hz)          ((uint32_t)((_hz) * 1024.0f))

/*
 * api for using sensors (enum is not synced with sensor sub/unsub, this is ok since we do not expect a lot of dynamic sub/unsub)
 */
const struct SensorInfo* sensorFind(uint32_t sensorType, uint32_t idx, uint32_t *handleP); //enumerate all sensors of a type
bool sensorRequest(uint32_t clientTid, uint32_t sensorHandle, uint32_t rate, uint64_t latency);
bool sensorRelease(uint32_t clientTid, uint32_t sensorHandle);

#define EVT_NO_FIRST_SENSOR_EVENT        0x00000200    //sensor type SENSOR_TYPE_x produces events of type EVT_NO_FIRST_SENSOR_EVENT + SENSOR_TYPE_x for all Google-defined sensors
#define sensorGetMyEventType(_sensorType) (EVT_NO_FIRST_SENSOR_EVENT + (_sensorType))

#endif
