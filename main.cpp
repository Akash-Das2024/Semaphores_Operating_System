#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <bits/stdc++.h>
#include <time.h>
#include "staff.cpp"
#include "guest.cpp"

using namespace std;

int main()
{
    srand(time(NULL));
    cout << "Enter the Number of Cleaners: ";
    cin >> num_cleaners;
    cout << "Enter the Number of Guests: ";
    cin >> num_guests;
    cout << "Enter the Number of Rooms: ";
    cin >> num_rooms;

    pthread_mutex_init(&std_lock, NULL); // std mutex
    guest_threads.resize(num_guests); // guest
    cleaning_threads.resize(num_cleaners); // staff

    sem_init(&room_list_sem, 0, num_guests);
    sem_init(&cleaning_sem, 0, num_cleaners);
    sem_init(&vec_sem, 0, 1);
    int prty;
    for (int i = 0; i < num_rooms; i++)
    {
        rooms.push_back(*(new Room(i, 0, 0)));
    }
    for (int i = 0; i < num_guests; i++)
    {
        prty = get_random(1, num_guests);

        guest_id.push_back({i + 1, prty});
        int *arg = new int(i);
        pthread_create(&guest_threads[i], NULL, guest_thread, arg);
        usleep(97);
    }
    for (int i = 0; i < num_cleaners; i++)
    {
        staff_id.push_back(i + 1);
        int *arg = new int(i);
        pthread_create(&cleaning_threads[i], NULL, staff_thread, arg);
        usleep(97);
    }

    for (int i = 0; i < num_guests; i++)
    {
        pthread_join(guest_threads[i], NULL);
    }

    for (int i = 0; i < num_cleaners; i++)
    {
        pthread_join(cleaning_threads[i], NULL);
    }
    pthread_mutex_destroy(&std_lock);

    for (int i = 0; i < num_rooms; i++)
    {
        sem_destroy(&rooms[i].room_sem);
    }
    sem_destroy(&room_list_sem);
    sem_destroy(&vec_sem);

    return 0;
}