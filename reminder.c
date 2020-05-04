#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if defined(_WIN32)
    #include <windows.h> // Windows
    #define ON_WIN
#elif defined(_WIN64)
    #include <windows.h> // Windows
    #define ON_WIN
#elif defined(__CYGWIN__) && !defined(_WIN32)
    #include <windows.h> // Windows (Cygwin POSIX under Microsoft Window)
    #define ON_WIN
#endif

#define EXPANDVALUE 15

struct time {
    int seconds;
    int minutes;
    int hours;
};

struct date {
    int day;
    int month;
    int year;
};

struct event {
    char name[150];
    char description[300];
    struct time Time;
    struct date Date;
};

struct checkReminderArgs {
    struct event **events;
    int *n;
};

// Cross-platform sleep function
void sleep_ms(int milliseconds)
{
    #ifdef ON_WIN
        Sleep(milliseconds);
    #elif _POSIX_C_SOURCE >= 199309L
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
        nanosleep(&ts, NULL);
    #else
        usleep(milliseconds * 1000);
    #endif
}

// Check if allocated memory is valid.
void checkMemoryAlloc(void *pointer) {
    if (!pointer) {
        printf("Memory Allocation failed! Aborting...\n");
        exit(-1);
    }
}

// Remove a trailing newline.
void removeNewline(char *str) {
    // Set newline to null character.
    str[strcspn(str, "\n")] = 0;
}

// Get reminder data.
void readReminder(struct event *eventToStore) {
    // Get reminder data from user and store it.
    char reminderDate[100], reminderTime[100];
    printf("If you do not specify a field it will be set to zero by default.\n");

    printf("Name of reminder (Max. 150 characters): ");
    fgets(eventToStore->name, sizeof(eventToStore->name), stdin);
    removeNewline(eventToStore->name);

    printf("Description of reminder (Max. 300 characters): ");
    fgets(eventToStore->description, sizeof(eventToStore->description), stdin);
    removeNewline(eventToStore->description);

    printf("What date? FORMAT: DAY/MONTH/YEAR ");
    fgets(reminderDate, sizeof(reminderDate), stdin);
    sscanf(reminderDate, "%d %*c %d %*c %d", 
        &eventToStore->Date.day, &eventToStore->Date.month, &eventToStore->Date.year);
    
    printf("What time? FORMAT: HOURS:MINUTES:SECONDS ");
    fgets(reminderTime, sizeof(reminderTime), stdin);
    sscanf(reminderTime, "%d %*c %d %*c %d", &eventToStore->Time.hours,
        &eventToStore->Time.minutes, &eventToStore->Time.seconds);
}

// Dynamically expand reminder array.
void expandReminderArray(struct event **eventAddr, int *n) {
    // This function is only called if the array is full;
    // Re-allocate more memory to store more reminder events.
    size_t newSize = (*n * sizeof(struct event)) 
            + (sizeof(struct event) * EXPANDVALUE);
    *eventAddr = (struct event*) realloc(*eventAddr, newSize);

    // Initialize all NEW elements to zero.
    struct event *eventPointer = *eventAddr + *n;
    struct event *endEventArray = *eventAddr + (newSize / sizeof(struct event));
    struct event zeroEvent = {0};
    while (eventPointer < endEventArray) {
        *eventPointer = zeroEvent;
        eventPointer++;
    }
    
    *n += EXPANDVALUE;
}

// Multi-threaded function to check if any reminders has been hit.
void* checkReminders(void *arguments) {
    time_t lastReminderTimeHit = 0;
    while (true) {
        time_t rawtime;
        struct tm *timeinfo;

        // Get local time.
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        // Check if a reminder is matching the local time.
        struct event **events = (struct event**) (((struct checkReminderArgs*) arguments)->events);
        const int *n = (int*) ((struct checkReminderArgs*) arguments)->n;
        for (int i = 0; i < *n; i++) {
            if ((*events)[i].Date.day == timeinfo->tm_mday 
            && (*events)[i].Date.month == timeinfo->tm_mon + 1 
            && ((*events)[i].Date.year - 1900) == timeinfo->tm_year 
            && (*events)[i].Time.seconds == timeinfo->tm_sec 
            && (*events)[i].Time.minutes == timeinfo->tm_min 
            && (*events)[i].Time.hours == timeinfo->tm_hour
            && lastReminderTimeHit != rawtime) {
                printf("\nRemember your task: %s - %s", (*events)[i].name, asctime(timeinfo));
                printf("Description: %s", (*events)[i].description);
                // Only if on windows platform.
                #ifdef ON_WIN
                    MessageBeep(MB_ICONWARNING);
                #endif

                lastReminderTimeHit = rawtime;
            }
        }
        // Sleep.
        sleep_ms(900);
    }
    return NULL;
}

int main() {
    int option = 0;
    int indexReached = 0;
    
    // For holding ALL reminder events.
    int eventArrSize = EXPANDVALUE; // Same as EXPANDVALUE of array.
    struct event *reminderEvents = (struct event*) calloc(eventArrSize, sizeof(struct event));
    checkMemoryAlloc(reminderEvents);
    printf("Easy C-Reminder - Set reminders and remember your tasks!\n\n");

    // Start multithreaded-handling for checking reminders.
    pthread_t thread_id;
    struct checkReminderArgs *rArgs = calloc(1, sizeof(struct checkReminderArgs));
    checkMemoryAlloc(rArgs);
    // Assign values to arguments struct.
    rArgs->events = &reminderEvents;
    rArgs->n = &eventArrSize;

    // Create and run thread.
    int rc = pthread_create(&thread_id, NULL, (void*)checkReminders, (void *)rArgs);
    if (rc) {
        printf("ERROR: return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    // Run thread.
    pthread_join((void *) &thread_id, NULL);

    // Start main loop for creating reminders.
    bool isRunning = true;
    while (isRunning) {
        // Ask the user what they want to do.
        printf("What do you want to do?\n");
        printf("1: Set a reminder. 2: Quit. ");
        scanf("%d", &option);
        // Remove newline left in input stream.
        getchar();

        struct event *eventToStore = (struct event*) calloc(1, sizeof(struct event));
        switch (option) {
            // Set a reminder.
            case 1:
                if (indexReached == eventArrSize) {
                    // The array of reminders is totally full;
                    // re-allocate more memory to store more reminders.
                    expandReminderArray(&reminderEvents, &eventArrSize);

                    readReminder(eventToStore);
                    // Store the reminder event.
                    reminderEvents[indexReached] = *eventToStore;
                    indexReached++;
                    continue;
                }
                readReminder(eventToStore);

                // Store the reminder event.
                reminderEvents[indexReached] = *eventToStore;
                indexReached++;
                break;
            // Quit.
            case 2:
                printf("\nSee you!\n");
                isRunning = false;
                break;
        }
        free(eventToStore);
    }

    free(reminderEvents);
    free(rArgs);

    return 0;
}
