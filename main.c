#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>

int main(){
    printf("enter file name:");
    char filename[256];
    if (scanf("%s", filename) <= 0){
        printf("input format error\n");
        return -1;
    }

    char pipeName[] = "pipe";
    char mutexName[] = "mutex";
    char mutex2Name[] = "mutex2";
    char pipe1SizeName[] = "size";
    int pipe1 = shm_open(pipeName, O_RDWR | O_CREAT, S_IRWXU);
    int pipe1Size = shm_open(pipe1SizeName, O_RDWR | O_CREAT, S_IRWXU);
    int mutex1 = shm_open(mutexName, O_RDWR | O_CREAT, S_IRWXU);
    int mutex2 = shm_open(mutex2Name, O_RDWR | O_CREAT, S_IRWXU);
    if (pipe1 == -1 || pipe1Size == -1 || mutex1 == -1 || mutex2 == -1){
        perror("shm_open\n");
        return -1;
    }

    if (ftruncate(pipe1, getpagesize()) == -1){
        perror("ftruncate error\n");
        return -1;
    }
    if (ftruncate(pipe1Size, sizeof(int)) == -1){
        perror("ftruncate error\n");
        return -1;
    }
    if (ftruncate(mutex1, sizeof(pthread_mutex_t)) == -1){
        perror("ftruncate error\n");
        return -1;
    }
    if (ftruncate(mutex2, sizeof(pthread_mutex_t)) == -1){
        perror("ftruncate error\n");
        return -1;
    }

    float* mmfData = (float *) mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, pipe1, 0);
    int* mmfDataSize = (int *) mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, pipe1Size, 0);
    *mmfDataSize = -1;
    pthread_mutex_t* mutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutex1, 0);
    pthread_mutex_t* secondMutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutex2, 0);
    if (mmfData == MAP_FAILED || mmfDataSize == MAP_FAILED || mutex == MAP_FAILED || secondMutex == MAP_FAILED){
        perror("mmap error\n");
        return -1;
    }
    pthread_mutexattr_t mutexAttribute;
    if (pthread_mutexattr_init(&mutexAttribute) != 0){
        perror("pthread_mutexattr_init error\n");
        return -1;
    }
    if (pthread_mutexattr_setpshared(&mutexAttribute, PTHREAD_PROCESS_SHARED) != 0){
        perror("pthread_mutexattr_setpshared error\n");
        return -1;
    }
    if (pthread_mutex_init(mutex, &mutexAttribute) != 0){
        perror("pthread_mutex_init error\n");
        return -1;
    }
    if (pthread_mutex_init(secondMutex, &mutexAttribute) != 0){
        perror("pthread_mutex_init error\n");
        return -1;
    }

    if (pthread_mutex_lock(secondMutex) != 0){
        perror("pthread_mutex_lock error\n");
        return -1;
    }
    if (pthread_mutex_lock(mutex) != 0){
        perror("pthread_mutex_lock error\n");
        return -1;
    }
    int id = fork();
    if (id == -1){
        perror("fork error\n");
        return -1;
    } else if (id == 0) {
        char* argv[] = {"child", filename, mutexName, mutex2Name, pipeName, pipe1SizeName, (char *)NULL};
        if (execv("child", argv) == -1){
            perror("execl error\n");
            return -1;
        }
    } else {
        printf("insert sum terms:");
        float currentTerm;
        char c;
        printf("retard1\n");
        int cnt = 0;
        printf("retard2\n");
        while(scanf("%f%c", &currentTerm, &c) > 0){
            mmfData[cnt] = currentTerm;
            ++cnt;
            if (c == '\n'){
                *mmfDataSize = cnt;
                if (pthread_mutex_unlock(mutex) != 0){
                    perror("pthread_mutex_unlock error\n");
                    return -1;
                }
                if (pthread_mutex_lock(secondMutex) != 0){
                    perror("pthread_mutex_lock error\n");
                    return -1;
                }
                printf("retard3\n");
                cnt = 0;
            }
        }
        printf("retard4\n");
        *mmfDataSize = -2;
        if (pthread_mutex_unlock(mutex) != 0){
            perror("pthread_mutex_unlock error\n");
            return -1;
        }
    }

    if (shm_unlink(pipeName) != 0){
        perror("shm_unlink error\n");
        return -1;
    }
    if (shm_unlink(pipe1SizeName) != 0){
        perror("shm_unlink error\n");
        return -1;
    }
    if (shm_unlink(mutexName) != 0){
        perror("shm_unlink error\n");
        return -1;
    }
    if (shm_unlink(mutex2Name) != 0){
        perror("shm_unlink error\n");
        return -1;
    }

    return 0;
}