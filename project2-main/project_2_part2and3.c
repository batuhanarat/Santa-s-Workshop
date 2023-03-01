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
int counter = 1 ; // timer 
bool flag =false ;
char *string; 
int snapshotTime = 0;    
int giftid = 1;
int taskID = 0;
int filler =0;

       


void* ElfA(void *arg); // the one that can paint
void* ElfB(void *arg); // the one that can assemble
void* Santa(void *arg); 
void* ControlThread(void *arg); // handles printing and queues (up to you)
Task* generateTask();
void createLog();
void logTask(Task* task);


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
	
	else if(!strcmp(argv[ii], "-n"))  {
		snapshotTime = atoi(argv[++ii]);
	}
	
    }
    srand(seed);
   
    Queue* packaging = ConstructQueue(1000);
    Queue* painting = ConstructQueue(1000);
    Queue* assembly = ConstructQueue(1000);
    Queue* qa = ConstructQueue(1000);
    Queue* delivery = ConstructQueue(1000);
    createLog();
    
    
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
	

	
	flag = (currentTime.tv_sec - startTime.tv_sec) > (counter * 30) ;
	
	 
	Task* taskP = generateTask();
	
	
	
	
	 if (taskP->type == 1) {	
		pthread_mutex_lock(&packaging->lock);
		
		if(flag) {
		EnqueueWithBeginning(packaging,taskP);
		counter++;
		flag = false;
		
		 string = "[Main] Registering Special! (Type 1) Task!"; 
		} else {
		
		Enqueue(packaging, taskP);
		 string = "[Main] Registering (Type 1) Task!"; 
		}

        	 printf("%-40s Gift ID: %d\n", string, taskP->giftID);

		pthread_mutex_unlock(&packaging->lock);
	}
	else if (taskP->type == 2) {	
		pthread_mutex_lock(&painting->lock);
		
		
		if(flag) {
		EnqueueWithBeginning(painting,taskP);
		counter++;
		flag = false;
		 string = "[Main] Registering Special! (Type 2) Task!";
		} else {
		
		Enqueue(painting, taskP);
		 string = "[Main] Registering (Type 2) Task!"; 
		
		}

        	 printf("%-40s Gift ID: %d\n", string, taskP->giftID);
		pthread_mutex_unlock(&painting->lock);
	}
	else if (taskP->type == 3) {
		pthread_mutex_lock(&assembly->lock);
		
		if(flag) {
		EnqueueWithBeginning(assembly,taskP);
		counter++;
		flag = false;
		 string = "[Main] Registering Special! (Type 3) Task!";
		} else {
		
		Enqueue(assembly, taskP);
		string = "[Main] Registering (Type 3) Task!"; 
		}
		
		

                printf("%-40s Gift ID: %d\n", string, taskP->giftID);
		pthread_mutex_unlock(&assembly->lock);
	}
	else if (taskP->type == 4) {
		pthread_mutex_lock(&painting->lock);
		
		if(flag) {
		EnqueueWithBeginning(painting,taskP);
		pthread_mutex_unlock(&painting->lock);
		
		pthread_mutex_lock(&qa->lock);
		EnqueueWithBeginning(qa, taskP);
		pthread_mutex_unlock(&qa->lock);
		counter++;
		flag = false;
		 string = "[Main] Registering Special! (Type 4) Task!";
		} else {
		
		Enqueue(painting, taskP);
		pthread_mutex_unlock(&painting->lock);
		
		pthread_mutex_lock(&qa->lock);
		Enqueue(qa, taskP);
		pthread_mutex_unlock(&qa->lock);
		
		string = "[Main] Registering (Type 4) Task!"; 
		}
		
		

        	 printf("%-40s Gift ID: %d\n", string, taskP->giftID);


		
		
	}
	else {
		pthread_mutex_lock(&assembly->lock);
		
		
		if(flag) {
		EnqueueWithBeginning(assembly,taskP);
		pthread_mutex_unlock(&assembly->lock);
		
		pthread_mutex_lock(&qa->lock);
		EnqueueWithBeginning(qa, taskP);
		pthread_mutex_unlock(&qa->lock);
		counter++;
		flag = false;
		 string = "[Main] Registering Special! (Type 5) Task!";
		} else {
		
		Enqueue(assembly, taskP);
		pthread_mutex_unlock(&assembly->lock);
		
		pthread_mutex_lock(&qa->lock);
		Enqueue(qa, taskP);
		pthread_mutex_unlock(&qa->lock);
		
		string = "[Main] Registering (Type 5) Task!"; 
		}
		

		printf("%-40s Task ID: %d\n", string, taskP->ID);
		
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
				taskID++;
				taskP->ID = taskID;
				taskP->taskType = 'P';
				taskP->responsible = 'A';
				char *string = "[Elf A] Painting wooden toy!";	
        			printf("%-40s Gift ID: %d\n", string, taskP->giftID);
				pthread_mutex_unlock(&painting->lock);  
				logTask(taskP);                             		
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
                        taskID++;
			taskP->ID = taskID;
			taskP->taskType = 'C';
			taskP->responsible = 'A';
			char *string = "[Elf A] Packaging present!";
        	         printf("%-40s Gift ID: %d\n", string, taskP->giftID);
                        pthread_mutex_unlock(&packaging->lock);
                        logTask(taskP);                                                                     
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
        			taskID++;
				taskP->ID = taskID;
				taskP->taskType = 'A';
				taskP->responsible = 'B';
				char *string = "[Elf B] Assembling plastic toy!"; 
        			printf("%-40s Gift ID: %d\n", string, taskP->giftID);
        			pthread_mutex_unlock(&assembly->lock);
        			logTask(taskP);                
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
                        taskID++;
			taskP->ID = taskID;
			taskP->taskType = 'C';
			taskP->responsible = 'B';
			char *string = "[Elf B] Packaging present!";
        	        printf("%-40s Gift ID: %d\n", string, taskP->giftID);
                        pthread_mutex_unlock(&packaging->lock);
                        logTask(taskP);                                                                      
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

//(a) No more presents are waiting to be delivered,
//(b) 3 or more GameStations are waiting to go through QA.

	while (true) {
        	pthread_mutex_lock(&qa->lock); 
        	if((qa->size) <3) {
        	    pthread_mutex_unlock(&qa->lock); 
        	
          	
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
        				taskID++;
					taskP->ID = taskID;
					taskP->taskType = 'Q';
					taskP->responsible = 'S';
					char *string = "[Santa] Doing quality assurance!";
        				printf("%-40s Gift ID: %d\n", string, taskP->giftID);
        				pthread_mutex_unlock(&qa->lock);
        				logTask(taskP);                
				
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
                        	taskID++;
				taskP->ID = taskID;
				taskP->taskType = 'D';
				taskP->responsible = 'S';
				char *string = "[Santa] Delivering present!"; 
        			printf("%-40s Gift ID: %d\n", string, taskP->giftID);
                        	pthread_mutex_unlock(&delivery->lock);    
                        	logTask(taskP);                                             
                	}
                
                
                	}  
                	else {
                	
                	pthread_sleep(1);	
        				Task *taskP = Dequeue(qa);
        				taskID++;
					taskP->ID = taskID;
					taskP->taskType = 'Q';
					taskP->responsible = 'S';
					char *string = "[Santa] Doing quality assurance!";
        			        printf("%-40s Gift ID: %d\n", string, taskP->giftID);
        				pthread_mutex_unlock(&qa->lock);
        				logTask(taskP);                
				
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
}

// the function that controls queues and output
void* ControlThread(void *arg){

}

Task* generateTask() {
	int randNum = rand() % 20;
	Task *taskP = (Task *) malloc(sizeof(Task));

	pthread_mutex_init(&taskP->lock, NULL);

	
	 if (-1 < randNum && randNum < 10) {
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

	taskP->giftID = giftid;
	taskP->santaWork = false;
	taskP->elfWork = false;
	giftid++;
	return taskP;
}


void createLog() {

FILE *fp = fopen("events.log", "w");
  	fprintf(fp, "\tTaskID\t\tGiftID\t\tGiftType\tTaskType\tResponsible\n");
  	fprintf(fp, "    -----------------------------------------------------------------------------------------------------------------------\n");
  	fclose(fp);

}





void logTask(Task* task) {
	//job.endTime = time(NULL) - timeZero;
	
	FILE *fp = fopen("events.log", "a");
	fprintf(fp,"\t%d\t\t%d\t\t%d\t\t%c\t\t%c \n",task->ID,task->giftID,task->type,task->taskType,task->responsible);
	
  	
  	fclose(fp);
}









