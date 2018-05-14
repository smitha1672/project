#include <sensor_manager.h>
#include <sensors.h>

#include <stdio.h>

struct smSensorServerInfo {
  struct SensorInfo *info;
  uint32_t rate;
  uint64_t latency;
  uint32_t handle;
  uint32_t evtType;
};

struct smSensorClientInfo {
  struct SensorInfo *info;
};

struct smSensorOrderInfo {
  uint32_t tid;
  struct smSensorServerInfo server;
  struct smSensorClientInfo client;
};

struct smSensOrderInfoNode {
  struct smSensorOrderInfo *info;
  struct smSensOrderInfoNode *next;
};

struct smSensOrderLinkedList {
  struct smSensOrderInfoNode *head;
  struct smSensOrderInfoNode *current;
  struct smSensOrderInfoNode *tail;
};

struct smSensServerTypesNode {
  struct SensorInfo *info;
  uint32_t handle;
  uint32_t event_type;
  struct smSensOrderLinkedList *list;
  struct smSensServeTypesNode *next;
};

struct smSensServerTypeLinkedList {
  struct smSensServeTypesNode *head;
  struct smSensServeTypesNode *current;
  struct smSensServeTypesNode *tail;
};

static struct smSensServerTypeLinkedList mlist = {NULL, NULL, NULL};

int smRequestSensorOrder(struct SensOrderInfo info) {
  int iret = -1;
  return iret;
}

int smAllocServerSensor(uint32_t sensorType) {
  int iret = -1;
  uint32_t handle = 0;
  const struct SensorInfo* info = NULL;

  info = sensorFind(sensorType, 0, &handle);
  if (!info) {
    printf("sensor type %d cannot be found\n", sensorType);
    goto exit;
  }

  printf("found a sensor name: %s\n", info->sensorName);
    //TODO:
    /*
      * if sensor name did not exist in smSensServerTypeLinkedList:
      *   create a smSensServerTypesNode
      *   add the smSensServerTypesNode in smSensServerTypeLinkedList
      *   return 1, 1 means the node has been added in smSensServerTypeLinkedList succesfully
      * else
      *   return 0, the node has already been added in smSensServerTypeLinkedList
    */
exit:
  return iret;
}
