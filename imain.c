#include <mpi.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>



/* Konfiguracja świata "far far away" */

#define YETI_NUMBERS 10
#define PROJECTORS_NUMBER 10
#define ROOM_NUMBERS 10
#define MAX_POWER 100 					// w jednostkach
#define NEW_LECTURE_CHANCE 30 			// w procentach
#define MIN_SLEEP_BETWEENS_ATTEMPS 10	// w sekundach
#define MAX_SLEEP_BETWEENS_ATTEMPS 7	// w sekundach


#define LECTURE_TIME 5					// w sekundach
#define LECTURE_POWER_CONSUMPTION 10 	// w jednostkach

#define POWER_REGENERATION_TIME 30		// w sekundach

/* ---------------------------------------------------------- */


#define MPI_ASK 1
#define MPI_ANS 2
#define MPI_UNBLOCK 3
#define MPI_GET 4
#define MPI_UNBLOCK_ROOM 5
#define MPI_UNBLOCK_YETI 6


#define FREE 0
#define BLOCKED 1
#define WORKING 2

// udawanie tablic asocjacyjnych

//yeti
#define A_YETI_STAN 0
#define A_YETI_POWER 1

// group
#define A_GROUP_YETI 0
#define A_GROUP_ROOM 1

// meditaion_room
#define A_TIMESTAMP 0
#define A_YETI_ID 1 

//lectures
#define A_L_TIMESTAMP 0
#define A_L_YETI 1
#define A_L_ROOM 2 



/*  Zmienne globalne  */

int timestamp;

int mpi_size, mpi_rank;

int free_projectors = PROJECTORS_NUMBER;  		//ilość dostępnych projektorów
int free_yetis = YETI_NUMBERS;
int free_rooms = ROOM_NUMBERS;

int yeti[YETI_NUMBERS][2];
int room[ROOM_NUMBERS];


int lectures[ROOM_NUMBERS][3];
int active_lectures_count = 0;

int meditation_room[YETI_NUMBERS][2];
int meditator_yetis = 0;

int next_new_lecture_time = 0;

int tmp;
	int test_index;
	int test_flag;
	int answer;
	MPI_Status test_status;

	int ask_buf[2];
	int unblock_buf[2];
	int get_buf[2];
	int unblock_room_buf;
	int unblock_yeti_buf[2];


	int test =0;

	MPI_Request ask_request;
	MPI_Request unblock_request;
	MPI_Request get_request;
	MPI_Request unblock_room_request;
	MPI_Request unblock_yeti_request;
	int request_list[5];

/*  Funkcje   */


void response(){
		request_list[0] = ask_request;
	request_list[1] = unblock_request;
	request_list[2] = get_request;
	request_list[3] = unblock_room_request;
	request_list[4] = unblock_yeti_request;
 MPI_Request any_request;

	MPI_Testany( 5, request_list, &test_index, &test_flag, &test_status );
	while (test_flag){
		//printf("%i: %i from %i\n", mpi_rank, test_index, test_status.MPI_SOURCE);
		switch(test_index){
			case 0:	//pytanie o zasoby i blokowanie
			
				if (yeti[ask_buf[A_GROUP_YETI]][A_YETI_STAN] == FREE && room[ask_buf[A_GROUP_ROOM]] == FREE && free_projectors > 0){
					yeti[ask_buf[A_GROUP_YETI]][A_YETI_STAN] = BLOCKED;
					room[ask_buf[A_GROUP_ROOM]] = BLOCKED;
					free_yetis = free_yetis - 1;
					free_rooms = free_rooms - 1;
					free_projectors = free_projectors - 1;

					answer = 1;
					MPI_Issend (&answer, 1, MPI_INT, test_status.MPI_SOURCE, MPI_ANS, MPI_COMM_WORLD, &any_request);
	
				//	printf("%i Otrzymano zapytanie o zasoby od zarządcy %i: zgoda\n", mpi_rank, test_status.MPI_SOURCE);
				}else{
					answer = 0;
					MPI_Issend (&answer, 1, MPI_INT, test_status.MPI_SOURCE, MPI_ANS, MPI_COMM_WORLD, &any_request);
					//printf("%i Otrzymano zapytanie o zasoby od zarządcy %i: odmowa\n", mpi_rank, test_status.MPI_SOURCE);
				}

				MPI_Irecv (ask_buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ASK, MPI_COMM_WORLD, &ask_request);
				break;

			case 1: //odblokowywanie zasobów
				//printf("%i Odblokowano zasoby od zarządcy %i\n", mpi_rank, test_status.MPI_SOURCE);
				yeti[unblock_buf[A_GROUP_YETI]][A_YETI_STAN] = FREE;
				room[unblock_buf[A_GROUP_ROOM]] = FREE;
				free_yetis = free_yetis + 1;
				free_rooms = free_rooms + 1;
				free_projectors = free_projectors + 1;

				MPI_Irecv (unblock_buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_UNBLOCK, MPI_COMM_WORLD, &unblock_request);
				break;

			case 2: //zajęcie zasobów
				//printf("%i Zajęto zasoby dla zarządcy %i\n", mpi_rank, test_status.MPI_SOURCE);
				
				yeti[get_buf[A_GROUP_YETI]][A_YETI_STAN] = WORKING;
				room[get_buf[A_GROUP_ROOM]] = WORKING;

				MPI_Irecv (get_buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_GET, MPI_COMM_WORLD, &get_request);
				break;

			case 3: // odblokowanie room
				room[unblock_room_buf] = FREE;
				free_rooms = free_rooms + 1;
				free_projectors = free_projectors + 1;
				MPI_Irecv (&unblock_room_buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_UNBLOCK_ROOM, MPI_COMM_WORLD, &unblock_room_request);
				break;

			case 4: //odblokowanie yeti
				yeti[unblock_yeti_buf[0]][A_YETI_STAN] = FREE;
				yeti[unblock_yeti_buf[0]][A_YETI_POWER] = unblock_yeti_buf[1];
				free_yetis = free_yetis + 1;

				MPI_Irecv (unblock_yeti_buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_UNBLOCK_YETI, MPI_COMM_WORLD, &unblock_yeti_request);
				break;			
		}	

		MPI_Testany( 5, request_list, &test_index, &test_flag, &test_status );
	}
}


int search_in_array(int our_array[], int count, int value, int el_no){
	bool found = false;
	int i;

	for (i = 0; i < count; i++){
		if (our_array[i] == value)
			el_no = el_no - 1;
		
		if (el_no == 0){
			found = true;
			break; 
		}
	}

	if (found)
		return i;
	else
		return -1;
}

int search_depper_in_array(int our_array[][2], int count, int name, int value, int el_no){
	bool found = false;
	int i;

	for (i = 0; i < count; i++){

		if (our_array[i][name] == value)
			el_no = el_no - 1;
		
		if (el_no == 0){
			found = true;
			break; 
		}
	}

	if (found)
		return i;
	else
		return -1;
}


void give_back_yeti(int yeti_id){
	int i;
	int yeti_tmp[] = {yeti_id, yeti[yeti_id][A_YETI_POWER]};

	int tmp_flag;
	MPI_Status tmp_status;
	MPI_Request tmp_request;

	yeti[yeti_id][A_YETI_STAN] = FREE;
	free_yetis = free_yetis + 1;
	for ( i = 0; i < mpi_size; i++ ){
		if ( i == mpi_rank )
			continue;

		MPI_Issend (&yeti_tmp, 2, MPI_INT, i, MPI_UNBLOCK_YETI, MPI_COMM_WORLD, &tmp_request);//pyta i blokuje
		do{
			response();
			MPI_Test (&tmp_request, &tmp_flag, &tmp_status);

		}while (!tmp_flag);		
	}
}

void end_lecture(){
	int yeti_id = lectures[0][A_L_YETI];
	int i;

	int tmp_flag;
	MPI_Status tmp_status;
	MPI_Request tmp_request;


	room[lectures[0][A_L_ROOM]] = FREE;
	free_rooms = free_rooms + 1;
	free_projectors = free_projectors + 1;
	for (i = 0; i < mpi_size; i++){
		if ( i == mpi_rank )
				continue;

		MPI_Issend (&lectures[0][A_L_ROOM], 1, MPI_INT, i, MPI_UNBLOCK_ROOM, MPI_COMM_WORLD, &tmp_request);//odblokowuje pokój
		do{
			response();
			MPI_Test (&tmp_request, &tmp_flag, &tmp_status);

		}while (!tmp_flag);
	}
	yeti[yeti_id][A_YETI_POWER] = yeti[yeti_id][A_YETI_POWER] - LECTURE_POWER_CONSUMPTION;
	if (yeti[yeti_id][A_YETI_POWER] <= LECTURE_POWER_CONSUMPTION){
		meditation_room[meditator_yetis][A_YETI_ID] = yeti_id;
		meditation_room[meditator_yetis][A_TIMESTAMP] = timestamp + POWER_REGENERATION_TIME;

		meditator_yetis = meditator_yetis + 1; 
		printf("%i: \twysyłam Mistrza na medytacje (%i)\n", mpi_rank, yeti_id);
	
	}else
		give_back_yeti(yeti_id);


	for (i = 1; i <= active_lectures_count - 1; i++){
		lectures[i-1][A_L_TIMESTAMP] = lectures[i][A_L_TIMESTAMP];
		lectures[i-1][A_L_YETI] = lectures[i][A_L_YETI];
		lectures[i-1][A_L_ROOM] = lectures[i][A_L_ROOM];

	}
	lectures[active_lectures_count - 1][A_L_TIMESTAMP] = 0;
	active_lectures_count = active_lectures_count - 1;

}


void new_lecture(){
	bool block_error;
	int answer;
	MPI_Status status;
	int i, j;
	int our_group[2];

	int tmp_flag;
	MPI_Status tmp_status;
	MPI_Request tmp_request;
	
	if ( rand()%100 <= NEW_LECTURE_CHANCE && free_yetis > 0 && free_rooms > 0 && free_projectors > 0){
		our_group[A_GROUP_YETI] = search_depper_in_array(yeti, YETI_NUMBERS, A_YETI_STAN, FREE, (1 +rand() % free_yetis));
		our_group[A_GROUP_ROOM] = search_in_array(room, ROOM_NUMBERS, FREE, (1 + rand() % free_rooms));


		if (our_group[A_GROUP_YETI] != -1 && our_group[A_GROUP_ROOM] != -1 && free_projectors > 0){
			//printf("%i: Rozpoczęto tworzenie wykładu\n", mpi_rank);		
		
			block_error = false;
			yeti[our_group[A_GROUP_YETI]][A_YETI_STAN] = BLOCKED;
			room[our_group[A_GROUP_ROOM]] = BLOCKED;
			free_yetis = free_yetis - 1;
			free_rooms = free_rooms - 1;
			free_projectors = free_projectors - 1;
			for (i = 0; i < mpi_size; i++){
				if ( i == mpi_rank )
					continue;

				MPI_Issend (&our_group, 2, MPI_INT, i, MPI_ASK, MPI_COMM_WORLD, &tmp_request);//pyta i blokuje
				do{
					response();
					MPI_Test (&tmp_request, &tmp_flag, &tmp_status);

				}while (!tmp_flag);

				MPI_Irecv (&answer,1,  MPI_INT, i, MPI_ANS, MPI_COMM_WORLD, &tmp_request);
				do{
					response();	
					MPI_Test (&tmp_request, &tmp_flag, &tmp_status);

				}while (!tmp_flag);

				
				if (answer == 0 ){			//jeśli ktoś nie zablokował tzn, że jest niedostepny jakiś zasób
					for ( j = 0; j <= i; j++){
						if ( j == mpi_rank )
							continue;

						MPI_Issend (&our_group, 2, MPI_INT, i, MPI_UNBLOCK, MPI_COMM_WORLD, &tmp_request);//odblokowuje
						do{
							response();
							MPI_Test (&tmp_request, &tmp_flag, &tmp_status);

						}while (!tmp_flag);
					}

					printf("%i: \tnieudane rozpoczęcie wykładu: %i nie udostępnił zasobów! (%i:%i)\n", mpi_rank, i, our_group[A_GROUP_YETI], our_group[A_GROUP_ROOM]);
					block_error = true;
					break;
				}

			}


			if (!block_error){
				yeti[our_group[A_GROUP_YETI]][A_YETI_STAN] = WORKING;
				room[our_group[A_GROUP_ROOM]] = WORKING;
				for (i = 0; i < mpi_size; i++){
					if ( i == mpi_rank )
						continue;

					MPI_Issend (&our_group, 2, MPI_INT, i, MPI_GET, MPI_COMM_WORLD, &tmp_request);//zajmuje zasoby
					do{
						response();
						MPI_Test (&tmp_request, &tmp_flag, &tmp_status);

					}while (!tmp_flag);
				}

				lectures[active_lectures_count][A_L_TIMESTAMP] = timestamp + LECTURE_TIME;
				lectures[active_lectures_count][A_L_YETI] = our_group[A_GROUP_YETI];
				lectures[active_lectures_count][A_L_ROOM] = our_group[A_GROUP_ROOM];

				active_lectures_count = active_lectures_count + 1;


				printf("%i: \trozpoczęto wykład! (%i:%i)\n", mpi_rank, our_group[A_GROUP_YETI], our_group[A_GROUP_ROOM]);
			}else{

				yeti[our_group[A_GROUP_YETI]][A_YETI_STAN] = FREE;
				room[our_group[A_GROUP_ROOM]] = FREE;
				free_yetis = free_yetis + 1;
				free_rooms + 1;
				free_projectors = free_projectors + 1;

				//printf("%i: nie udało się rozpocząć wykładu!\n", mpi_rank);
			}
		}
	}

	next_new_lecture_time = timestamp + (rand() % (MAX_SLEEP_BETWEENS_ATTEMPS - MIN_SLEEP_BETWEENS_ATTEMPS)) + MIN_SLEEP_BETWEENS_ATTEMPS;

}


int main(int argc, char **argv){
	int i;

	for ( i = 0; i < YETI_NUMBERS; i++ ){
		yeti[i][A_YETI_STAN] = FREE;
		yeti[i][A_YETI_POWER] = MAX_POWER;
	} 

	for ( i = 0; i < ROOM_NUMBERS; i++ ){
		room[i] = FREE;
	} 

	MPI_Init(&argc, &argv);

	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);


	srand(time(NULL)+mpi_rank);
	//printf("%i: Ready!\n", mpi_rank);

	MPI_Irecv (ask_buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ASK, MPI_COMM_WORLD, &ask_request);
	MPI_Irecv (unblock_buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_UNBLOCK, MPI_COMM_WORLD, &unblock_request);
	MPI_Irecv (get_buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_GET, MPI_COMM_WORLD, &get_request);
	MPI_Irecv (&unblock_room_buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_UNBLOCK_ROOM, MPI_COMM_WORLD, &unblock_room_request);
	MPI_Irecv (unblock_yeti_buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_UNBLOCK_YETI, MPI_COMM_WORLD, &unblock_yeti_request);
	

	
	while (1){
		timestamp = time(NULL);

		response();


		while ( timestamp > lectures[0][A_L_TIMESTAMP] && active_lectures_count > 0){
			end_lecture();
			printf("%i: \tzakończono wykład\n", mpi_rank);
		}

		while ( timestamp > meditation_room[0][A_TIMESTAMP] && meditator_yetis > 0){
			yeti[meditation_room[0][A_YETI_ID]][A_YETI_POWER] = MAX_POWER;
			give_back_yeti(meditation_room[0][A_YETI_ID]);

			printf("%i: \tMistrz wrócił z odpoczynku! (%i)\n", mpi_rank, meditation_room[0][A_YETI_ID]);

			for (i = 1; i <= meditator_yetis - 1; i++){
				meditation_room[i-1][A_YETI_ID] = meditation_room[i][A_YETI_ID];
				meditation_room[i-1][A_TIMESTAMP] = meditation_room[i][A_TIMESTAMP];

			}
			lectures[meditator_yetis - 1][A_L_TIMESTAMP] = 0;
			meditator_yetis = meditator_yetis - 1;	
		}

		while ( timestamp > next_new_lecture_time ){

			new_lecture();
		}


fflush(0);
		usleep(150);
	}

    MPI_Finalize();
    return 0;
}

