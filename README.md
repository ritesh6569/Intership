# LRU Cache Implementation
This project implements a Least Recently Used (LRU) Cache in C to cache file metadata, such as file size, last modified time, and inode number, for efficient access. The cache combines a doubly linked list and a hash table to achieve O(1) time complexity for key operations and includes an expiry mechanism to remove stale entries.
## Concept
The LRU Cache is a data structure that stores a fixed number of items, prioritizing the most recently accessed ones. When the cache is full, the least recently used item is evicted to accommodate new entries. This implementation caches file metadata retrieved via the stat system call, with an expiry mechanism to remove entries not accessed within a specified time.
Key Features

O(1) Operations: Uses a hash table for fast lookups and a doubly linked list to track access order.
Expiry Mechanism: Removes entries older than a specified duration.
File Metadata Storage: Stores filepath, access time, file size, last modified time, and inode number.
Memory Management: Properly handles dynamic memory allocation and deallocation.

## Algorithm
The LRU Cache is implemented with the following components and algorithms:

## Data Structures:

FileInfo: Stores file metadata (filepath, access time, struct stat).
LRUCacheNode: Represents a cache entry with FileInfo, hash value, and prev/next pointers for the doubly linked list.
LRUCache: Contains the hash table, head/tail pointers, capacity, size, and expiry duration.
Hash Table: An array of LRUCacheNode pointers, handling collisions via chaining.


## Key Operations:

Hash Function: Uses the djb2 algorithm to map filepaths to hash table indices.
Add (lru_add):
Checks for existing entries; updates metadata and moves to head if found.
Evicts the tail node if the cache is full.
Adds new nodes to the hash table and head of the linked list.

Search (lru_search):
Looks up entries by hash and filepath.
Updates access time and moves found nodes to the head.
Returns FileInfo or NULL if not found.

Remove (lru_remove):
Removes nodes from the hash table and linked list, freeing memory.

Remove Expired (lru_remove_expired):
Removes tail nodes whose access time exceeds the expiry duration.

Free (lru_free):
Deallocates all nodes, their FileInfo, and the hash table.


## Time Complexity:

Add: O(1) average case.
Search: O(1) average case.
Remove: O(1) average case.
Remove Expired: O(n) worst case, where n is the number of expired entries.


## Space Complexity:

O(n) for the hash table and linked list, where n is the capacity.
Additional space for filepaths and metadata.




# Multithreaded Unique Number Finder

This project implements a multithreaded program in C to read numbers from a file, identify unique numbers using a hash set, and output the count and list of unique numbers. It uses Windows threading APIs and a critical section for thread-safe operations. The program divides the file's numbers among multiple threads to process them concurrently, improving performance for large datasets.

## Concept

The program reads a file containing integers, splits the workload across multiple threads, and uses a hash set to track unique numbers. Each thread processes a portion of the file, collects locally unique numbers, and then adds them to a global hash set in a thread-safe manner using a critical section. The hash set uses linear probing to handle collisions, ensuring efficient storage and lookup of numbers.

## Key Features
Multithreading: Uses up to 4 threads to process the input file concurrently.
Thread Safety: Employs a critical section to synchronize access to the global hash set.
Hash Set: Implements a hash set with linear probing for O(1) average-case insertion and lookup.
Efficient File Processing: Tracks file positions to allow threads to read specific ranges of numbers.

## Algorithm

The program uses the following components and algorithms:

## Data Structures:

HashSet: Stores numbers (num) and their presence (f) in arrays, with a count (cnt) and size (size).
ThreadData: Contains a file pointer, start/end indices for the number range, and an array of file positions (num_pos).
Global HashSet: A single g_unique hash set shared across threads for storing unique numbers.
Critical Section: Ensures thread-safe updates to the global hash set.



## Key Operations:

Initialization (init):
Allocates memory for the hash set's num and f arrays.
Initializes the count and size.

Add (add):
Computes an index using the absolute value of the number modulo the hash set size.
Uses linear probing to resolve collisions.
Returns false if the number exists; otherwise, inserts the number and returns true.

Thread Processing (proc):
Each thread reads a portion of the file, starting at a specific position (num_pos[st_num]).
Collects locally unique numbers in a temporary array (new_num).
Adds locally unique numbers to the global hash set within a critical section.

Deallocation (dealloc):
Frees the hash set's arrays.



## Main Program:

Reads the input file to count numbers and store their file positions.
Divides the numbers among threads (up to MAX_THREADS).
Creates and waits for threads to complete.
Prints the count and list of unique numbers.



## Time Complexity:

Hash Set Operations: O(1) average case for insertion and lookup (linear probing).
File Reading: O(n) for reading n numbers.
Thread Processing: O(n/t) per thread, where n is the number of numbers and t is the number of threads.
Critical Section: O(k) for adding k locally unique numbers to the global hash set.



## Space Complexity:

O(n) for the hash set (num and f arrays) and file position array (num_pos).
O(k) per thread for the local unique number array (new_num), where k is the number of locally unique numbers.
