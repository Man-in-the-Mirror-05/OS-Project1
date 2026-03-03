#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>       // 需链接-lm

#define MIN_ARRAY_SIZE 8
#define MAX_ARRAY_SIZE 10000000

typedef struct {
    int *array;
    int left;
    int right;
    int current_depth;
    int max_depth;
} sort_params_t;

pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
int active_thread_count = 0;
int max_thread_count;

void merge(int array[], int left, int mid, int right) {
    int left_size = mid - left + 1;
    int right_size = right - mid;

    int *left_array = (int *)malloc(left_size * sizeof(int));
    int *right_array = (int *)malloc(right_size * sizeof(int));
    if (left_array == NULL || right_array == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < left_size; i++)
        left_array[i] = array[left + i];
    for (int j = 0; j < right_size; j++)
        right_array[j] = array[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < left_size && j < right_size) {
        if (left_array[i] <= right_array[j]) {
            array[k] = left_array[i];
            i++;
        } else {
            array[k] = right_array[j];
            j++;
        }
        k++;
    }

    while (i < left_size) {
        array[k] = left_array[i];
        i++;
        k++;
    }

    while (j < right_size) {
        array[k] = right_array[j];
        j++;
        k++;
    }

    free(left_array);
    free(right_array);
}

void* merge_sort(void* arg) {
    sort_params_t* params = (sort_params_t*)arg;
    int left = params->left;
    int right = params->right;
    int current_depth = params->current_depth;
    int *array = params->array;
    int max_depth = params->max_depth;

    if (left < right) {
        int mid = left + (right - left) / 2;
        
        if (current_depth < max_depth) {
            sort_params_t left_params = {array, left, mid, current_depth + 1, max_depth};
            sort_params_t right_params = {array, mid + 1, right, current_depth + 1, max_depth};
            
            pthread_t left_thread, right_thread;
            int left_thread_created = 0, right_thread_created = 0;
            
            pthread_mutex_lock(&thread_mutex);
            if (active_thread_count + 2 <= max_thread_count) {
                active_thread_count += 2;
                pthread_mutex_unlock(&thread_mutex);
                
                if (pthread_create(&left_thread, NULL, merge_sort, &left_params) == 0) {
                    left_thread_created = 1;
                } else {
                    pthread_mutex_lock(&thread_mutex);
                    active_thread_count--;
                    pthread_mutex_unlock(&thread_mutex);
                }
                
                if (pthread_create(&right_thread, NULL, merge_sort, &right_params) == 0) {
                    right_thread_created = 1;
                } else {
                    pthread_mutex_lock(&thread_mutex);
                    active_thread_count--;
                    pthread_mutex_unlock(&thread_mutex);
                }
            } else {
                pthread_mutex_unlock(&thread_mutex);
            }
            
            if (left_thread_created) {
                pthread_join(left_thread, NULL);
                pthread_mutex_lock(&thread_mutex);
                active_thread_count--;
                pthread_mutex_unlock(&thread_mutex);
            } else {
                merge_sort(&left_params);
            }
            
            if (right_thread_created) {
                pthread_join(right_thread, NULL);
                pthread_mutex_lock(&thread_mutex);
                active_thread_count--;
                pthread_mutex_unlock(&thread_mutex);
            } else {
                merge_sort(&right_params);
            }
        } else {
            merge_sort(&(sort_params_t){array, left, mid, current_depth + 1, max_depth});
            merge_sort(&(sort_params_t){array, mid + 1, right, current_depth + 1, max_depth});
        }
        
        merge(array, left, mid, right);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <MaxThreads>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    max_thread_count = atoi(argv[1]);
    if (max_thread_count < 1 || (max_thread_count & (max_thread_count - 1))) {
        fprintf(stderr, "Error: MaxThreads must be a power of 2\n");
        return EXIT_FAILURE;
    }
    
    int n;
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Error: Invalid input size\n");
        return EXIT_FAILURE;
    }

    if (n < MIN_ARRAY_SIZE || n > MAX_ARRAY_SIZE) {
        fprintf(stderr, "Error: n must be between 8 and %d\n", MAX_ARRAY_SIZE);
        return EXIT_FAILURE;
    }

    int *array = (int *)malloc(n * sizeof(int));
    if (array == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++) {
        if (scanf("%d", &array[i]) != 1) {
            fprintf(stderr, "Error: Invalid array element\n");
            free(array);
            return EXIT_FAILURE;
        }
    }
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    int max_depth = (int)(log2(max_thread_count));
    sort_params_t params = {array, 0, n - 1, 0, max_depth};
    merge_sort(&params);
    
    gettimeofday(&end, NULL);
    
    for (int i = 0; i < n; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
    
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    
    fprintf(stderr, "Execution time: %.6f seconds\n", elapsed);
    
    free(array);
    pthread_mutex_destroy(&thread_mutex);
    return EXIT_SUCCESS;
}