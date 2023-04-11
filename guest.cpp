#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <bits/stdc++.h>
#include <time.h>

using namespace std;

vector<pair<int, int>> guest_id;

void *guest_thread(void *arg)
{

    signal(SIGQUIT, signal_handler);
    int index = *((int *)arg);
    int guest = guest_id[index].first;
    int priority = guest_id[index].second;

    while (true)
    {
        int sleep_time = get_random(MIN_SLEEP, MAX_SLEEP);
        pthread_mutex_lock(&std_lock);
        cout << "GUEST THREAD: " << guest << " Priority: " << priority << " Sleeping for: " << sleep_time << endl;
        pthread_mutex_unlock(&std_lock);
        sleep(sleep_time);
        int stay_time = get_random(MIN_STAY, MAX_STAY);
        int empty_room_found = 0;
        sem_wait(&room_list_sem);
        int value;
        do
        {
            sem_getvalue(&cleaning_sem, &value);
        } while (value < num_cleaners);
        usleep(sleep_time);
        for (int i = 0; i < num_rooms; i++)
        {
            pthread_mutex_lock(&contention_lock);
            if (rooms[i].guest_id == 0 && rooms[i].dirty < 2)
            {

                rooms[i].guest_id = guest;
                rooms[i].guest_priority = priority;
                rooms[i].dirty++;
                dirty_level++;
                empty_room_found = 1;
                rooms[i].time += stay_time;
                pthread_mutex_unlock(&contention_lock);

                sem_wait(&rooms[i].room_sem);

                pthread_mutex_lock(&std_lock);
                cout << "GUEST THREAD: " << guest
                     << " Priority: " << priority << " Room Number: " << i << " Stay time: " << stay_time << endl;
                pthread_mutex_unlock(&std_lock);
                int sleep_flag = sleep(stay_time);
                pthread_mutex_lock(&contention_lock);
                rooms[i].guest_id = 0;
                rooms[i].guest_priority = 0;
                pthread_mutex_unlock(&contention_lock);
                if (!sleep_flag)
                {
                    pthread_mutex_lock(&std_lock);
                    cout << "GUEST THREAD: " << guest
                         << " Priority: " << priority << " Room Number: " << i << " Discharged by Guest voluntarily" << endl;
                    pthread_mutex_unlock(&std_lock);
                }
                sem_post(&rooms[i].room_sem);
                break;
            }
            else
            {
                pthread_mutex_unlock(&contention_lock);
            }
        }
        if (empty_room_found == 0)
        {

            int indx = lowest_priority_guest();
            pthread_mutex_lock(&contention_lock);
            if (indx != -1 && rooms[indx].guest_priority < priority && rooms[indx].dirty < 2)
            {
                pthread_mutex_lock(&std_lock);
                cout << "GUEST THREAD: " << rooms[indx].guest_id << " Priority: " << rooms[indx].guest_priority << " Kicked from room number: " << indx << " By guest: " << guest << " with higher priority: " << priority << endl;
                pthread_mutex_unlock(&std_lock);
                if (rooms[indx].guest_id != 0)
                {
                    remove_guest(indx);
                }
                // cout << "flag" << endl;

                rooms[indx].guest_id = guest;
                rooms[indx].guest_priority = priority;
                rooms[indx].dirty++;
                dirty_level += 1;
                pthread_mutex_unlock(&contention_lock);

                sem_wait(&rooms[indx].room_sem);

                rooms[indx].time += stay_time;
                pthread_mutex_lock(&std_lock);
                cout << "GUEST THREAD: " << guest << " Priority: " << priority << " Room Number: " << indx << " Stay time: " << stay_time << endl;
                pthread_mutex_unlock(&std_lock);
                int sleep_flag = sleep(stay_time);

                pthread_mutex_lock(&contention_lock);
                rooms[indx].guest_id = 0;
                rooms[indx].guest_priority = 0;
                pthread_mutex_unlock(&contention_lock);
                if (!sleep_flag)
                {
                    pthread_mutex_lock(&std_lock);
                    cout << "GUEST THREAD: " << guest << " Priority: " << priority << " Room Number: " << indx << " Discharged by Guest voluntarily" << endl;
                    pthread_mutex_unlock(&std_lock);
                }
                sem_post(&rooms[indx].room_sem);
            }
            else
            {
                pthread_mutex_unlock(&contention_lock);
            }
        }
        sem_post(&room_list_sem);
    }
    pthread_exit(NULL);
}