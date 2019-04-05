#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define COUNT 90

struct station
{
    pthread_mutex_t stationMutex;
    pthread_cond_t trainArrived;
    pthread_cond_t trainLeftStation;
    pthread_cond_t passengerLoaded;
    int numPassengers;
    int numPassengersWaiting;
    int numAvailableSeats;
    char trainInStation;

};

typedef struct station Station;

void stationInit(Station *station)
{
    pthread_mutex_init(&(station->stationMutex),NULL);
    pthread_cond_init(&(station->trainArrived),NULL);
    pthread_cond_init(&(station->trainLeftStation),NULL);
    pthread_cond_init(&(station->passengerLoaded),NULL);
    station->numPassengers = 0;
    station->numPassengersWaiting = 0;
    station->trainInStation = 0;
}

void station_load_train(Station *station, int count)
{
    pthread_mutex_lock(&(station->stationMutex));
    //If There Is A Train In The Station Loading Passengers
    while(station->trainInStation == 1)
        pthread_cond_wait(&(station->trainLeftStation),&(station->stationMutex));
    station->trainInStation = 1;
    station->numAvailableSeats = count;

    printf("Train Arrived And Number of Available Seats is %d\n",station->numAvailableSeats);

    pthread_cond_broadcast(&(station->trainArrived));

    //Either Train Is Full or All Waiting Passengers Have Boarded
    while(((station->numAvailableSeats != 0) && (station->numPassengers > 0))||((station->numAvailableSeats>station->numPassengers) && (station->numPassengers!=0)))
    {
        printf("Still Loading Passengers\n");
        pthread_cond_wait(&(station->passengerLoaded), &(station->stationMutex));
    }
    //Let The Next Train Start Loading Passengers
    pthread_cond_broadcast(&(station->trainLeftStation));

    printf("Train%d Is Loaded Successfully\n",pthread_self());
    printf("numAvailableSeats = %d and numWaitingPassengers = %d\n",station->numAvailableSeats,station->numPassengersWaiting);

    station->trainInStation = 0;
    pthread_mutex_unlock(&(station->stationMutex));
}

void station_wait_for_train(Station *station)
{
    pthread_mutex_lock(&(station->stationMutex));
    station->numPassengersWaiting++;
    printf("A Passenger Is Waiting For Train\n");

    //Waiting On Those Two Conditions Make Sure A Passenger Wait For A Train
    //Even If The Current Train Has Loaded And Left So It Will Wait For The
    //Next Train
    while((station->trainInStation != 1) || (station->numAvailableSeats == 0))
        pthread_cond_wait(&(station->trainArrived),&(station->stationMutex));

    //Move The Passenger On Board
    station->numPassengersWaiting--;
}

void station_on_board(Station *station)
{
    //Sit The Passenger
    station->numAvailableSeats--;
    station->numPassengers--;
    printf("The Passenger Now Is On Board and Passengers Left = %d\n",station->numPassengers);
    printf("Number Of Available Seats After Boarding Passenger = %d\n",station->numAvailableSeats);
    pthread_cond_signal(&(station->passengerLoaded));
    pthread_mutex_unlock(&(station->stationMutex));

}

void *train(void *args)
{
    Station *station = (Station*) args;
    station_load_train(station,COUNT);
    pthread_exit(NULL);
}

void *passenger(void *args)
{
    Station *station = (Station*) args;
    station_wait_for_train(station);
    station_on_board(station);
    pthread_exit(NULL);
}
int main() {

    int num_passengers=0;
    int num_trains=0;
    printf("Enter Number of Passengers: ");
    scanf("%d",&num_passengers);
    printf("Enter Number of Trains: ");
    scanf("%d",&num_trains);

    pthread_t passengers[num_passengers];
    pthread_t trains[num_trains];
    Station *station = malloc(sizeof(Station));
    stationInit(station);
    station->numPassengers = num_passengers;
    for (int i = 0; i < num_passengers ; ++i)
    {
        pthread_create(&passengers[i],NULL,passenger,(void*)station);
    }

    for (int j = 0; j < num_trains ; ++j)
    {
        pthread_create(&trains[j],NULL,train,(void*)station);
    }


    for(int i=0 ; i< num_passengers ; i++)
        pthread_join(passengers[i],NULL);

    for (int k = 0; k < num_trains ; ++k)
    {
        pthread_join(trains[k],NULL);
    }
    return 0;
}