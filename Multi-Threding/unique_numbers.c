#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>

#define MAX_THREADS 4
#define HASH_SIZE 1000000 
#define MAX_num 1000000 

typedef struct {
    int* num;
    bool* f;
    int size;
    int cnt;
} HashSet;

HashSet g_unique;
CRITICAL_SECTION cs;

typedef struct {
    FILE* file;
    long st_num;
    long end_num;
    long* num_pos;
} ThreadData;

void init(HashSet* set, int size) {
    set->num = (int*)malloc(size * sizeof(int));
    set->f = (bool*)calloc(size, sizeof(bool));
    set->size = size;
    set->cnt = 0;
    printf("size %d\n", size);
    fflush(stdout);
}

bool add(HashSet* set, int number) {
    int ind = abs(number) % set->size;
    while (set->f[ind]) {
        if (set->num[ind] == number) {
            printf("Number %d already exists\n", number);
            fflush(stdout);
            return false; 
        }
        ind = (ind + 1) % set->size; 
    }
    set->num[ind] = number;
    set->f[ind] = true;
    set->cnt++;
    printf("Inserted %d\n", number);
    fflush(stdout);
    return true;
}

void dealloc(HashSet* set) {
    free(set->num);
    free(set->f);
    printf("Freed hash set\n");
    fflush(stdout);
}

DWORD WINAPI proc(LPVOID arg) {
    ThreadData* data = (ThreadData*)arg;
    int new_num[MAX_num];
    int new_cnt = 0;

    fseek(data->file, data->num_pos[data->st_num], SEEK_SET);
    printf("Thread %ld: Starting at number %ld (file pos %ld), ending at number %ld\n",
           GetCurrentThreadId(), data->st_num, data->num_pos[data->st_num], data->end_num);
    fflush(stdout);

    int number;
    for (long i = data->st_num; i < data->end_num && fscanf(data->file, "%d", &number) == 1; i++) {
        printf("Thread %ld: Read number %d\n", GetCurrentThreadId(), number);
        fflush(stdout);
        bool is_unique = true;
        for (int j = 0; j < new_cnt; j++) {
            if (new_num[j] == number) {
                is_unique = false;
                printf("Thread %ld: Number %d is not unique locally\n", GetCurrentThreadId(), number);
                fflush(stdout);
                break;
            }
        }
        if (is_unique) {
            if (new_cnt < MAX_num) {
                new_num[new_cnt++] = number;
                printf("Thread %ld: Added %d to local unique list\n", GetCurrentThreadId(), number);
                fflush(stdout);
            } else {
                printf("Thread %ld: Local array full, cannot add %d\n", GetCurrentThreadId(), number);
                fflush(stdout);
            }
        }
    }
    printf("Thread %ld: Found %d local unique num\n", GetCurrentThreadId(), new_cnt);
    fflush(stdout);

    EnterCriticalSection(&cs);
    printf("Thread %ld: Entered critical section\n", GetCurrentThreadId());
    fflush(stdout);
    for (int i = 0; i < new_cnt; i++) {
        add(&g_unique, new_num[i]);
    }
    LeaveCriticalSection(&cs);
    printf("Thread %ld: Left critical section\n", GetCurrentThreadId());
    fflush(stdout);

    return 0;
}

int main(int argc, char* argv[]) {
    printf("Program started at %s\n", __TIME__);
    fflush(stdout);

    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        fflush(stdout);
        return 1;
    }
    printf("Input file: %s\n", argv[1]);
    fflush(stdout);

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Error opening file: %s\n", argv[1]);
        fflush(stdout);
        return 1;
    }
    printf("File %s opened successfully\n", argv[1]);
    fflush(stdout);

    long num_cnt = 0;
    long* num_pos = (long*)malloc(MAX_num * sizeof(long));
    if (!num_pos) {
        printf("Error: Memory allocation failed for num_pos\n");
        fclose(file);
        fflush(stdout);
        return 1;
    }
    printf("Allocated memory for num_pos\n");
    fflush(stdout);

    int number;
    num_pos[num_cnt++] = ftell(file);
    while (fscanf(file, "%d", &number) == 1) {
        printf("Main: Read number %d at position %ld\n", number, ftell(file));
        fflush(stdout);
        if (num_cnt < MAX_num) {
            num_pos[num_cnt++] = ftell(file);
        } else {
            printf("Main: Reached MAX_num limit (%d), stopping cnt\n", MAX_num);
            fflush(stdout);
            break;
        }
    }
    printf("Main: cnted %ld num in file\n", num_cnt);
    fflush(stdout);
    rewind(file);

    init(&g_unique, HASH_SIZE);
    InitializeCriticalSection(&cs);
    printf("Initialized critical section\n");
    fflush(stdout);

    HANDLE threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    long num_per_thread = num_cnt / MAX_THREADS;
    printf("Creating %d threads, each processing ~%ld num\n", MAX_THREADS, num_per_thread);
    fflush(stdout);

    for (int i = 0; i < MAX_THREADS; i++) {
        thread_data[i].file = file;
        thread_data[i].st_num = i * num_per_thread;
        thread_data[i].end_num = (i == MAX_THREADS - 1) ? num_cnt : (i + 1) * num_per_thread;
        thread_data[i].num_pos = num_pos;

        threads[i] = CreateThread(NULL, 0, proc, &thread_data[i], 0, NULL);
        if (!threads[i]) {
            printf("Error creating thread %d\n", i);
            fclose(file);
            free(num_pos);
            dealloc(&g_unique);
            DeleteCriticalSection(&cs);
            fflush(stdout);
            return 1;
        }
        printf("Created thread %d\n", i);
        fflush(stdout);
    }

    printf("Waiting for threads to complete\n");
    fflush(stdout);
    WaitForMultipleObjects(MAX_THREADS, threads, TRUE, INFINITE);
    printf("All threads completed\n");
    fflush(stdout);

    printf("Unique num found: %d\n", g_unique.cnt);
    printf("List of unique num:\n");
    for (int i = 0; i < g_unique.size; i++) {
        if (g_unique.f[i]) {
            printf("%d ", g_unique.num[i]);
        }
    }
    printf("\n");
    fflush(stdout);
    printf("Printed unique num\n");
    fflush(stdout);

    for (int i = 0; i < MAX_THREADS; i++) {
        CloseHandle(threads[i]);
        printf("Closed thread %d handle\n", i);
        fflush(stdout);
    }
    
    fclose(file);
    printf("Closed file\n");
    fflush(stdout);
    free(num_pos);
    printf("Freed num_pos\n");
    fflush(stdout);
    dealloc(&g_unique);
    DeleteCriticalSection(&cs);
    printf("Deleted critical section\n");
    fflush(stdout);

    return 0;
}