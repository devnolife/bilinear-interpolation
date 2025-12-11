#include <stdio.h>
#include <omp.h>

int main() {
    printf("Testing OpenMP...\n");

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        printf("Hello from thread %d\n", id);
    }

    printf("Max threads: %d\n", omp_get_max_threads());
    return 0;
}
