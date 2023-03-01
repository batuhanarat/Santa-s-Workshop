#include "queue.c"
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

int simulationTime = 120;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 30; // frequency of emergency gift requests from New Zealand
int taskID = 1;

void* ElfA(void *arg); // the one that can paint
void* ElfB(void *arg); // the one that can assemble
void* Santa(void *arg); 
void* ControlThread(void *arg); // handles printing and queues (up to you)
Task* generateTask();

// pthread sleeper function
int pthread_sleep (int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;
    
    pthread_mutex_lock(&mutex);
    int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);
    
    //Upon successful completion, a value of zero shall be returned
    return res;
}

int main(int argc, char *argv[]) {
    // -t (int) => simulation time in seconds
    // -s (int) => change the random seed
    for (int ii = 1; ii < argc; ii++){
        if (!strcmp(argv[ii], "-t")) { 
		simulationTime = atoi(argv[++ii]);
	}
        else if (!strcmp(argv[ii], "-s")) {
		seed = atoi(argv[++ii]);
	}
    }
    srand(seed);
   
    Queue* packaging = ConstructQueue(1000);
    Queue* painting = ConstructQueue(1000);
    Queue* assembly = ConstructQueue(1000);
    Queue* qa = ConstructQueue(1000);
    Queue* delivery = ConstructQueue(1000);
    
    Queue *elfA[] = {packaging, painting, delivery};
    Queue *elfB[] = {packaging, assembly, delivery};
    Queue *santa[] = {delivery, qa, packaging};

    struct timeval startTime;
    struct timeval currentTime;

    gettimeofday(&startTime, NULL);
    
    pthread_t threads[3]; 
    pthread_create(&threads[0], NULL, ElfA, (void *) elfA);
    pthread_create(&threads[1], NULL, ElfB, (void *) elfB);
    pthread_create(&threads[2], NULL, Santa, (void *) santa);

    while (true) {
	
	gettimeofday(&currentTime, NULL);
	if ((currentTime.tv_sec - startTime.tv_sec) > simulationTime) { 
		break;
	}
	Task* taskP = generateTask();
	
	if (taskP == NULL) {
		printf("[Main] Registering no task!\n");
	}
	else if (taskP->type == 1) {	
		pthread_mutex_lock(&packaging->lock);
		char *string = "[Main] Registering (Type 1) Task!"; 
		printf("%-40s Gift ID: %d\n", string, taskP->ID);
		Enqueue(packaging, taskP);
		pthread_mutex_unlock(&packaging->lock);
	}
	else if (taskP->type == 2) {	
		pthread_mutex_lock(&painting->lock);
		char *string = "[Main] Registering (Type 2) Task!";
		printf("%-40s Gift ID: %d\n", string, taskP->ID);
		Enqueue(painting, taskP);
		pthread_mutex_unlock(&painting->lock);
	}
	else if (taskP->type == 3) {
		pthread_mutex_lock(&assembly->lock);
		char *string = "[Main] Registering (Type 3) Task!"; 
		printf("%-40s Gift ID: %d\n", string, taskP->ID);
		Enqueue(assembly, taskP);
		pthread_mutex_unlock(&assembly->lock);
	}
	else if (taskP->type == 4) {
		pthread_mutex_lock(&painting->lock);
		char *string = "[Main] Registering (Type 4) Task!";
		printf("%-40s Gift ID: %d\n", string, taskP->ID);
		Enqueue(painting, taskP);
		pthread_mutex_unlock(&painting->lock);
		
		pthread_mutex_lock(&qa->lock);
		Enqueue(qa, taskP);
		pthread_mutex_unlock(&qa->lock);
	}
	else {
		pthread_mutex_lock(&assembly->lock);
		char *string = "[Main] Registering (Type 5) Task!";
		printf("%-40s Gift ID: %d\n", string, taskP->ID);
		Enqueue(assembly, taskP);
		pthread_mutex_unlock(&assembly->lock);

		pthread_mutex_lock(&qa->lock);
		Enqueue(qa, taskP);
		pthread_mutex_unlock(&qa->lock);
	}
	pthread_sleep(1);
    }

    exit(0);
        
    /* Queue usage example
        Queue *myQ = ConstructQueue(1000);
        Task t;
        t.ID = myID;
        t.type = 2;
        Enqueue(myQ, t);
        Task ret = Dequeue(myQ);
        DestructQueue(myQ);
    */

    // your code goes here
    // you can simulate gift request creation in here, 
    // but make sure to launch the threads first

    return 0;
}

void* ElfA(void *arg) {
	
	Queue **arr = (Queue**) arg;

	Queue *packaging = arr[0];
	Queue *painting = arr[1];
	Queue *delivery = arr[2];

	while (true) {
		pthread_mutex_lock(&packaging->lock); 
		if (isEmpty(packaging) == true) {
			pthread_mutex_unlock(&packaging->lock); 
			pthread_mutex_lock(&painting->lock);
			if (isEmpty(painting) == true) {
				pthread_mutex_unlock(&painting->lock);
			}
			else {
                		pthread_sleep(3);	
				Task *taskP = Dequeue(painting);
				char *string = "[Elf A] Painting wooden toy!";	
				printf("%-40s Gift ID: %d\n", string, taskP->ID);
				pthread_mutex_unlock(&painting->lock);               		
         			if (taskP->type == 2) {
                                	pthread_mutex_lock(&packaging->lock);
                                	Enqueue(packaging, taskP);
                                	pthread_mutex_unlock(&packaging->lock);              
				}
				else { // Task Type 4
					pthread_mutex_lock(&taskP->lock);
					taskP->elfWork = true;
					if (taskP->santaWork == true) {
						pthread_mutex_lock(&packaging->lock);
						Enqueue(packaging, taskP); 
						pthread_mutex_unlock(&packaging->lock);
					}
					pthread_mutex_unlock(&taskP->lock);             
				}	
			}	
		}
		else {
                        pthread_sleep(1);	                                              
                        Task *taskP = Dequeue(packaging);
			char *string = "[Elf A] Packaging present!";
			printf("%-40s Gift ID: %d\n", string, taskP->ID);
                        pthread_mutex_unlock(&packaging->lock);
                                                                              
                        pthread_mutex_lock(&delivery->lock);
                        Enqueue(delivery, taskP);
                        pthread_mutex_unlock(&delivery->lock);                        
		}
	}
}

void* ElfB(void *arg) {

	Queue **arr =  (Queue**) arg;

	Queue *packaging = arr[0];
	Queue *assembly = arr[1];
	Queue *delivery = arr[2];

	while (true) {
        	pthread_mutex_lock(&packaging->lock); 
		if (isEmpty(packaging) == true) {
			pthread_mutex_unlock(&packaging->lock);                       	
        		pthread_mutex_lock(&assembly->lock);
        		if (isEmpty(assembly) == true) {
        			pthread_mutex_unlock(&assembly->lock);
			}
			else {
                		pthread_sleep(2);	
        			Task *taskP = Dequeue(assembly);
				char *string = "[Elf B] Assembling plastic toy!"; 
        			printf("%-40s Gift ID: %d\n", string, taskP->ID);
        			pthread_mutex_unlock(&assembly->lock);                
         			if (taskP->type == 3) {
                                	pthread_mutex_lock(&packaging->lock);
                                	Enqueue(packaging, taskP);
                                	pthread_mutex_unlock(&packaging->lock);       
        			}
        			else { // Task Type 5
					pthread_mutex_lock(&taskP->lock);
                                        taskP->elfWork = true;	
                                        if (taskP->santaWork == true) {
                                        	pthread_mutex_lock(&packaging->lock);
                                        	Enqueue(packaging, taskP); 
                                        	pthread_mutex_unlock(&packaging->lock);
                                        }
                                        pthread_mutex_unlock(&taskP->lock);           
        			}
			}
        	}
        	else {
                        pthread_sleep(1);	                                     
                        Task *taskP = Dequeue(packaging);
			char *string = "[Elf B] Packaging present!";
        		printf("%-40s Gift ID: %d\n", string, taskP->ID);
                        pthread_mutex_unlock(&packaging->lock);
                                                                              
                        pthread_mutex_lock(&delivery->lock);
                        Enqueue(delivery, taskP);
                        pthread_mutex_unlock(&delivery->lock);                    
		}
	}
		
}

// manages Santa's tasks
void* Santa(void *arg) {

	Queue **arr = (Queue **) arg;

	Queue *delivery = arr[0];
	Queue *qa = arr[1];
	Queue *packaging = arr[2];


	while (true) {
        	pthread_mutex_lock(&delivery->lock); 
        	if (isEmpty(delivery) == true) {
        		pthread_mutex_unlock(&delivery->lock); 
        		pthread_mutex_lock(&qa->lock);
        		if (isEmpty(qa) == true) {
        			pthread_mutex_unlock(&qa->lock);
			}
			else {
                		pthread_sleep(1);	
        			Task *taskP = Dequeue(qa);
				char *string = "[Santa] Doing quality assurance!";
        			printf("%-40s Gift ID: %d\n", string, taskP->ID);
        			pthread_mutex_unlock(&qa->lock);
				
				pthread_mutex_lock(&taskP->lock);
				taskP->santaWork = true;
				if (taskP->elfWork == true) {
					pthread_mutex_lock(&packaging->lock);
					Enqueue(packaging, taskP);
					pthread_mutex_unlock(&packaging->lock);
				}
				pthread_mutex_unlock(&taskP->lock);

			}	
        	}
        	else {
                        pthread_sleep(1);	                                     
                        Task *taskP = Dequeue(delivery);
			char *string = "[Santa] Delivering present!"; 
        		printf("%-40s Gift ID: %d\n", string, taskP->ID);
                        pthread_mutex_unlock(&delivery->lock);                                      
                }        
	}		
}

// the function that controls queues and output
void* ControlThread(void *arg){

}

Task* generateTask() {
	int randNum = rand() % 20;
	Task *taskP = (Task *) malloc(sizeof(Task));

	pthread_mutex_init(&taskP->lock, NULL);

	if (randNum == 0 || randNum == 1) {
		return NULL;
	}
	else if (1 < randNum && randNum < 10) {
		taskP->type = 1; 
	}
	else if (9 < randNum && randNum < 14) {
		taskP->type = 2; 
	}
	else if (13 < randNum && randNum < 18) {
		taskP->type = 3;
	}
	else if (randNum == 18) {
		taskP->type = 4;
	}
	else {
		taskP->type = 5;
	}

	taskP->ID = taskID;
	taskP->santaWork = false;
	taskP->elfWork = false;
	taskID++;
	return taskP;
}
