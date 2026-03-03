#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

#define MIN_ARRAY_SIZE 8
#define MAX_ARRAY_SIZE 10000000

void merge(int arr[], int left, int mid, int right) {
    int left_size = mid - left + 1;
    int right_size = right - mid;

    int *left_arr = (int *)malloc(left_size * sizeof(int));
    int *right_arr = (int *)malloc(right_size * sizeof(int));
    if (left_arr == NULL || right_arr == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < left_size; i++)
        left_arr[i] = arr[left + i];
    for (int j = 0; j < right_size; j++)
        right_arr[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < left_size && j < right_size) {
        if (left_arr[i] <= right_arr[j]) {
            arr[k] = left_arr[i];
            i++;
        } else {
            arr[k] = right_arr[j];
            j++;
        }
        k++;
    }

    while (i < left_size) {
        arr[k] = left_arr[i];
        i++;
        k++;
    }
    while (j < right_size) {
        arr[k] = right_arr[j];
        j++;
        k++;
    }

    free(left_arr);
    free(right_arr);
}

void merge_sort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Error: Invalid input format\n");
        return EXIT_FAILURE;
    }

    if (n < MIN_ARRAY_SIZE || n > MAX_ARRAY_SIZE) {
        fprintf(stderr, "Error: n must be between 8 and %d\n", MAX_ARRAY_SIZE);
        return EXIT_FAILURE;
    }

    int *arr = (int *)malloc(n * sizeof(int));
    if (arr == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++) {
        if (scanf("%d", &arr[i]) != 1) {
            fprintf(stderr, "Error: Invalid array element\n");
            free(arr);
            return EXIT_FAILURE;
        }
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);
    merge_sort(arr, 0, n - 1);
    gettimeofday(&end, NULL);

    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    
    fprintf(stderr, "Execution time: %.6f seconds\n", elapsed);

    free(arr);
    return EXIT_SUCCESS;
}