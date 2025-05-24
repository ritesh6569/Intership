#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <time.h>
#include <sys/stat.h>

typedef struct {
    char* filepath;          
    time_t access_time;      
    struct stat metadata;    
} FileInfo;

typedef struct LRUCacheNode {
    FileInfo* info;
    struct LRUCacheNode* prev;
    struct LRUCacheNode* next;
    int hash;       
} LRUCacheNode;


typedef struct {
    size_t capacity;        
    size_t size;             
    time_t expiry_duration; 
    LRUCacheNode** hash_table; 
    LRUCacheNode* head;     
    LRUCacheNode* tail;    
    size_t table_size;      
} LRUCache;

LRUCache* lru_create(size_t capacity, time_t expiry_duration);

void lru_free(LRUCache* cache);

int lru_add(LRUCache* cache, const char* filepath);

FileInfo* lru_search(LRUCache* cache, const char* filepath);

int lru_remove(LRUCache* cache, const char* filepath);

void lru_remove_expired(LRUCache* cache);

#endif