#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define SUCCESS 0

#define count_of_mutexes 3
#define count_of_lines 10
#define time_for_sleep 1
#define size_of_string 10

char* msg_about_error_of_lock_mtx = "Error of lock of mutex";
char* msg_about_error_of_unlock_mtx = "Error of unlock of mutex";

typedef struct st_args_of_thread {
    pthread_mutex_t *mutexes;
    char *text;
    int number_of_thread;
} args_of_thread;

int checkOfErrors (int result_of_action, char *info_about_error) {
    if (result_of_action != SUCCESS) {
        perror(info_about_error);
        return EXIT_FAILURE;
    }
    return SUCCESS;
}

int destroyOfMutexes (pthread_mutex_t *mutexes) {
    for (int i = 0; i < count_of_mutexes; i++) {
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
            destroyOfMutexes(&mutexes[i]);
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

void *printText (args_of_thread *argumets) {
    char* text = argumets->text;
    pthread_mutex_t *mtxs = argumets->mutexes;
    int num_of_thrd = argumets->number_of_thread;

    if (num_of_thrd == 2) {
        lockOfMutex(num_of_thrd % count_of_mutexes, mtxs);
    }

    for (int i = 0; i < count_of_lines; i++) {
        errno = lockOfMutex((num_of_thrd + 2) % count_of_mutexes, mtxs);

        printf("%s %d\n", text, i);

        unlockOfMutex(num_of_thrd, mtxs);

        lockOfMutex((num_of_thrd + 1) % count_of_mutexes, mtxs);

        unlockOfMutex((num_of_thrd + 2) % count_of_mutexes, mtxs);

        lockOfMutex(num_of_thrd, mtxs);

        unlockOfMutex((num_of_thrd + 1) % count_of_mutexes, mtxs);
    }

    if (num_of_thrd == 2) {
        unlockOfMutex(num_of_thrd, mtxs);
    }
}

int main (int  argc, char *argv[]) {
    pthread_t id_of_thread;
    pthread_mutex_t mutexes[count_of_mutexes];
    char text_of_parent[size_of_string] = "Parent: ";
    char text_of_child[size_of_string] = "Child: ";

    args_of_thread args_of_parent,
                    args_of_child;
    args_of_parent.text = text_of_parent;
    args_of_child.text = text_of_child;
    args_of_parent.mutexes = mutexes;
    args_of_child.mutexes = mutexes;
    args_of_parent.number_of_thread = 1;
    args_of_child.number_of_thread = 2;

    errno = initializeOfMutexes(mutexes);
    int result_of_init = checkOfErrors(errno, "Error of initialization of attributes of mutexes");
    if (result_of_init != SUCCESS) {
        exit(EXIT_FAILURE);
    }

    errno = lockOfMutex(args_of_parent.number_of_thread, args_of_parent.mutexes);
    int result_of_lock = checkOfErrors(errno, "Error of lock of mutex before pthread_create() function");
    if (result_of_lock != SUCCESS) {
        destroyOfMutexes(mutexes);
        exit(EXIT_FAILURE);
    }

    errno = pthread_create(&id_of_thread, NULL, printText, &args_of_child);
    int result_of_creating = checkOfErrors(errno, "Error of creating of thread");
    if (result_of_creating != SUCCESS) {
        destroyOfMutexes(mutexes );
        exit(EXIT_FAILURE);
    }

    sleep(time_for_sleep);

    printText(&args_of_parent);

    errno = unlockOfMutex(args_of_parent.number_of_thread, args_of_parent.mutexes);
    checkOfErrors(errno, "Error of unlock of mutex before pthread_join() function");

    errno = pthread_join(id_of_thread, NULL);
    int result_of_joining = checkOfErrors(errno, "Error of joining of thread");
    if (result_of_joining != SUCCESS) {
        destroyOfMutexes(mutexes);
        exit(EXIT_FAILURE);
    }

    errno = destroyOfMutexes(mutexes);
    checkOfErrors(errno, "Error of destroying of mutexes");

    return SUCCESS;
}
