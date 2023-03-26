#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <bits/stdc++.h>
#include <time.h>
#define MIN_SLEEP 10
#define MAX_SLEEP 20
#define MIN_STAY 10
#define MAX_STAY 30
using namespace std;

typedef struct Room
{
    int room_id;
    int guest_id;
    int guest_priority;
    int dirty;
    int time;
    sem_t room_sem;
    Room(int id, int g, int p)
    {
        room_id = id;
        guest_id = g;
        guest_priority = p;
        dirty = 0;
        time = 0;
        sem_init(&room_sem, 0, 1);
    }
} Room;

vector<int> staff_id;
pthread_mutex_t std_lock;
sem_t vec_sem;
vector<int> rooms_to_be_cleaned;
int num_cleaners, num_guests, num_rooms;
vector<Room> rooms;
vector<pthread_t> guest_threads;
vector<pthread_t> cleaning_threads;
sem_t room_list_sem;
sem_t cleaning_sem;
int dirty_level = 0;

int get_random(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

void signal_handler(int sig)
{
    return;
}

void remove_guest(int indx)
{
    pthread_mutex_lock(&std_lock);
    cout << "GUEST THREAD: " << rooms[indx].guest_id << " Priority: " << rooms[indx].guest_priority << " Removed ..." << endl;
    pthread_mutex_unlock(&std_lock);
    pthread_kill(guest_threads[rooms[indx].guest_id - 1], SIGQUIT);
    rooms[indx].guest_id = 0;
    rooms[indx].guest_priority = 0;
}

int lowest_priority_guest()
{
    int prty = INT_MAX;
    int indx = -1;
    for (int i = 0; i < rooms.size(); i++)
    {
        if (rooms[i].guest_priority < prty && rooms[i].dirty < 2)
        {
            prty = rooms[i].guest_priority;
            indx = i;
        }
    }
    return indx;
}

void *staff_thread(void *arg)
{
    signal(SIGQUIT, signal_handler);
    int id = staff_id[*(int *)arg];
    while (true)
    {

        while (dirty_level < 2 * num_rooms)
            ;
        sem_wait(&vec_sem);
        rooms_to_be_cleaned.resize(num_rooms);
        for (int i = 0; i < num_rooms; i++)
        {
            rooms_to_be_cleaned[i] = i;
            if (rooms[i].guest_id != 0)
                remove_guest(i);
        }
        sem_post(&vec_sem);

        sem_wait(&cleaning_sem);
        while (dirty_level > 0)
        {
            sem_wait(&vec_sem);
            int idx;
            if(rooms_to_be_cleaned.size() == 0)
            {
                sem_post(&vec_sem);
                pthread_mutex_lock(&std_lock);
                // cout << "yay!" << endl;
                pthread_mutex_unlock(&std_lock);
                break;
            }
            if (rooms_to_be_cleaned.size() > 1)
                idx = get_random(0, rooms_to_be_cleaned.size() - 1);
            else
                idx = 0;
            int r = rooms_to_be_cleaned[idx];
            if (rooms_to_be_cleaned.size() > 1)
                swap(rooms_to_be_cleaned[idx], rooms_to_be_cleaned.back());
            rooms_to_be_cleaned.pop_back();
            sem_post(&vec_sem);

            sem_wait(&rooms[r].room_sem);
            if (rooms[r].dirty != 0)
            {
                pthread_mutex_lock(&std_lock);
                cout << "CLEANING THREAD: " << id << " Room: " << r << " Dirty Level: " << rooms[r].dirty << endl;
                pthread_mutex_unlock(&std_lock);
                sleep(rooms[r].time / 2);

                dirty_level -= rooms[r].dirty;
                rooms[r].dirty = 0;
                rooms[r].guest_id = 0;
                rooms[r].guest_priority = 0;
                rooms[r].time = 0;

                pthread_mutex_lock(&std_lock);
                cout << "CLEANING THREAD: " << id << " Room cleaned: " << r << endl;
                cout << "Dirty Level: " << dirty_level << endl;
                pthread_mutex_unlock(&std_lock);
            }
            sem_post(&rooms[r].room_sem);
        }
        pthread_mutex_lock(&std_lock);
        cout << "CLEANING THREAD: " << id << " Leaving ..." << endl;
        pthread_mutex_unlock(&std_lock);
        sem_post(&cleaning_sem);
    }
    pthread_exit(NULL);
}