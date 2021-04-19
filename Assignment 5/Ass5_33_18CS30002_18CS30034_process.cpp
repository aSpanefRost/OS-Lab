/*
Producers/Consumers set of processes
Group No. : 33
Group Members:
1) Aayush Prasad - 18CS30002
2) Rajdeep Das - 18CS30034
*/
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <chrono>
#include <iomanip>

using namespace std;

#define MAX_SIZE 100

/* for output formatting */
const char separator = ' ';
const int nameWidth = 10;
const int numWidth = 8;
template < typename T > void printElement(T t,
  const int & width) {
  cout << left << setw(width) << setfill(separator) << t;
}

/* job structure to store job details */
typedef struct {
  int process_id, producer_number, priority, compute_time, job_id;
}
Job;

/* shared memory wrapper */
typedef struct {
  Job jobs[MAX_SIZE];
  int max_queue_size, queue_size, job_created, job_completed;
  pthread_mutex_t lock;
}
ShmWrapper;

/* function to create job with random values */
Job createjob(int process_id, int producer_number) {
  Job j;
  j.process_id = process_id;
  j.producer_number = producer_number;
  j.priority = rand() % 10 + 1;
  j.compute_time = rand() % 4 + 1;
  j.job_id = rand() % 100000 + 1;
  return j;
}

/* function to create shared memory wrapper */
ShmWrapper * createShmWrapper(int max_queue_size, int shmid) {
  ShmWrapper * pq = (ShmWrapper * ) shmat(shmid, (void * ) 0, 0);
  pq -> queue_size = pq -> job_created = pq -> job_completed = 0;
  pq -> max_queue_size = max_queue_size;

  /* mutex for synchronization between processes */
  pthread_mutexattr_t lock_attr;
  pthread_mutexattr_init( & lock_attr);
  pthread_mutexattr_setpshared( & lock_attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init( & pq -> lock, & lock_attr);

  return pq;
}
/* function to insert job into queue */
void insert(ShmWrapper * pq, Job j) {

  pq -> queue_size++;
  (pq -> jobs)[pq -> queue_size] = j;
  return;
}

/* function to remove highest priority job from queue */
Job remove(ShmWrapper * pq) {
  if (pq -> queue_size == 0) {
    Job j;
    cout << "ERROR\n";
    return j;
  } else if (pq -> queue_size == 1) {
    pq -> queue_size--;
    return (pq -> jobs)[1];
  } else {

    int i = 1;
    for (int j = 2; j <= pq -> queue_size; j++) {
      if (pq -> jobs[i].priority < pq -> jobs[j].priority)
        i = j;
    }
    Job removedJob = pq -> jobs[i];
    for (int j = i + 1; j <= pq -> queue_size; j++) {
      pq -> jobs[j - 1] = pq -> jobs[j];
    }

    pq -> queue_size--;
    return removedJob;
  }
}

/* consumer function for executing highest-priority job in queue and removing it*/
void consumer(ShmWrapper * pq, int consumer_number, int process_id, int total_jobs) {
  for (;;) {

    if (pq -> job_completed == total_jobs) break;
    /* sleeping for random number of seconds */
    float number_of_seconds = (float)(rand() % 4);
    clock_t start_time = clock();
    while (clock() < start_time + number_of_seconds * CLOCKS_PER_SEC);

    for (;;) {
      /* using mutex for mutual exclusion from other processes*/
      pthread_mutex_lock( & pq -> lock);
      if ((pq -> job_completed) == total_jobs) {
        pthread_mutex_unlock( & pq -> lock);
        break;
      } else if ((pq -> queue_size) <= 0) {
        pthread_mutex_unlock( & pq -> lock);
        continue;
      } else {
        Job j = remove(pq);
        /* sleeping for compute_time seconds of removed job */
        number_of_seconds = (float)(j.compute_time);
        start_time = clock();
        while (clock() < start_time + number_of_seconds * CLOCKS_PER_SEC);
        /* printing details of executed job */
        cout << '|' << "  ";
        printElement("Consume", nameWidth);
        cout << '|' << "  ";
        printElement(j.job_id, numWidth);
        cout << '|' << "  ";
        printElement(j.priority, numWidth);
        cout << '|' << "  ";
        printElement(j.process_id, numWidth);
        cout << '|' << "  ";
        printElement(j.producer_number, numWidth);
        cout << '|' << "  ";
        printElement(process_id, numWidth);
        cout << '|' << "  ";
        printElement(consumer_number, numWidth);
        cout << '|' << "  ";
        printElement(j.compute_time, numWidth);
        cout << '|' << "  ";
        cout << endl;
        (pq -> job_completed) = (pq -> job_completed) + 1;

        pthread_mutex_unlock( & pq -> lock);
      }
    }
  }
  return;
}
/* producer function for inserting job into queue */
void producer(ShmWrapper * pq, int producer_number, int process_id, int total_jobs) {
  for (;;) {
    /* sleeping for random numbe of seconds */
    if (pq -> job_created >= total_jobs) break;
    float number_of_seconds = (float)(rand() % 4);
    clock_t start_time = clock();
    while (clock() < start_time + number_of_seconds * CLOCKS_PER_SEC);
    for (;;) {
      /* mutex for mutual exclusion from other processes */
      pthread_mutex_lock( & pq -> lock);

      if (pq -> job_created == total_jobs) {
        pthread_mutex_unlock( & pq -> lock);
        break;
      } else if ((pq -> queue_size) >= (pq -> max_queue_size)) {
        pthread_mutex_unlock( & pq -> lock);
        continue;
      } else {
        Job j = createjob(process_id, producer_number);

        pq -> job_created++;
        /* printing job details */
        insert(pq, j);
        cout << '|' << "  ";
        printElement("Produce", nameWidth);
        cout << '|' << "  ";
        printElement(j.job_id, numWidth);
        cout << '|' << "  ";
        printElement(j.priority, numWidth);
        cout << '|' << "  ";
        printElement(j.process_id, numWidth);
        cout << '|' << "  ";
        printElement(j.producer_number, numWidth);
        cout << '|' << "  ";
        printElement("-", numWidth);
        cout << '|' << "  ";
        printElement("-", numWidth);
        cout << '|' << "  ";
        printElement(j.compute_time, numWidth);
        cout << '|' << "  ";
        cout << endl;
        pthread_mutex_unlock( & pq -> lock);
      }
    }
  }
  return;
}

int main() {
  /* taking input from user */
  int NP, NC, total_jobs, max_queue_size;
  cout << "producers: ";
  cin >> NP;
  cout << "consumers: ";
  cin >> NC;
  cout << "total jobs: ";
  cin >> total_jobs;
  cout << "max queue size: ";
  cin >> max_queue_size;
  /* generating a random key for shmid */
  key_t key = ftok("/dev/random", 'd');
  int shmid = shmget(key, sizeof(ShmWrapper), 0700 | IPC_CREAT);
  if (shmid >= 0) {

    srand(time(0));
    ShmWrapper * pq = createShmWrapper(max_queue_size, shmid);

    auto begin = std::chrono::high_resolution_clock::now();

    pid_t pid;
    cout << endl;
    /* printing job details attributes */
    cout << string(91, '-') << endl;
    cout << '|' << "  ";
    printElement("Action", nameWidth);
    cout << '|' << "  ";
    printElement("Job_ID", numWidth);
    cout << '|' << "  ";
    printElement("Job_P", numWidth);
    cout << '|' << "  ";
    printElement("P_PID", numWidth);
    cout << '|' << "  ";
    printElement("P_Num", numWidth);
    cout << '|' << "  ";
    printElement("C_PID", numWidth);
    cout << '|' << "  ";
    printElement("C_Num", numWidth);
    cout << '|' << "  ";
    printElement("Time", numWidth);
    cout << '|' << "  ";
    cout << endl;
    cout << string(91, '-');
    cout << endl;
    /* producer process created using fork() */
    for (int i = 1; i <= NP; i++) {
      if (fork() == 0) {
        int process_id = getpid();
        /* srand to ensure random values for every run */
        srand(time(0) ^ process_id * 7);
        producer(pq, i, process_id, total_jobs);
        return 0;
      }
    }
    /* consumer process created using fork() */
    for (int i = 1; i <= NC; ++i) {
      if (fork() == 0) {
        int process_id = getpid();
        /* srand to ensure random values for every run */
        srand(time(NULL) ^ (process_id << 16));
        consumer(pq, i, process_id, total_jobs);
        return 0;
      }
    }
    /* printing time taken when all jobs are created and consumed */
    for (;;) {

      pthread_mutex_lock( & pq -> lock);
      if (!((pq -> job_completed) == total_jobs and(pq -> job_created) == total_jobs)) {
        pthread_mutex_unlock( & pq -> lock);
      } else {

        auto end = std::chrono::high_resolution_clock::now();
        auto consumed = std::chrono::duration_cast < std::chrono::microseconds > (end - begin);
        cout << string(91, '-');
        cout << endl;
        cout << "Total time taken for " << total_jobs << " jobs = " << (float) consumed.count() / 1000000 << " seconds." << endl;
        pthread_mutex_unlock( & pq -> lock);
        break;
      }

    }
  } else {
    cout << "SHARED MEMORY ALLOCATION FAILED!\n";
    exit(EXIT_FAILURE);
  }

  return 0;
}
