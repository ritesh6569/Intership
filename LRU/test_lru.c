#include "lru_cache.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void print_file_info(FileInfo* info) {
    if (!info) {
        printf("File info not found\n");
        return;
    }
    printf("Filepath: %s\n", info->filepath);
    printf("Access timestamp: %ld\n", info->access_time);
    printf("File size: %ld bytes\n", info->metadata.st_size);
    printf("Last modified: %ld\n", info->metadata.st_mtime);
    printf("Inode: %ld\n", info->metadata.st_ino);
    printf("------------------------\n");
}

int main() {
    LRUCache* cache = lru_create(5, 10);
    if (!cache) {
        printf("Failed to create cache\n");
        return 1;
    }

    FILE* f1 = fopen("test1.txt", "w");
    FILE* f2 = fopen("test2.txt", "w");
    FILE* f3 = fopen("test3.txt", "w");
    
    if (!f1 || !f2 || !f3) {
        printf("Failed to create one or more test files\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        if (f3) fclose(f3);
        lru_free(cache);
        return 1;
    }
    
    fclose(f1);
    fclose(f2);
    fclose(f3);

    printf("Adding files to cache...\n");
    if (lru_add(cache, "test1.txt") == -1) {
        printf("Failed to add test1.txt\n");
    }
    if (lru_add(cache, "test2.txt") == -1) {
        printf("Failed to add test2.txt\n");
    }
    if (lru_add(cache, "test3.txt") == -1) {
        printf("Failed to add test3.txt\n");
    }

    printf("\nSearching for files...\n");
    FileInfo* info = lru_search(cache, "test1.txt");
    print_file_info(info);

    info = lru_search(cache, "test2.txt");
    print_file_info(info);

    printf("\nWaiting for 5 seconds...\n");
    sleep(5);

    printf("\nAdding another file...\n");
    if (lru_add(cache, "test1.txt") == -1) {
        printf("Failed to update test1.txt\n");
    }

    printf("\nCleaning up old entries...\n");
    lru_remove_expired(cache);

    printf("\nRemoving test2.txt...\n");
    if (lru_remove(cache, "test2.txt") == -1) {
        printf("Failed to remove test2.txt\n");
    }

    printf("\nSearching for removed file...\n");
    info = lru_search(cache, "test2.txt");
    print_file_info(info);

    lru_free(cache);

    unlink("test1.txt");
    unlink("test2.txt");
    unlink("test3.txt");

    return 0;
}