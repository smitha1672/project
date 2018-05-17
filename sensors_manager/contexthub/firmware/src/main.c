#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include <sensor_manager.h>

#define osLog(...) \
  do { \
    printf( __VA_ARGS__); \
  } while (0)

#define ARRAY_SZIE(array) (sizeof(array)/sizeof(array[0]))

const struct SensorIndex {
  char *name;
  uint32_t sensorType;
} sensor_idx[] = {
  {"Accelerometer", 1},
  {"Any Motion", 2},
  {"No Motion", 3}
};



const struct SensorIndex* get_sensor_type(uint32_t idx) {
  for (uint32_t i = 0; i < ARRAY_SZIE(sensor_idx); i++) {
    if (sensor_idx[i].sensorType == idx)
      return &sensor_idx[i];
  }

  return NULL;
}

int main(int argc, char *argv[])
{
  //char *delivery = "";
  int count = 0;
  char ch;
  const struct SensorIndex *idx = NULL;

  while ( (ch = getopt(argc, argv, "a:s")) != EOF) {
    switch(ch)
    {
      case 'a': { /*allocate server sensor*/
        idx = get_sensor_type(atoi(optarg));
        if (idx) {
          printf("[allocate] sensor name: %s, sensor type: %d\n", idx->name, idx->sensorType);
          smAllocServerSensor(idx->sensorType);
        }
        break;
      }
      case 'f': { /*free server sensor*/
        break;
      }
      case 'r': {/*request a sensor order*/
        //delivery = optarg;
        break;
      }
      case 'd': { /*detech a sensor order*/
        break;
      }
      case 's': { /*display list content*/
        smListServerSensors();
        break;
      }
      default:
        fprintf(stderr, "Unknown option: '%s'\n", optarg);
        return 1;
    }
  }

  argc -= optind;
  argv += optind;

  //if (delivery[0])
  //  printf("To be delivered %s.\n", delivery);

  //puts("Ingredients:");
  for (count = 0 ; count < argc; count++)
    puts(argv[count]);

  return 0;
}

#if 0
struct SensClientNode {
  char *realSensorName;
  uint32_t rate;
  uint64_t latency;
  uint32_t realSensorHandle;
  uint32_t evtType;
  uint32_t subscriberTid;
  char *usageVirSensorName;
  struct SensClientNode *next;
};

struct ClientLinkedList {
  struct SensClientNode *head;
  struct SensClientNode *tail;
  struct SensClientNode *current;
};

static struct ClientLinkedList realAccList = {NULL, NULL, NULL};
static struct ClientLinkedList *ptrRealAccList = &realAccList;

static void showRealAccLinkedList(struct ClientLinkedList *list)
{
  struct SensClientNode *current = ptrRealAccList->head;
  while (current != NULL) {
    printf("real sensor name: %s\n", current->realSensorName);
    printf("rate: %d\n", current->rate);
    //printf("%"PRIu64"\n", current->latency);
    printf("virtual sensor name:%s\n", current->usageVirSensorName);
    current = current->next;
  }
}

void realSensorRequest(const char *realSensorName, uint32_t rate, uint32_t latency, const char *virSensorName)
{
  struct SensClientNode *node = \
    (struct SensClientNode*)heapAlloc(sizeof(struct SensClientNode));

  node->realSensorName = (char*)heapAlloc(strlen(realSensorName) + 1);
  node->usageVirSensorName = (char*)heapAlloc(strlen(virSensorName) + 1);
  strcpy(node->realSensorName, realSensorName);
  strcpy(node->usageVirSensorName, virSensorName);
  node->rate = rate;
  node->latency = latency;
  node->next = NULL;
  if (ptrRealAccList->head == NULL)
    ptrRealAccList->head = node;
  else
    ptrRealAccList->tail->next = node;

  ptrRealAccList->tail = node;
}

int main(int argc, char *argv) {
  printf("char* sizo = %zu\n", sizeof(char*));
  realSensorRequest("acc", 1, 11, "pedo");
  realSensorRequest("acc", 2, 22, "whun");
  realSensorRequest("gyro", 3, 33, "ori");
  showRealAccLinkedList(&realAccList);
  return 0;
}
#endif

#if 0
void thread(void)
{
  int i;
  for(i=0;i<13;i++)
  {
    osLog("This is a pthread.\n");
    sleep(1);
  }
}

int main(void)
{
  pthread_t id;
  int i,ret;
  ret=pthread_create(&id,NULL,(void *) thread,NULL);
  if(ret!=0)
  {
    printf ("Create pthread error!\n");
    exit (1);
  }
  for(i=0;i<13;i++)
  {
    printf("This is the main process.\n");
    sleep(2);
  }
  pthread_join(id,NULL);//等待線程（pthread）結束
  return (0);
}
#endif

