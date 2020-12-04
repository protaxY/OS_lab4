#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>

int main(int argc, char* argv[]){
    printf("enter file name:");
    char filename[256];
    scanf("%s", filename);

    char pipeName[] = "pipe";
    char mutexName[] = "mutex";
    char pipe1SizeName[] = "size";
    char condName[] = "cond";
    int pipe1 = shm_open(pipeName, O_RDWR | O_CREAT, S_IRWXU);
    int pipe1Size = shm_open(pipe1SizeName, O_RDWR | O_CREAT, S_IRWXU);
    int mutex1 = shm_open(mutexName, O_RDWR | O_CREAT, S_IRWXU);
    int cond1 = shm_open(condName, O_RDWR | O_CREAT, S_IRWXU);

    ftruncate(pipe1, getpagesize());
    ftruncate(pipe1Size, sizeof(int));
    ftruncate(mutex1, sizeof(pthread_mutex_t));
    ftruncate(cond1, sizeof(pthread_cond_t));
    float* mmfData = (float *) mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, pipe1, 0);
    int* mmfDataSize = (int *) mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, pipe1Size, 0);
    *mmfDataSize = 0;
    pthread_mutex_t* mutex = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, mutex1, 0);
    pthread_mutexattr_t mutexAttribute;
    pthread_mutexattr_init(&mutexAttribute);
    pthread_mutexattr_setpshared(&mutexAttribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutexAttribute);

    pthread_cond_t* cond = (pthread_cond_t *) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED, cond1, 0);
    pthread_condattr_t condAttribute;
    pthread_condattr_init(&condAttribute);
    pthread_condattr_setpshared(&condAttribute, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond, &condAttribute);


    int id = fork();
    if (id == -1){
        printf("fork error\n");
        return -1;
    }
    else if (id == 0){
        char* argv[] = {"child", filename, mutexName, condName, pipeName, pipe1SizeName, (char *)NULL};
        if (execv("child", argv) == -1){
            perror("execl error\n");
        }
    }
    else{
        printf("insert sum terms:");
        float currentTerm;
        char c;
        pthread_mutex_lock(mutex);
        int cnt = 0;
        printf("retard\n");
        while(scanf("%f%c", &currentTerm, &c) > 0){
            mmfData[cnt] = currentTerm;
            ++cnt;
            if (c == '\n'){
                *mmfDataSize = cnt;
                while (*mmfDataSize != -2){
                    printf("ok\n");
                    pthread_cond_wait(cond, mutex);
                }
                cnt = 0;
            }
        }
        *mmfDataSize = -1;
        pthread_mutex_unlock(mutex);
    }
    return 0;
}