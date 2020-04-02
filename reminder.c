#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define EVENTSIZE 365

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
    struct time Time;
    struct date Date;
};

struct checkReminderArgs {
    struct event *events;
    int n;
};

void *checkReminders(void *arguments) {
    time_t lastReminderTimeHit = 0;
    while (true) {
        time_t rawtime;
        struct tm *timeinfo;

        // Get local time.
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        // Check if a reminder is matching the local time.
        struct event *events = ((struct checkReminderArgs*)arguments)->events;
        int n = ((struct checkReminderArgs*)arguments)->n;
        for (int i = 0; i < n; i++) {
            if (events[i].Date.day == timeinfo->tm_mday 
            && events[i].Date.month == timeinfo->tm_mon + 1 
            && (events[i].Date.year - 1900) == timeinfo->tm_year 
            && events[i].Time.seconds == timeinfo->tm_sec 
            && events[i].Time.minutes == timeinfo->tm_min 
            && events[i].Time.hours == timeinfo->tm_hour
            && lastReminderTimeHit != rawtime) {
                printf("\nWe have hit the reminder at %s", asctime(timeinfo));
                lastReminderTimeHit = rawtime;
            }
        }
        // Sleep.
        sleep(.9);
    }
    return NULL;
}

void checkMemoryAlloc(void* pointer) {
    if (!pointer) {
        printf("Memory Allocation failed! Aborting...\n");
        exit(-1);
    }
}

int main() {
    int option = 0;
    int indexReached = 0;
    
    // For holding ALL reminder events.
    struct event *reminderEvents = (struct event*) calloc(EVENTSIZE, sizeof(struct event));
    checkMemoryAlloc(reminderEvents);
    printf("Easy C-Reminder - Set reminders and remember your tasks!\n\n");

    // Start multithreaded-handling for checking reminders.
    pthread_t thread_id;
    struct checkReminderArgs *rArgs = calloc(1, sizeof(struct checkReminderArgs));
    checkMemoryAlloc(rArgs);
    // Assign values to arguments struct.
    rArgs->events = reminderEvents;
    rArgs->n = EVENTSIZE;

    // Create and run thread.
    int rc = pthread_create(&thread_id, NULL, (void*)checkReminders, (void *)rArgs);
    if (rc) {
        printf("ERROR: return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    // Run thread.
    pthread_join((void *)&thread_id, NULL);

    // Start main loop for creating reminders.
    bool isRunning = true;
    while (isRunning) {
        // Ask the user what they want to do.
        printf("What do you want to do?\n");
        printf("1: Set a reminder. 2: Quit. ");
        scanf("%d", &option);

        struct event *eventToStore = (struct event*) calloc(1, sizeof(struct event));
        switch (option) {
            // Set a reminder.
            case 1:
                if (indexReached >= EVENTSIZE) {
                    printf("Your calendar is totally full!\n");
                    continue;
                }
                printf("If you do not specify a field it will be set to zero by default.\n");
                printf("What date? FORMAT: DAY/MONTH/YEAR ");
                scanf("%d/%d/%d", &eventToStore->Date.day,
                      &eventToStore->Date.month, &eventToStore->Date.year);
                printf("What time? FORMAT: HOURS:MINUTES:SECONDS ");
                scanf("%d:%d:%d", &eventToStore->Time.hours,
                      &eventToStore->Time.minutes, &eventToStore->Time.seconds);

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
    }

    return 0;
}
