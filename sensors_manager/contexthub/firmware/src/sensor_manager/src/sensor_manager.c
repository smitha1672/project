#include <sensor_manager.h>
#include <sensors.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

#define STR_LEN(str) (strlen(str) + 1)

typedef int (*TYPE_DIFF) (const struct SensorInfo *,
    const struct SensorInfo *);

typedef int (*DS_SER_LL) (const struct SensorInfo*);

static struct smSensServerTypesNode* _smSensServerTypeLinkedListGetNote(
    struct smSensServerTypeLinkedList *list,
    TYPE_DIFF diff,
    const struct SensorInfo *info);

static int _smSensServerTypeDiff(const struct SensorInfo *a_info,
    const struct SensorInfo *b_info);

static struct SensorInfo *_smFillSensInfo(
    const struct SensorInfo *pSrcInfo);


typedef int (*ORDER_DIFF) (const struct smSensorOrderInfo *,
    const struct smSensorOrderInfo *);

static int smSensOrderDiff(const struct smSensorOrderInfo* a_order,
    const struct smSensorOrderInfo* b_order) {
  int iret = 1;

  if (a_order->tid != b_order->tid) {
    iret = 0;
    goto exit;
  }

  if (strcmp(a_order->server.info->sensorName,
          b_order->server.info->sensorName)) {
    iret = 0;
    goto exit;
  }

  if (a_order->server.rate != b_order->server.rate) {
    iret = 0;
    goto exit;
  }

  if (a_order->server.latency != b_order->server.latency) {
    iret = 0;
    goto exit;
  }

  if (a_order->server.handle != b_order->server.handle) {
    iret = 0;
    goto exit;
  }

  if (a_order->server.evtType != b_order->server.evtType) {
    iret = 0;
    goto exit;
  }

exit:
  return iret;
}

static struct smSensOrderInfoNode* _smSensOrderLinkedListGetNote(
    struct smSensOrderLinkedList *list,
    ORDER_DIFF diff,
    const struct smSensorOrderInfo *order_info) {
  struct smSensOrderInfoNode* node = NULL;

  node = (!list)? NULL: (struct smSensOrderInfoNode*)list->head;
  while (node != NULL) {
    if (diff(node->info, order_info))
      return node;
    node = (struct smSensOrderInfoNode*)node->next;
  }
  return NULL;
}

int smRequestSensorOrder(struct SensOrderInfo order_info) {
  int iret = 0;
  const struct SensorInfo *server_info = NULL;
  const struct SensorInfo *client_info = NULL;
  struct smSensOrderInfoNode *sm_order_info_node = NULL;
  struct smSensServerTypesNode *type_node = NULL;
  char *sensorName = NULL;
  uint32_t server_handle = 0, client_handle = 0;

  server_info = sensorFind(order_info.server.sensorType, 0, &server_handle);
  if (!server_info) {
    iret = -1;
    goto exit;
  }

  type_node = \
    (struct smSensServerTypesNode*)_smSensServerTypeLinkedListGetNote(&mlist,
        &_smSensServerTypeDiff,
        server_info);
  if (!type_node) {
    printf("type list does not supported %s sensor\n", server_info->sensorName);
    iret = -1;
    goto exit;
  }

  client_info = sensorFind(order_info.client.sensorType, 0, &client_handle);
  if (!client_info) {
    iret = -1;
    goto exit;
  }

  sm_order_info_node = \
    (struct smSensOrderInfoNode*)heapAlloc(sizeof(struct smSensOrderInfoNode));
  if (!sm_order_info_node) {
    iret = -1;
    goto exit;
  }

  sm_order_info_node->next = NULL;
  sm_order_info_node->info = \
    (struct smSensorOrderInfo*)heapAlloc(sizeof(struct smSensorOrderInfo));
  if (!sm_order_info_node->info) {
    heapFree(sm_order_info_node);
    iret = -1;
    goto exit;
  }
  sm_order_info_node->info->tid = order_info.tid;

  sm_order_info_node->info->server.info = _smFillSensInfo(server_info);
  if (!sm_order_info_node->info->server.info) {
    free(sm_order_info_node);
    iret = -1;
    goto exit;
  }

  sm_order_info_node->info->server.rate = order_info.server.supportedRate;
  sm_order_info_node->info->server.latency = order_info.server.latency;
  sm_order_info_node->info->server.handle = server_handle;
  sm_order_info_node->info->server.evtType = \
    sensorGetMyEventType(server_info->sensorType);
  sm_order_info_node->info->client.info = _smFillSensInfo(client_info);
  if (!sm_order_info_node->info->client.info) {
    free(sm_order_info_node);
    iret = -1;
    goto exit;
  }
  sm_order_info_node->info->client.rate = order_info.client.supportedRate;
  sm_order_info_node->info->client.latency = order_info.client.latency;
  sm_order_info_node->info->client.handle = client_handle;
  sm_order_info_node->info->client.evtType = \
    sensorGetMyEventType(client_info->sensorType);
  printf("FUNCTION: %s\n", __FUNCTION__);
  printf("server.info.sensorName: %s\n", sm_order_info_node->info->server.info->sensorName);
  printf("client.info.sensorName: %s\n", sm_order_info_node->info->client.info->sensorName);
  /*get node from order linked list*/
  if (!_smSensOrderLinkedListGetNote(type_node->order_list,
        &smSensOrderDiff,
        sm_order_info_node->info))
    printf("No anything in linked list\n");
exit:
  return iret;
}

static int _smSensServerTypeDiff(const struct SensorInfo *a_info,
    const struct SensorInfo *b_info)
{
  return (a_info->sensorType == b_info->sensorType)? 1: 0;
}

static struct smSensServerTypesNode* _smSensServerTypeLinkedListGetNote(
    struct smSensServerTypeLinkedList *list,
    TYPE_DIFF diff,
    const struct SensorInfo *info) {
  struct smSensServerTypesNode *node = NULL;

  node = (!list)? NULL: (struct smSensServerTypesNode*)list->head;
  while (node != NULL) {
    if (diff(node->info, info))
      return node;
    node =(struct smSensServerTypesNode*)node->next;
  }
  return NULL;
}

static void _smSensServerTypeLinkedListAddTail(
    struct smSensServerTypeLinkedList *list,
    struct smSensServerTypesNode *node) {
  if (list->head == NULL)
    list->head = node;
  else
    list->tail->next = (struct smSensServerTypesNode*)node;

  list->tail = (struct smSensServerTypesNode*)node;
}

static struct SensorInfo *_smFillSensInfo(
    const struct SensorInfo *pSrcInfo) {
  struct SensorInfo *pInfo = \
    (struct SensorInfo *)heapAlloc(sizeof(struct SensorInfo));
  if (!pInfo) {
    pInfo = NULL;
    goto exit;
  }

  pInfo->sensorName = (char*)heapAlloc(STR_LEN(pSrcInfo->sensorName));
  if (!pInfo->sensorName) {
    heapFree(pInfo);
    pInfo = NULL;
    goto exit;
  }
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

exit:
  return pInfo;
}

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
  if (!node) {
    struct SensorInfo *pDestInfo = NULL;
    node = (struct smSensServerTypesNode*)heapAlloc(sizeof(struct smSensServerTypesNode));
    if (!node) {
      iret = -2;
      goto exit;
    }

    pDestInfo = _smFillSensInfo(pSrcInfo);
    if (!pDestInfo) {
      heapFree(node);
      node = NULL;
      iret = -3;
      goto exit;
    }

    node->order_list= NULL;
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
