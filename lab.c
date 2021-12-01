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

static pthread_mutex_t mutexes[count_of_mutexes];
bool readiness = false;

int checkOfErrors (int result_of_action, char *info_about_error) {
    if (result_of_action != SUCCESS) {
        perror(info_about_error);
        return EXIT_FAILURE;
    }
    return SUCCESS;
}
void destroyOfMutexes (int count_of_mtxs) {
    for (int i = 0; i < count_of_mtxs; i++) {
        errno = pthread_mutex_destroy(&mutexes[i]);
        checkOfErrors(errno, "Error of destroying of mutexes");
    }
}

int initializeOfMutexes () {
    pthread_mutexattr_t mattr;

    checkOfErrors(errno, "Error of initialization of attributes of mutexes");

    errno = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
    checkOfErrors(errno, "Error of creation of attributes of mutexes");

    for (int i = 0; i < count_of_mutexes; i++) {
        errno = pthread_mutex_init(&mutexes[i], &mattr);
        int result_of_init = checkOfErrors(errno, "Error of initialization of mutexes");
        if (result_of_init != SUCCESS) {
            destroyOfMutexes(count_of_mutexes);
            exit(EXIT_FAILURE);
        }
    }

    return SUCCESS;
}

int lockOfMutex (int number_of_mtx) {
    errno = pthread_mutex_lock(&mutexes[number_of_mtx]);
    checkOfErrors(errno, msg_about_error_of_lock_mtx);
    return SUCCESS;
}

int unlockOfMutex (int number_of_mtx) {
    errno = pthread_mutex_unlock(&mutexes[number_of_mtx]);
    checkOfErrors(errno, msg_about_error_of_unlock_mtx);
    return SUCCESS;
}

void *printText (void* whois) {
    char *string = (char*)whois;
    int current_mutex = 0,
        next_mutex = 0;

    if (!readiness) {
        current_mutex = 2;

        errno = lockOfMutex(current_mutex);
        checkOfErrors(errno, msg_about_error_of_lock_mtx);
        readiness = true;
    }

    for (int i = 0; i < count_of_lines; i++) {
        next_mutex = (current_mutex + 1) % count_of_mutexes;
        errno = lockOfMutex(next_mutex);
        checkOfErrors(errno, msg_about_error_of_lock_mtx);
        printf("%s %d\n", string, i++);
        errno = unlockOfMutex(current_mutex);
        checkOfErrors(errno, msg_about_error_of_unlock_mtx);
        current_mutex = next_mutex;
    }

    errno = unlockOfMutex(current_mutex);
    checkOfErrors(errno, msg_about_error_of_unlock_mtx);
    readiness = false;
}


int main (int  argc, char *argv[]) {
    pthread_t id_of_thread;
    char *text_of_parent = "Parent: ";
    char *text_of_child = "Child: ";
    int first_mutex = 0;

    errno = initializeOfMutexes();
    checkOfErrors(errno, "Error of initialization of attributes of mutexes");

    errno = lockOfMutex(first_mutex);
    int result_of_lock = checkOfErrors(errno, msg_about_error_of_lock_mtx);
    if (result_of_lock != SUCCESS) {
        destroyOfMutexes(count_of_mutexes);
        exit(EXIT_FAILURE);
    }

    errno = pthread_create(&id_of_thread, NULL, printText, text_of_child);
    int result_of_creating = checkOfErrors(errno, "Error of creating of thread");
    if (result_of_creating != SUCCESS) {
        destroyOfMutexes(count_of_mutexes);
        exit(EXIT_FAILURE);
    }

    while (readiness != true) {}

    printText(text_of_parent);

    errno = pthread_join(id_of_thread, NULL);
    int result_of_joining = checkOfErrors(errno, "Error of joining of thread");
    if (result_of_joining != SUCCESS) {
        destroyOfMutexes(count_of_mutexes);
        exit(EXIT_FAILURE);
    }

    destroyOfMutexes(count_of_mutexes);
    return SUCCESS;
}
