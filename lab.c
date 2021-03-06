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
        int result_of_destroying = checkOfErrors(errno, "Error of destroying of mutexes");
        if (result_of_destroying != SUCCESS) {
            return EXIT_FAILURE;
        }
    }
    return SUCCESS;
}

int initializeOfMutexes (pthread_mutex_t *mutexes) {
    pthread_mutexattr_t mattr;

    errno = pthread_mutexattr_init(&mattr);
    int result_of_init_of_mattr = checkOfErrors(errno, "Error of initialization of attributes of mutexes");
    if (result_of_init_of_mattr != SUCCESS) {
        return EXIT_FAILURE;
    }

    errno = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
    int result_of_setting_type = checkOfErrors(errno, "Error of setting of attributes of mutexes");
    if (result_of_setting_type != SUCCESS) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < count_of_mutexes; i++) {
        errno = pthread_mutex_init(&mutexes[i], &mattr);
        int result_of_init = checkOfErrors(errno, "Error of initialization of mutexes");
        if (result_of_init != SUCCESS) {
            destroyOfMutexes(&mutexes[i]);
            return EXIT_FAILURE;
        }
    }
    return SUCCESS;
}

int lockOfMutex (int number_of_mtx, pthread_mutex_t *mutexes) {
    errno = pthread_mutex_lock(&mutexes[number_of_mtx]);
    int result_of_lock = checkOfErrors(errno, "Error of lock of mutex");
    if (result_of_lock != SUCCESS) {
        destroyOfMutexes(mutexes);
        return EXIT_FAILURE;
    }
    return SUCCESS;
}

int unlockOfMutex (int number_of_mtx, pthread_mutex_t *mutexes) {
    errno = pthread_mutex_unlock(&mutexes[number_of_mtx]);
    int result_of_unlock = checkOfErrors(errno, "Error of unlock of mutex");
    if(result_of_unlock != SUCCESS) {
        destroyOfMutexes(mutexes);
        return EXIT_FAILURE;
    }
    return SUCCESS;
}

void *printText (args_of_thread *argumets) {
    char* text = argumets->text;
    pthread_mutex_t *mtxs = argumets->mutexes;
    int num_of_thrd = argumets->number_of_thread;
    int result_of_lock;
    int result_of_unlock;

    if (num_of_thrd == 2) {
        result_of_lock = lockOfMutex(num_of_thrd % count_of_mutexes, mtxs);
        if(result_of_lock != SUCCESS) {
            return (void *)EXIT_FAILURE;
        }
    }

    for (int i = 0; i < count_of_lines; i++) {
        result_of_lock = lockOfMutex((num_of_thrd + 2) % count_of_mutexes, mtxs);
        if(result_of_lock != SUCCESS) {
            return (void *)EXIT_FAILURE;
        }

        printf("%s %d\n", text, i);

        result_of_unlock = unlockOfMutex(num_of_thrd, mtxs);
        if(result_of_unlock != SUCCESS) {
            return (void *)EXIT_FAILURE;
        }

        result_of_lock = lockOfMutex((num_of_thrd + 1) % count_of_mutexes, mtxs);
        if(result_of_lock != SUCCESS) {
            return (void *)EXIT_FAILURE;
        }

        result_of_unlock = unlockOfMutex((num_of_thrd + 2) % count_of_mutexes, mtxs);
        if(result_of_unlock != SUCCESS) {
            return (void *)EXIT_FAILURE;
        }

        result_of_lock = lockOfMutex(num_of_thrd, mtxs);
        if(result_of_lock != SUCCESS) {
            return (void *)EXIT_FAILURE;
        }

        result_of_unlock = unlockOfMutex((num_of_thrd + 1) % count_of_mutexes, mtxs);
        if(result_of_unlock != SUCCESS) {
            return (void *)EXIT_FAILURE;
        }
    }

    if (num_of_thrd == 2) {
        result_of_unlock = unlockOfMutex(num_of_thrd, mtxs);
        if(result_of_unlock != SUCCESS) {
            return (void *)EXIT_FAILURE;
        }
    }
    return SUCCESS;
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

    int result_of_lock = lockOfMutex(args_of_parent.number_of_thread, args_of_parent.mutexes);
    if (result_of_lock != SUCCESS) {
        exit(EXIT_FAILURE);
    }

    errno = pthread_create(&id_of_thread, NULL, printText, &args_of_child);
    int result_of_creating = checkOfErrors(errno, "Error of creating of thread");
    if (result_of_creating != SUCCESS) {
        destroyOfMutexes(mutexes);
        exit(EXIT_FAILURE);
    }

    sleep(time_for_sleep);

    int result_of_print_parent;
    result_of_print_parent = (int)printText(&args_of_parent);

    int result_of_unlock = unlockOfMutex(args_of_parent.number_of_thread, args_of_parent.mutexes);
    if (result_of_unlock != SUCCESS) {
        exit(EXIT_FAILURE);
    }

    int result_of_print_child;
    errno = pthread_join(id_of_thread, (void **)&result_of_print_child);
    int result_of_joining = checkOfErrors(errno, "Error of joining of thread");
    if (result_of_joining != SUCCESS) {
        destroyOfMutexes(mutexes);
        exit(EXIT_FAILURE);
    }

    if (result_of_print_child != SUCCESS || result_of_print_parent != SUCCESS) {
        fprintf(stderr, "Error in printText() function");
        exit(EXIT_FAILURE);
    }

    int result_of_destroying = destroyOfMutexes(mutexes);
    if (result_of_destroying != SUCCESS) {
        exit(EXIT_FAILURE);
    }

    return SUCCESS;
}
