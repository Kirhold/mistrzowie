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

#define ASK_FOR_PROJECTOR
#define ASK_FOR_ROOM
#define ASK_FOR_YETI

#define ANS_FOR_PROJECTOR
#define ANS_FOR_ROOM
#define ANS_FOR_YETI


void seminary_process(){
	Sleep(SEMINARY_TIME*1000);
	// todo: oddać projektor i salę

	our_yeti['power'] = our_yeti['power'] - SEMINARY_POWER_CONSUMPTION;
	if (our_yeti['power'] <= SEMINARY_POWER_CONSUMPTION){
		Sleep(POWER_REGENERATION_TIME*1000);

		// todo: oddać yeti
	}
}

void new_seminary_process(){
	if ( rand()%100 <= NEW_SEMINARY_CHANCE ){
		
		// todo: wybranie zasobów


		block_error = false;
		for (i = 0; i < mpi_size && i != mpi_rank; i++){
			MPI_Send( &ask, 2, MPI_INT, i, MPI_ASK, MPI_COMM_WORLD );				//pyta i blokuje
			MPI_Recv( &answer, 1, MPI_INT, i, MPI_ANS, MPI_COMM_WORLD, &status);

			if (answer == 0 ){			//jeśli ktoś nie zablokował tzn, że jest niedostepny jakiś zasób
				for ( j = 0; j <= i; j++){
					MPI_Send( &ask, 2, MPI_INT, i, MPI_UNBLOCK, MPI_COMM_WORLD );				//odblokowuje

				}

				block_error = true;
				break;
			}

		}

		if (!block_error){
			for (i = 0; i < mpi_size && i != mpi_rank; i++)
				MPI_Send( &ask, 2, MPI_INT, i, MPI_GET, MPI_COMM_WORLD );				//zajmuje zasób


			if ( !fork() ){
				seminary_process();
				exit(0);
			}
		}
	}

	Sleep(((rand() % MAX_SLEEP_BETWEENS_ATTEMPS - MIN_SLEEP_BETWEENS_ATTEMPS) + MIN_SLEEP_BETWEENS_ATTEMPS)*1000);
}



int main(int argc, char **argv)
{
	int mpi_size, mpi_rank;
 
	int free_projectors = PROJECTORS_NUMBER;  		//ilość dostępnych projektorów
	int free_yetis = YETI_NUMBERS;
	int free_room = ROOM_NUMBERS;
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
		// todo: główna obsługa komunikacji między zarządcami oraz wewnętrzna - między forkami

	}{
		bool block_error;
		while (1)
			new_seminary_process();
	}


    MPI_Finalize();
}

