#include "lru_cache.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int hash_string(const char* str, size_t table_size) {
    int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; 
    return hash % table_size;
}

static FileInfo* create_file_info(const char* filepath) {
    FileInfo* info = malloc(sizeof(FileInfo));
    if (!info) return NULL;
    
    info->filepath = strdup(filepath);
    if (!info->filepath) {
        free(info);
        return NULL;
    }
    
    info->access_time = time(NULL);
    if (stat(filepath, &info->metadata) != 0) {
        free(info->filepath);
        free(info);
        return NULL;
    }
    
    return info;
}

static LRUCacheNode* create_cache_node(const char* filepath, int hash) {
    LRUCacheNode* node = malloc(sizeof(LRUCacheNode));
    if (!node) return NULL;
    
    node->info = create_file_info(filepath);
    if (!node->info) {
        free(node);
        return NULL;
    }
    
    node->hash = hash;
    node->prev = node->next = NULL;
    return node;
}

LRUCache* lru_create(size_t capacity, time_t expiry_duration) {
    LRUCache* cache = malloc(sizeof(LRUCache));
    if (!cache) return NULL;
    
    cache->capacity = capacity;
    cache->size = 0;
    cache->expiry_duration = expiry_duration;
    cache->head = cache->tail = NULL;
    
    cache->table_size = capacity * 2 + 1;
    cache->hash_table = calloc(cache->table_size, sizeof(LRUCacheNode*));
    if (!cache->hash_table) {
        free(cache);
        return NULL;
    }
    
    return cache;
}

void lru_free(LRUCache* cache) {
    if (!cache) return;
    
    LRUCacheNode* current = cache->head;
    while (current) {
        LRUCacheNode* next = current->next;
        free(current->info->filepath);
        free(current->info);
        free(current);
        current = next;
    }
    
    free(cache->hash_table);
    free(cache);
}

int lru_add(LRUCache* cache, const char* filepath) {
    if (!cache || !filepath) return -1;

    lru_remove_expired(cache);
    
   int hash = hash_string(filepath, cache->table_size);

    LRUCacheNode* node = cache->hash_table[hash];
    while (node) {
        if (node->hash == hash && strcmp(node->info->filepath, filepath) == 0) {
            node->info->access_time = time(NULL);
            if (stat(filepath, &node->info->metadata) != 0) {
                return -1;
            }

            if (node != cache->head) {
                if (node->prev) node->prev->next = node->next;
                if (node->next) node->next->prev = node->prev;
                if (node == cache->tail) cache->tail = node->prev;
                
                node->next = cache->head;
                node->prev = NULL;
                cache->head->prev = node;
                cache->head = node;
            }
            return 0;
        }
        node = node->next;
    }
    
    if (cache->size >= cache->capacity) {
        LRUCacheNode* lru = cache->tail;
        if (lru) {
            int lru_hash = hash_string(lru->info->filepath, cache->table_size);
            LRUCacheNode* prev = cache->hash_table[lru_hash];
            if (prev == lru) {
                cache->hash_table[lru_hash] = lru->next;
            } else {
                while (prev && prev->next != lru) prev = prev->next;
                if (prev) prev->next = lru->next;
            }
            
            cache->tail = lru->prev;
            if (cache->tail) cache->tail->next = NULL;
            free(lru->info->filepath);
            free(lru->info);
            free(lru);
            cache->size--;
        }
    }
    
    node = create_cache_node(filepath, hash);
    if (!node) return -1;
    
    node->next = cache->hash_table[hash];
    if (node->next) node->next->prev = node;
    cache->hash_table[hash] = node;
    
    node->next = cache->head;
    if (cache->head) cache->head->prev = node;
    cache->head = node;
    if (!cache->tail) cache->tail = node;
    cache->size++;
    
    return 0;
}

FileInfo* lru_search(LRUCache* cache, const char* filepath) {
    if (!cache || !filepath) return NULL;
    
    int hash = hash_string(filepath, cache->table_size);
    LRUCacheNode* node = cache->hash_table[hash];
    
    while (node) {
        if (node->hash == hash && strcmp(node->info->filepath, filepath) == 0) {
            node->info->access_time = time(NULL);
        
            if (node != cache->head) {
                if (node->prev) node->prev->next = node->next;
                if (node->next) node->next->prev = node->prev;
                if (node == cache->tail) cache->tail = node->prev;
                
                node->next = cache->head;
                node->prev = NULL;
                cache->head->prev = node;
                cache->head = node;
            }
            
            return node->info;
        }
        node = node->next;
    }
    
    return NULL;
}

int lru_remove(LRUCache* cache, const char* filepath) {
    if (!cache || !filepath) return -1;
    
    int hash = hash_string(filepath, cache->table_size);
    LRUCacheNode* node = cache->hash_table[hash];
    LRUCacheNode* prev = NULL;
    
    while (node) {
        if (node->hash == hash && strcmp(node->info->filepath, filepath) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                cache->hash_table[hash] = node->next;
            }
            if (node->next) node->next->prev = prev;
            
            if (node->prev) node->prev->next = node->next;
            if (node->next) node->next->prev = node->prev;
            if (node == cache->head) cache->head = node->next;
            if (node == cache->tail) cache->tail = node->prev;
            
            free(node->info->filepath);
            free(node->info);
            free(node);
            cache->size--;
            return 0;
        }
        prev = node;
        node = node->next;
    }
    
    return -1;
}

void lru_remove_expired(LRUCache* cache) {
    if (!cache || cache->expiry_duration == 0) return;
    
    time_t current_time = time(NULL);
    LRUCacheNode* node = cache->tail;
    
    while (node && (current_time - node->info->access_time) > cache->expiry_duration) {
        LRUCacheNode* to_remove = node;
        node = node->prev;
        
       int hash = hash_string(to_remove->info->filepath, cache->table_size);
        LRUCacheNode* hash_node = cache->hash_table[hash];
        LRUCacheNode* prev = NULL;
        
        while (hash_node && hash_node != to_remove) {
            prev = hash_node;
            hash_node = hash_node->next;
        }
        
        if (hash_node) {
            if (prev) {
                prev->next = hash_node->next;
            } else {
                cache->hash_table[hash] = hash_node->next;
            }
            if (hash_node->next) hash_node->next->prev = prev;
        }
        
        cache->tail = node;
        if (node) node->next = NULL;
        if (!node) cache->head = NULL;
        
        free(to_remove->info->filepath);
        free(to_remove->info);
        free(to_remove);
        cache->size--;
    }
}