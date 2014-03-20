#include <mpi.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

/* Konfiguracja świata "far far away" */

#define YETI_NUMBERS 10
#define PROJECTORS_NUMBER 10
#define ROOM_NUMBERS 10
#define MAX_POWER 100
#define NEW_SEMINARY_CHANCE 20 			//w procentach
#define MIN_SLEEP_BETWEENS_ATTEMPS 50	// w sekundach
#define MAX_SLEEP_BETWEENS_ATTEMPS 7	// w sekundach

#define SEMINARY_TIME 30				// w sekundach
#define SEMINARY_POWER_CONSUMPTION 10

#define POWER_REGENERATION_TIME 50		// w sekundach

/* ---------------------------------------------------------- */


#define YETI_FREE 0
#define YETI_BLOCKED 1
#define YETI_WORKING 2



void seminary_process(){
	Sleep(SEMINARY_TIME*1000);
	//oddać projektor i salę

	our_yeti['power'] = our_yeti['power'] - SEMINARY_POWER_CONSUMPTION;
	if (our_yeti['power'] <= SEMINARY_POWER_CONSUMPTION){
		Sleep(POWER_REGENERATION_TIME*1000);

		//oddać yeti
	}
}

void new_seminary_process(){
	if ( rand()%100 <= NEW_SEMINARY_CHANCE ){

		//ustalanie zasobów

		if ( !fork() ){
			seminary_process();
			exit(0);
		}

	}

	Sleep(((rand() % MAX_SLEEP_BETWEENS_ATTEMPS - MIN_SLEEP_BETWEENS_ATTEMPS) + MIN_SLEEP_BETWEENS_ATTEMPS)*1000);
}



int main(int argc, char **argv)
{
	int mpi_size, mpi_rank;
 
	int free_projectors = PROJECTORS_NUMBER;  		//ilość dostępnych projektorów
	int yeti[YETI_NUMBERS][2];
	int room[ROOM_NUMBERS];
	int i;

	for ( i = 0; i < YETI_NUMBERS; i++ ){
		yeti[i]['stan'] = YETI_FREE;
		yeti[i]['power'] = MAX_POWER;
	} 



	MPI_Init(&argc, &argv);

	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);



	if ( fork() ){
		//główna obsługa komunikacji między zarządcami oraz wewnętrzna - między forkami

	}{
		while (1)
			new_seminary_process();
	}


    MPI_Finalize();
}

