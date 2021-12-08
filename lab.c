#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define SUCCESS 0

#define count_of_mutexes 3
#define count_of_lines 10

char* msg_about_error_of_lock_mtx = "Error of lock of mutex";
char* msg_about_error_of_unlock_mtx = "Error of unlock of mutex";

bool readiness = false;

int checkOfErrors (int result_of_action, char *info_about_error) {
    if (result_of_action != SUCCESS) {
        perror(info_about_error);
        return EXIT_FAILURE;
    }
    return SUCCESS;
}

int destroyOfMutexes (int count_of_mtxs, pthread_mutex_t *mutexes) {
    for (int i = 0; i < count_of_mtxs; i++) {
        errno = pthread_mutex_destroy(&mutexes[i]);
        checkOfErrors(errno, "Error of destroying of mutexes");
    }
    return SUCCESS;
}

int initializeOfMutexes (pthread_mutex_t *mutexes) {
    pthread_mutexattr_t mattr;
    
    errno = pthread_mutexattr_init(&mattr);
    checkOfErrors(errno, "Error of initialization of attributes of mutexes");

    errno = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
    checkOfErrors(errno, "Error of creation of attributes of mutexes");

    for (int i = 0; i < count_of_mutexes; i++) {
        errno = pthread_mutex_init(&mutexes[i], &mattr);
        int result_of_init = checkOfErrors(errno, "Error of initialization of mutexes");
        if (result_of_init != SUCCESS) {
            destroyOfMutexes(count_of_mutexes, &mutexes[i]);
            exit(EXIT_FAILURE);
        }
    }
    
    return SUCCESS;
}

int lockOfMutex (int number_of_mtx, pthread_mutex_t *mutexes) {
    errno = pthread_mutex_lock(&mutexes[number_of_mtx]);
    checkOfErrors(errno, msg_about_error_of_lock_mtx);
    return SUCCESS;
}

int unlockOfMutex (int number_of_mtx, pthread_mutex_t *mutexes) {
    errno = pthread_mutex_unlock(&mutexes[number_of_mtx]);
    checkOfErrors(errno, msg_about_error_of_unlock_mtx);
    return SUCCESS;
}

void *printText (void* whois, pthread_mutex_t *mutexes) {
    char *string = (char*)whois;
    int current_mutex = 0,
        next_mutex = 0;

    if (readiness == false) {
        current_mutex = 2;

        errno = lockOfMutex(current_mutex, &mutexes[current_mutex]);
        checkOfErrors(errno, msg_about_error_of_lock_mtx);
        
        readiness = true;
    }

    for (int i = 0; i < count_of_lines; i++) {
        next_mutex = (current_mutex + 1) % count_of_mutexes;
        
        errno = lockOfMutex(next_mutex, &mutexes[next_mutex]);
        checkOfErrors(errno, msg_about_error_of_lock_mtx);
        
        printf("%s %d\n", string, i++);
        
        errno = unlockOfMutex(current_mutex, &mutexes[current_mutex]);
        checkOfErrors(errno, msg_about_error_of_unlock_mtx);
        
        current_mutex = next_mutex;
    }

    errno = unlockOfMutex(current_mutex, &mutexes[current_mutex]);
    checkOfErrors(errno, msg_about_error_of_unlock_mtx);
    
    readiness = false;
}


int main (int  argc, char *argv[]) {
    pthread_t id_of_thread;
    pthread_mutex_t mutexes[count_of_mutexes];
    char text_of_parent[10] = "Parent: ";
    char text_of_child[10] = "Child: ";
    int first_mutex = 0;

    printf("p1\n");

    errno = initializeOfMutexes(mutexes);
    int result_of_init = checkOfErrors(errno, "Error of initialization of attributes of mutexes");
    if (result_of_init != SUCCESS) {
        exit(EXIT_FAILURE);
    }
    printf("p2\n");

    errno = lockOfMutex(first_mutex, mutexes);
    int result_of_lock = checkOfErrors(errno, msg_about_error_of_lock_mtx);
    if (result_of_lock != SUCCESS) {
        destroyOfMutexes(count_of_mutexes, mutexes);
        exit(EXIT_FAILURE);
    }

    printf("p3\n");

    errno = pthread_create(&id_of_thread, NULL, printText, text_of_child);
    int result_of_creating = checkOfErrors(errno, "Error of creating of thread");
    if (result_of_creating != SUCCESS) {
        destroyOfMutexes(count_of_mutexes, mutexes);
        exit(EXIT_FAILURE);
    }

    printf("p4\n");

    while (readiness != true) {}

    printText(text_of_parent, mutexes);

    printf("p5\n");

    errno = pthread_join(id_of_thread, NULL);
    int result_of_joining = checkOfErrors(errno, "Error of joining of thread");
    if (result_of_joining != SUCCESS) {
        destroyOfMutexes(count_of_mutexes, mutexes);
        exit(EXIT_FAILURE);
    }

    printf("p6\n");

    destroyOfMutexes(count_of_mutexes, mutexes);
    return SUCCESS;
}
