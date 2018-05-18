#include <sensor_manager.h>
#include <sensors.h>
#include <stdio.h>
#include <string.h>

#define heapAlloc malloc
#define heapFree free
#define DEBUG_PRINT printf

struct smSensorConfigInfo {
  struct SensorInfo *info;
  uint32_t rate;
  uint64_t latency;
  uint32_t handle;
  uint32_t evtType;
};

struct smSensorOrderInfo {
  uint32_t tid;
  struct smSensorConfigInfo server;
  struct smSensorConfigInfo client;
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
  uint32_t evtType;
  struct smSensOrderLinkedList *order_list;
  struct smSensServerTypesNode *next;
};

struct smSensServerTypeLinkedList {
  struct smSensServerTypesNode *head;
  struct smSensServerTypesNode *current;
  struct smSensServerTypesNode *tail;
};

static struct smSensServerTypeLinkedList mlist = {NULL, NULL, NULL};

typedef int (*TYPE_DIFF) (const struct SensorInfo *,
    const struct SensorInfo *);

static struct smSensServerTypesNode* _smSensServerTypeLinkedListGetNote(
    struct smSensServerTypeLinkedList *list,
    TYPE_DIFF diff,
    const struct SensorInfo *info);

static int _smSensServerTypeDiff(const struct SensorInfo *a_info,
    const struct SensorInfo *b_info);

int smRequestSensorOrder(struct SensOrderInfo order_info) {
  int iret = 0;
  const struct SensorInfo *server_info = NULL;
  struct smSensServerTypesNode *type_note = NULL;
  struct smSensOrderInfoNode *order_note = NULL;
  uint32_t handle = 0;

  server_info = sensorFind(order_info.server.sensorType, 0, &handle);
  if (!server_info) {
    iret = -1;
    goto exit;
  }

  type_note = _smSensServerTypeLinkedListGetNote(&mlist,
    &_smSensServerTypeDiff,
    server_info);
  if (!type_note) {
    iret = -2;
    goto exit;
  }

exit:
  return iret;
}

typedef int (*TYPE_DIFF) (const struct SensorInfo *,
    const struct SensorInfo *);

static int _smSensServerTypeDiff(const struct SensorInfo *a_info,
    const struct SensorInfo *b_info)
{
  return (a_info->sensorType == b_info->sensorType)? 1: 0;
}

static struct smSensServerTypesNode* _smSensServerTypeLinkedListGetNote(
    struct smSensServerTypeLinkedList *list,
    TYPE_DIFF diff,
    const struct SensorInfo *info) {
  struct smSensServerTypesNode *node = (struct smSensServerTypesNode*)list->head;
  while(node != NULL) {
    if (diff(node->info, info))
      return node;
    node =(struct smSensServerTypesNode*)node->next;
  }
  return NULL;
}

#define STR_LEN(str) (strlen(str) + 1)
static void _smSensServerTypeLinkedListAddTail(
    struct smSensServerTypeLinkedList *list,
    struct smSensServerTypesNode *node) {
  if (list->head == NULL)
    list->head = node;
  else
    list->tail->next = (struct smSensServerTypesNode*)node;

  list->tail = (struct smSensServerTypesNode*)node;
}

static struct SensorInfo *_smSensorTypeFillInfo(
    const struct SensorInfo *pSrcInfo) {
  struct SensorInfo *pInfo = \
    (struct SensorInfo *)heapAlloc(sizeof(struct SensorInfo));
  if (!pInfo)
    goto err1;

  pInfo->sensorName = (char*)heapAlloc(STR_LEN(pSrcInfo->sensorName));
  if (!pInfo->sensorName) {
    goto err2;
  strcpy((char*)pInfo->sensorName, (char*)pSrcInfo->sensorName);
  pInfo->supportedRates = (uint32_t*)pSrcInfo->supportedRates;
  pInfo->sensorType = pSrcInfo->sensorType;
  pInfo->numAxis = pSrcInfo->numAxis;
  pInfo->interrupt = pSrcInfo->interrupt;
  pInfo->flags1 = pSrcInfo->flags1;
  pInfo->minSamples = pSrcInfo->minSamples;
  pInfo->biasType = pSrcInfo->biasType;
  pInfo->rawType = pSrcInfo->rawType;
  pInfo->rawScale = pSrcInfo->rawScale;

err1:
  return NULL;
err2:
  heapFree(pInfo);

  return pInfo;
}

typedef int (*DS_SER_LL) (const struct SensorInfo*);

int _smShowServerSensors(const struct SensorInfo *info) {
  DEBUG_PRINT("[show] sensor name:%s, type:%d \n",
    info->sensorName,
    info->sensorType);
  return 0;
}

int _smShowServerSensll(struct smSensServerTypeLinkedList *list,
    DS_SER_LL show)
{
  struct smSensServerTypesNode *cur = \
    (struct smSensServerTypesNode *)list->head;

  while (cur) {
    show(cur->info);
    cur = cur->next;
  }

  return 0;
}

int smListServerSensors(void) {
  _smShowServerSensll(&mlist, &_smShowServerSensors);
  return 0;
}

int smAllocServerSensor(uint32_t sensorType) {
  int iret = 0;
  uint32_t handle = 0;
  const struct SensorInfo *pSrcInfo = NULL;
  struct smSensServerTypesNode *node = NULL;

  pSrcInfo = sensorFind(sensorType, 0, &handle);
  if (!pSrcInfo) {
    DEBUG_PRINT("sensor type %d cannot be found\n", sensorType);
    iret = -1;
    goto exit;
  }

  node = \
    (struct smSensServerTypesNode*)_smSensServerTypeLinkedListGetNote(&mlist,
        &_smSensServerTypeDiff,
        pSrcInfo);
  if (node == NULL) {
    struct SensorInfo *pDestInfo = NULL;
    node = (struct smSensServerTypesNode*)heapAlloc(sizeof(struct smSensServerTypesNode));
    if (!node) {
      iret = -2;
      goto exit;
    }

    pDestInfo = _smSensorTypeFillInfo(pSrcInfo);
    if (!pDestInfo) {
      iret = -3;
      goto exit;
    }

    node->next = NULL;
    node->info = pDestInfo;
    node->evtType = sensorGetMyEventType(pSrcInfo->sensorType);
    node->handle = handle;
    _smSensServerTypeLinkedListAddTail(&mlist, node);
  } else {
    DEBUG_PRINT("[reject] sensor:%s has been placed in list\n", pSrcInfo->sensorName);
  }
exit:
  return iret;
}
