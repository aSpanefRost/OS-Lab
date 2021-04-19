/*
Producers/Consumers set of threads
Group No. : 33
Group Members:
1) Aayush Prasad - 18CS30002
2) Rajdeep Das - 18CS30034
*/
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <chrono>
#include <iomanip>

using namespace std;

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
  int producer_number;
  int priority;
  int compute_time;
  int job_id;
}
Job;

/* function to initialize job with random values */
Job jobInit(int producer_number) {
  Job j;
  j.producer_number = producer_number;
  j.priority = 1 + rand() % 10;
  j.compute_time = 1 + rand() % 4;
  j.job_id = 1 + rand() % 100000;
  return j;
}

#define BUFFER_SIZE 8
#define de(x) cout << "**" << x << "**" << endl;

/* global variables to store values shared by threads */
int NC, NP, total_jobs, max_queue_size;
int job_created = 0, job_completed = 0, in = 0, out = 0;
Job buffer[BUFFER_SIZE];
sem_t empty;
sem_t full;
pthread_mutex_t mutex;

/* producer function to insert job in buffer */
void * producer(void * producer_number) {
  int p_no = * ((int * ) producer_number);
  if (job_created >= total_jobs) {
    return NULL;
  }
  /* sleeping for random number of seconds */
  float number_of_seconds = (float)(rand() % 4);
  clock_t start_time = clock();
  while (clock() < start_time + number_of_seconds * CLOCKS_PER_SEC);
  /* each thread cannot insert more than total_jobs */
  for (int iter = 0; iter < total_jobs; iter++) {

    Job item = jobInit(p_no);
    sem_wait( & empty);
    /* locks before critical sections */
    pthread_mutex_lock( & mutex);
    if (job_created >= total_jobs) {
      pthread_mutex_unlock( & mutex);
      break;
    }
    /* inserting in buffer */
    buffer[ in ] = item;
    job_created++;
    Job j = item;
    /* printing job details */
    cout << '|' << "  ";
    printElement("Produce", nameWidth);
    cout << '|' << "  ";
    printElement(j.job_id, numWidth);
    cout << '|' << "  ";
    printElement(j.priority, numWidth);
    cout << '|' << "  ";
    printElement(j.producer_number, numWidth);
    cout << '|' << "  ";
    printElement("-", numWidth);
    cout << '|' << "  ";
    printElement(j.compute_time, numWidth);
    cout << '|' << "  ";
    cout << endl; in = ( in +1) % BUFFER_SIZE;
    pthread_mutex_unlock( & mutex);
    sem_post( & full);
  }
  return producer_number;
}

/* consumer function to delete highest priority job from buffer */
void * consumer(void * consumer_number) {
  int c_no = * ((int * ) consumer_number);
  if (job_completed == total_jobs) {
    return NULL;
  }
  /* sleep for random number of seconds */
  float number_of_seconds = (float)(rand() % 4);
  clock_t start_time = clock();
  while (clock() < start_time + number_of_seconds * CLOCKS_PER_SEC);

  /* consumer cannot remove more than total_jobs jobs */
  for (int iter = 0; iter < total_jobs; iter++) {

    sem_wait( & full);
    pthread_mutex_lock( & mutex);
    if (job_completed == total_jobs) {
      pthread_mutex_unlock( & mutex);
      break;
    }
    /* finding highest priority job to execute*/
    int idx = out;
    for (int i = 1; i < (( in -out + BUFFER_SIZE) % BUFFER_SIZE); i++) {
      if (buffer[(i + out) % BUFFER_SIZE].priority > buffer[idx].priority)
        idx = (i + out) % BUFFER_SIZE;
    }
    /* shifting array to correct position */
    Job item = buffer[idx];
    int idx_pos = (idx - out + BUFFER_SIZE) % BUFFER_SIZE;
    for (int i = idx_pos; i > 0; i--) {
      buffer[(i + out) % BUFFER_SIZE] = buffer[(i - 1 + out) % BUFFER_SIZE];
    }
    Job j = item;

    /* sleeping for compute_time seconds of selected job */
    number_of_seconds = (float)(j.compute_time);
    start_time = clock();
    while (clock() < start_time + number_of_seconds * CLOCKS_PER_SEC);

    job_completed++;
    /* printing job details */
    cout << '|' << "  ";
    printElement("Consume", nameWidth);
    cout << '|' << "  ";
    printElement(j.job_id, numWidth);
    cout << '|' << "  ";
    printElement(j.priority, numWidth);
    cout << '|' << "  ";
    printElement(j.producer_number, numWidth);
    cout << '|' << "  ";
    printElement(c_no, numWidth);
    cout << '|' << "  ";
    printElement(j.compute_time, numWidth);
    cout << '|' << "  ";
    cout << endl;
    out = (out + 1) % BUFFER_SIZE;
    pthread_mutex_unlock( & mutex);
    sem_post( & empty);
  }
  return consumer_number;
}

int main() {
  /* taking input from user */
  cout << "producers: ";
  cin >> NP;
  cout << "consumers: ";
  cin >> NC;
  cout << "total jobs: ";
  cin >> total_jobs;
  cout << "max queue size: ";
  cin >> max_queue_size;
  srand(time(0));
  /* defining producer and consumer threads */
  pthread_t producers[NP], consumers[NC];
  /* initializing mutex and semaphores */
  pthread_mutex_init( & mutex, NULL);
  sem_init( & empty, 0, max_queue_size);
  sem_init( & full, 0, 0);
  /* printing column names for job details */
  cout << string(69, '-') << endl;
  cout << '|' << "  ";
  printElement("Action", nameWidth);
  cout << '|' << "  ";
  printElement("Job_ID", numWidth);
  cout << '|' << "  ";
  printElement("Job_P", numWidth);
  cout << '|' << "  ";
  printElement("P_Num", numWidth);
  cout << '|' << "  ";
  printElement("C_Num", numWidth);
  cout << '|' << "  ";
  printElement("Time", numWidth);
  cout << '|' << "  ";
  cout << endl;
  cout << string(69, '-');
  cout << endl;
  /* starting clock to record total time */
  auto begin = std::chrono::high_resolution_clock::now();

  /* array to use it's as consumer number and producer number */
  int a[10];
  for (int i = 0; i < max(NP, NC); i++)
    a[i] = i + 1;

  /* creating threads and calling srand to ensure random values in each run */
  for (int i = 1; i <= NP; i++) {
    srand(time(0) ^ i * 7);
    pthread_create( & producers[i - 1], NULL, & producer, (void * ) & a[i - 1]);
  }
  for (int i = 1; i <= NC; i++) {
    srand(time(NULL) ^ (i << 4));
    pthread_create( & consumers[i - 1], NULL, & consumer, (void * ) & a[i - 1]);
  }
  for (int i = 1; i <= NP; i++) {
    pthread_join(producers[i - 1], NULL);
  }
  for (int i = 1; i <= NC; i++) {
    pthread_join(consumers[i - 1], NULL);
  }
  /* waiting for all jobs to be created and consumed */
  while (1) {
    pthread_mutex_lock( & mutex);
    if ((job_completed) == total_jobs and(job_created) == total_jobs) {
      auto end = std::chrono::high_resolution_clock::now();
      auto consumed = std::chrono::duration_cast < std::chrono::microseconds > (end - begin);
      cout << string(69, '-');
      cout << endl;
      cout << "Total time taken for " << total_jobs << " jobs = " << (float) consumed.count() / 1000000 << " seconds." << endl;
      pthread_mutex_unlock( & mutex);
      break;
    }
    pthread_mutex_unlock( & mutex);
  }
  /* destroying mutex and semaphores after use */
  pthread_mutex_destroy( & mutex);
  sem_destroy( & empty);
  sem_destroy( & full);
  return 0;
}
