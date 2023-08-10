#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>


typedef struct Map HM;


//allocate a hashmap with given number of buckets
HM* alloc_hashmap(size_t n_buckets);

//free a hashamp
void free_hashmap(HM* hm);

//insert val into the hm and return 0 if successful
//return 1 otherwise, e.g., could not allocate memory
int insert_item(HM* hm, long val);

//remove val from the hm, if it exist and return 0 if successful
//return 1 if item is not found
int remove_item(HM* hm, long val);

//check if val exists in hm, return 0 if found, return 1 otherwise
int lookup_item(HM* hm, long val);

//print all elements in the hashmap as follows:
//Bucket 1 - val1 - val2 - val3 ...
//Bucket 2 - val4 - val5 - val6 ...
//Bucket N -  ...
void print_hashmap(HM* hm);


#ifdef __cplusplus
}
#endif
