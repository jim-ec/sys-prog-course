#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "timer.h"
#include "thread.h"

#include "chashmap.h"


#define DEFAULT_DURATION                10000
#define DEFAULT_INITIAL                 256
#define DEFAULT_NB_THREADS              1
#define DEFAULT_RANGE                   0xFFFF
#define DEFAULT_SEED                    0
#define DEFAULT_UPDATE                  20

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

unsigned long range;
int update;
int n_buckets;
unsigned long nb_add;
unsigned long nb_remove;
unsigned long nb_contains;
unsigned long nb_found;
unsigned long nb_aborts;
unsigned int nb_threads;
unsigned long operations;
unsigned int seed;

HM* hm;

void *test(void *data)
{
	unsigned int mySeed = seed;

	long myOps = operations / nb_threads;
	long val = -1;
	int op;


	while (myOps > 0) {
		op = rand_r(&mySeed) % 100;
		if (op < update) {
			if (val == -1) {
				/* Add random value */
				val = (rand_r(&mySeed) % range) + 1;
				if(insert_item(hm, val) != 0) {
					val = -1;
				}
			} else {
				/* Remove random value */
				int ret = remove_item(hm, val);
				if(ret != 0){
					fprintf(stderr, "hashmap did not remove a value that exists\n");
					exit(1);
				}
				val = -1;
			}
		} else {
			/* Look for random value */
			long tmp = (rand_r(&mySeed) % range) + 1;
			lookup_item(hm, tmp);
		}

		myOps--;
	}

	return NULL;
}

# define no_argument        0
# define required_argument  1
# define optional_argument  2

int main(int argc, char *argv[]) {
	TIMER_T start;
	TIMER_T stop;


	struct option long_options[] = {
		// These options don't set a flag
		{"help",                      no_argument,       NULL, 'h'},
		{"duration",                  required_argument, NULL, 'd'},
		{"initial-size",              required_argument, NULL, 'i'},
		{"num-threads",               required_argument, NULL, 'n'},
		{"range",                     required_argument, NULL, 'r'},
		{"seed",                      required_argument, NULL, 's'},
		{"buckets",                   required_argument, NULL, 'b'},
		{"update-rate",               required_argument, NULL, 'u'},
		{NULL, 0, NULL, 0}
	};

	int i, c;
	long val;
	operations = 100000;
	unsigned int initial = 512;
	nb_threads = 1;
	range = 512;
	update = 0;
	n_buckets = 512;

	while(1) {
		i = 0;
		c = getopt_long(argc, argv, "hd:i:n:b:r:s:u:", long_options, &i);

		if(c == -1)
			break;

		if(c == 0 && long_options[i].flag == 0)
			c = long_options[i].val;

		switch(c) {
			case 0:
				/* Flag is automatically set */
				break;
			case 'h':
				printf("intset -- STM stress test "
						"(hash map)\n"
						"\n"
						"Usage:\n"
						"  intset [options...]\n"
						"\n"
						"Options:\n"
						"  -h, --help\n"
						"        Print this message\n"
						"  -d, --duration <int>\n"
						"        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
						"  -i, --initial-size <int>\n"
						"        Number of elements to insert before test (default=" XSTR(DEFAULT_INITIAL) ")\n"
						"  -n, --num-threads <int>\n"
						"        Number of threads (default=" XSTR(DEFAULT_NB_THREADS) ")\n"
						"  -r, --range <int>\n"
						"        Range of integer values inserted in set (default=" XSTR(DEFAULT_RANGE) ")\n"
						"  -s, --seed <int>\n"
						"        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
						"  -u, --update-rate <int>\n"
						"        Percentage of update transactions (default=" XSTR(DEFAULT_UPDATE) ")\n"
						);
				exit(0);
			case 'd':
				operations = atoi(optarg);
				break;
			case 'b':
				n_buckets = atoi(optarg);
				break;
			case 'i':
				initial = atoi(optarg);
				break;
			case 'n':
				nb_threads = atoi(optarg);
				break;
			case 'r':
				range = atoi(optarg);
				break;
			case 's':
				seed = atoi(optarg);
				break;
			case 'u':
				update = atoi(optarg);
				break;
			case '?':
				printf("Use -h or --help for help\n");
				exit(0);
			default:
				exit(1);
		}
	}

	if (seed == 0)
		srand((int)time(0));
	else
		srand(seed);

	long inserted_vals[initial];

	hm = alloc_hashmap(n_buckets);

	if (!hm) {
		fprintf(stderr, "hash map could not be allocated\n");
		exit(1);
	}


	for (i = 0; i < initial; i++) {
		val = (rand() % range) + 1;
		insert_item(hm, val);
		inserted_vals[i] = val;
	}

	for (i = 0; i < 10; i++) {
		int index = rand() % initial;
		long tmp = inserted_vals[index];
		if (lookup_item(hm, tmp) != 0) {
			free_hashmap(hm);
			fprintf(stderr, "Expected key %d to exists in hash map\n", tmp);
			exit(1);
		}
		if (lookup_item(hm, tmp + range + 10) != 1) {
			free_hashmap(hm);
			fprintf(stderr, "Expected key %d not to exists in hash map\n", tmp);
			exit(1);
		}
	}

	//print_hashmap(hm);

	thread_startup(nb_threads);

	seed = rand();
	TIMER_READ(start);

	thread_start(test, NULL);

	TIMER_READ(stop);

	//puts("done.");
	printf("%0.6lf\n", TIMER_DIFF_SECONDS(start, stop));
	fflush(stdout);

	thread_shutdown();

	print_hashmap(hm);
	free_hashmap(hm);

	return 0;
}
