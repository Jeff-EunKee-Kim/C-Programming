#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cache_data {
  int LRU;
  bool valid;
  int tag;
  char* data;
};

cache_data **cache;
char mem[1<<24];
int cache_size, associativity, block_size, set;
int tag_bits, set_bits, block_bits;

int log2(int n) {
  int r = 0;
  while (n>>=1){
    r++;
  }
  return r;
}

int get_tag(unsigned int addr) {
  return addr >> (set_bits+block_bits);
 }
int get_set(unsigned int addr) {
  return (addr << (tag_bits+8)) >> (tag_bits+block_bits+8);
 }
int get_offset(unsigned int addr) {
  return (addr << (tag_bits+set_bits+8)) >> (tag_bits+set_bits+8);
}

int check_cache(unsigned int addr) {
  int set_index = get_set(addr);
  int tag = get_tag(addr);
  for(int i = 0 ; i < associativity ; i++) {
    if(cache[set_index][i].valid && cache[set_index][i].tag == tag){
      return i;
    }
  }
  return -1;
}

void initialize_cache() {
  cache = (cache_data**) malloc (sizeof(cache_data*) * set);
  for(int i = 0 ; i < set ; i++) {
    cache[i] = (cache_data*) malloc (sizeof(cache_data) * associativity);
    for(int j = 0 ; j < associativity ; j++) {
      cache[i][j].valid = false;
      cache[i][j].tag = -1;
      cache[i][j].data = (char*) malloc (sizeof(char) * block_size);
    }
  }
}

int main(int argc, char* argv[]) {
  FILE* tracefile = fopen(argv[1], "r");
  cache_size = atoi(argv[2]);
  associativity = atoi(argv[3]); // ways
  block_size = atoi(argv[4]);
  set = (cache_size<<10)/block_size/associativity;

  set_bits = log2(set);
  block_bits = log2(block_size);
  tag_bits = 24 - set_bits - block_bits;

  initialize_cache();
  int time = 0;
  char buff[16], data[16];
  int addr, bytes;

  while(fscanf(tracefile, "%s %x %u", buff, &addr, &bytes) != EOF) {
    int set = get_set(addr);
    int offset = get_offset(addr);
    int tag = get_tag(addr);
    int res = check_cache(addr);

    if(strcmp(buff, "load") == 0) {
      if(res < 0) {
      	int a = time + 1;
      	for(int i = 0 ; i < associativity ; i++) {
      	  if(!cache[set][i].valid) {
      	    res = i;
            break;
      	  }
      	  if(a > cache[set][i].LRU) {
      	    a = cache[set][i].LRU;
      	    res = i;
      	  }
      	}
        printf("load 0x%x miss ", addr);
      	for(int i = 0 ; i < block_size ; i++){
          cache[set][res].data[i] = mem[addr - offset + i];
        }
      } else {
	       printf("load 0x%x hit ", addr);
      }
      for(int i = 0 ; i < bytes ; i++){
	       printf("%02hhx", cache[set][res].data[offset + i]);
      }
    } else {
        for(int i = 0 ; i < bytes ; i++){
          fscanf(tracefile, "%2hhx", &data[i]);
          mem[addr + i] = data[i];
        }
        if(res >= 0) {
          printf("store 0x%x hit", addr);
        	for(int i = 0 ; i < bytes ; i++){
        	  cache[set][res].data[offset + i] = data[i];
          }
        } else {
          printf("store 0x%x miss", addr);
        }
      }

    if(res >= 0) {
      cache[set][res].LRU = time;
      cache[set][res].valid = true;
      cache[set][res].tag = tag;
    }
    printf("\n");
    time++;
  }
  fclose(tracefile);
  return 0;
  // Did not free the malloc as it was not a requirement
}
