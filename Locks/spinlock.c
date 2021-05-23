#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#define NUM_BUCKETS 5     // Buckets in hash table
#define NUM_KEYS 100000   // Number of keys inserted per thread
int num_threads = 1;      // Number of threads (configurable)
int keys[NUM_KEYS];

// Declaring a lock (as many as the buckets,since buckets is the shared resource for the threads)
pthread_spinlock_t spinlk[NUM_BUCKETS];

typedef struct _bucket_entry {
    int key;
    int val;
    struct _bucket_entry *next;
} bucket_entry;

bucket_entry *table[NUM_BUCKETS];

void panic(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

double now() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Inserts a key-value pair into the table
void insert(int key, int val) {
    int i = key % NUM_BUCKETS;
    bucket_entry *e = (bucket_entry *) malloc(sizeof(bucket_entry));
    if (!e) panic("No memory to allocate bucket!");
    pthread_spin_lock(&spinlk[i]);    // Acquire a lock on the resource before using it 
    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;
    pthread_spin_unlock(&spinlk[i]);     // Release the acquired lock
}

// Retrieves an entry from the hash table by key
// Returns NULL if the key isn't found in the table
bucket_entry * retrieve(int key) {
    bucket_entry *b;
    for (b = table[key % NUM_BUCKETS]; b != NULL; b = b->next) {
        if (b->key == key) return b;
    }
    return NULL;
}

void * put_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;

    // If there are k threads, thread i inserts
    //      (i, i), (i+k, i), (i+k*2)
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        insert(keys[key], tid);
    }

    pthread_exit(NULL);
}

void * get_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;
    long lost = 0;

    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        if (retrieve(keys[key]) == NULL) lost++;
    }
    printf("[thread %ld] %ld keys lost!\n", tid, lost);

    pthread_exit((void *)lost);
}

int main(int argc, char **argv) {
    long i;
    pthread_t *threads;
    double start, end;

    if (argc != 2) {
        panic("usage: ./parallel_hashtable <num_threads>");
    }
    if ((num_threads = atoi(argv[1])) <= 0) {
        panic("must enter a valid number of threads to run");
    }

    srandom(time(NULL));
    for (i = 0; i < NUM_KEYS; i++)
        keys[i] = random();

    threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
    if (!threads) {
        panic("out of memory allocating thread handles");
    }


    // Initialize spinlocks
    for(i=0; i<NUM_BUCKETS; i++){
        pthread_spin_init(&spinlk[i], 0);
    }

    // Insert keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, put_phase, (void *)i);
    }
    
    // Barrier
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end = now();
    
    printf("[main] Inserted %d keys in %f seconds\n", NUM_KEYS, end - start);
    
    // Reset the thread array
    memset(threads, 0, sizeof(pthread_t)*num_threads);

    // Retrieve keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, get_phase, (void *)i);
    }

    // Collect count of lost keys
    long total_lost = 0;
    long *lost_keys = (long *) malloc(sizeof(long) * num_threads);
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], (void **)&lost_keys[i]);
        total_lost += lost_keys[i];
    }
    end = now();

    printf("[main] Retrieved %ld/%d keys in %f seconds\n", NUM_KEYS - total_lost, NUM_KEYS, end - start);

    return 0;
}

/* Theoretical Answers
 
 Part 1:
 When data is placed into buckets, the entries are lost.
 This is because many threads are attempting to access the same shared resource - leading to race condition.
 
 Part 2: There is a change in the timings. During mutex or spinlock, no keys are lost because the insertion part is set as a critical section to prevent the race condition
 The mutex and spinlock retrieval timings are as follows:
 
 Mutex:
 1 thread: 7.848206 sec
 10 threads: 2.246299 sec
 20 threads: 2.503090 sec
 
 Spinlock:
 1 thread: 7.750382 sec
 10 threads: 2.211787 sec
 20 threads: 2.392526 sec
 
 Mutex seems to take lesser time than spinlock.
 
 Part 3:
 No locks are required to retrieve an object from the hash table, as there are no shared resources.
 The Retrieve operation runs parallel as it is and needs no changes to the code.
 
 Part 4:
 The code has been changed to run in parallel for insertion operations.
 The locks are set when the value of i = key % NUM_BUCKETS, i.e. during race condition, after acquiring a lock by a thread, the next threads wait till the lock in been released.
 But we set locks for each bucket in the hash table by changing the above code, and will therefore be able to insert them in parallel.
 
 */

