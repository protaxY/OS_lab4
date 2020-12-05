#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>

int main(int argc, char* argv[]){
    FILE *file = fopen(argv[1], "a");
    if (file == NULL){
        printf("fopen error\n");
        return -1;
    }

    int mutex1 = shm_open(argv[2], O_RDWR | O_CREAT, S_IRWXU);
    int mutex2 = shm_open(argv[3], O_RDWR | O_CREAT, S_IRWXU);
    int pipe1 = shm_open(argv[4], O_RDWR | O_CREAT, S_IRWXU);
    int pipe1Size = shm_open(argv[5], O_RDWR | O_CREAT, S_IRWXU);

    pthread_mutex_t* mutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutex1, 0);
    pthread_mutex_t* secondMutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutex2, 0);
    float* mmfData = (float *) mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, pipe1, 0);
    int* mmfDataSize = (int *) mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, pipe1Size, 0);

    while (*mmfDataSize != -2){
        printf("hi\n");
        pthread_mutex_lock(mutex);
        while (*mmfDataSize == -1){
            pthread_cond_wait(cond, mutex);
        }
        if (*mmfDataSize == -2){
            break;
        }
        float result = 0;
        for (int i = 0; i < *mmfDataSize; ++i){
            result += mmfData[i];
        }
        fprintf(file, "%f\n", result);
        *mmfDataSize = -1;
        pthread_mutex_unlock(secondMutex);
    }
    printf("ne loh");
    fclose(file);
    return 0;
}