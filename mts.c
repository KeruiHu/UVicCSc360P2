/*
	Student Name: Xiaotian Li
	Student Number: V00786924
	CSC360 P2
*/

#include <semaphore.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>  


pthread_mutex_t track = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue = PTHREAD_MUTEX_INITIALIZER;


struct timeval start;
struct train{
	int number;
	int loadTime;
	int crossTime;
	int priority;
	char direction[5];
};
struct train lastTrain;

struct queueNode{
	struct train aTrain;
	struct queueNode *next;
};

struct queueNode *queueRoot;
int onTrack = -1;
int queueNumber = 0;
struct train tokenize(char* string);
float intveral();
void *mainTrack(void* train);
void addToQueue(struct train);
int compareTrain(struct queueNode*, struct queueNode*, struct queueNode*);




//Add the node to the waiting queue. 
void addToQueue(struct train currentTrain)
{

	struct queueNode *newNode, *conductor;
	newNode = malloc(sizeof(struct queueNode));
	newNode->aTrain = currentTrain;
	newNode->next = NULL;
	if(queueRoot == NULL)
	{
		queueRoot = newNode;
		queueRoot->next = NULL;
	}else{
		struct queueNode *current, *previous, *temp;
		current = queueRoot;
		previous = NULL;
		while(current != NULL)
		{
			//if need to exchange to nodes
			if(compareTrain(newNode, current, previous)){
				//if the root need to be exchanged.
				if(previous == NULL){
					newNode->next = queueRoot;
					queueRoot = newNode;
					break;
				}else{
					previous->next = newNode;
					newNode->next = current;
					temp = current->next;
					//check the node after the newNode
					//if currentNode has the same direction with newNode &&
					//the node after currentNode also has the same direction with newNode
					//Change the position.
					if(temp != NULL){
						while(!strcmp(newNode->aTrain.direction, current->aTrain.direction) && strcmp(current->aTrain.direction, temp->aTrain.direction))
						{	
								newNode->next = temp;
								if(temp->next == NULL){
									current->next = NULL;
								}else{
									current->next = temp->next;
								}
								temp->next = current;
								newNode = current;
								if(newNode->next == NULL || newNode->next->next == NULL){
									break;
								}else{
									current = newNode->next;
									temp = current->next;	
								}
						}
					}
					break;
				}
			}else{
				if(current->next != NULL){
					previous = current;
					current = current->next;	
				}else{
					current->next = newNode;
					newNode->next = NULL;
					break;
				}
			}

		}
	}
	queueNumber++;
	
}


//Compare two nodes with the specific requirement. 
int compareTrain(struct queueNode *newNode, struct queueNode *current, struct queueNode *previous)
{
	if(newNode->aTrain.priority > current->aTrain.priority) //newNode's priority  < currentNode priority
	{
		
		return 1;
	}else if(newNode->aTrain.priority == current->aTrain.priority){

		
		if(strcmp(newNode->aTrain.direction, current->aTrain.direction) == 0)
		{
			if(newNode->aTrain.loadTime < current->aTrain.loadTime){
				return 1;
			}else if(newNode->aTrain.loadTime == current->aTrain.loadTime){
				if(newNode->aTrain.number < current->aTrain.number){
					return 1;
				}
			}else{
				return 0;
			}
		}else{
			
			if(previous == NULL){
				if(!strcmp(newNode->aTrain.direction,"w") || !strcmp(newNode->aTrain.direction,"W")){
					return 1;
				}else{
					return 0;
				}
			}else{
				if(strcmp(newNode->aTrain.direction, previous->aTrain.direction) != 0){
					return 1;
				}	
				else{return 0;}
			}
			
		}
	}else{
		return 0;
	}
	return 0;
}


//To tokenize the input file.
struct train tokenize(char* string)
{
	struct train currentTrain;
	char *token = strtok(string, ":");
	//printf("%c\n", *(char *)token);
	switch(*token){
		case 'e':
			strcpy(currentTrain.direction,"East");
			currentTrain.priority = 0;
			break;
		case 'E':
			strcpy(currentTrain.direction,"East");
			currentTrain.priority = 1;
			break;
		case 'w':
			strcpy(currentTrain.direction,"West");
			currentTrain.priority = 0;
			break;
		case 'W':
			strcpy(currentTrain.direction,"West");
			currentTrain.priority = 1;
			break;
		default:
			strcpy(currentTrain.direction,"Wrong");
			currentTrain.priority = -1;
			break;
	}
	int trainData[4];
	int i = 0;
	while(token)
	{
		trainData[i++] = atoi(token);
		token = strtok(NULL, ",");	
	}
	currentTrain.loadTime = trainData[1];
	currentTrain.crossTime = trainData[2];
	currentTrain.number = trainData[3];
	//printf("LoadTime: %d\n", currentTrain.loadTime);
	// printf("crossTime: %d\n", currentTrain.crossTime);
	// printf("priority: %d\n", currentTrain.priority);
	// printf("direction: %s\n", currentTrain.direction);
	// printf("Number: %d\n\n", currentTrain.number);

	return currentTrain;
}

//Use this method to get interval. 
float interval()
{
	struct timeval curTime;
	gettimeofday(&curTime, NULL);
	float interval = (curTime.tv_sec - start.tv_sec)+((curTime.tv_usec - start.tv_usec)/1000000.0);
	return interval;
}


//The main method for thread. 
void *mainTrack(void *train)
{

	struct train currentTrain = *(struct train *)train;
	
	//printf("%s", trainBuffer);
	usleep(currentTrain.loadTime * 1E5F);
	printf("00:00:0%0.1f Train %2d is ready to go %4s\n", interval(), currentTrain.number, currentTrain.direction);

	//add the train to queue. 
	//in the waiting queue, the train will wait for signal to cross the track. 
	pthread_mutex_lock(&queue);
	addToQueue(currentTrain);

	//Following if statement for debugging. 
	if(0){
		
		struct queueNode *conductor;
		conductor = queueRoot;

		while(conductor != NULL){
			printf("I am #%d \n", currentTrain.number);
			printf("%d: %d, %d\n", conductor->aTrain.number, conductor->aTrain.loadTime, conductor->aTrain.crossTime);
			conductor = conductor->next;
		}
	}
	
	pthread_mutex_unlock(&queue);
	//let thread to sleep a lit bit time,
	//because it has to wait the other thread which they finish loading at the same time.
	usleep(0.3 * 1E5F);

	if(onTrack == -1 && queueNumber == 1){
		pthread_mutex_lock(&track);
		onTrack = currentTrain.number;
		pthread_mutex_unlock(&track);
		printf("00:00:0%0.1f Train %2d is ON the main track going %4s\n", interval(), currentTrain.number, currentTrain.direction);
		usleep(currentTrain.crossTime * 1E5F);
		printf("00:00:0%0.1f Train %2d is OFF the main track going %4s\n", interval(), currentTrain.number, currentTrain.direction);
	}else
	{
		if(onTrack == -1){
			onTrack = queueRoot->aTrain.number;
		}
		while(onTrack != currentTrain.number)
		{ // wait for a siganl which allow current train to cross
			//printf("I am waiting, my  number is %d\n",currentTrain.number);
		}

		pthread_mutex_lock(&track);
		onTrack = currentTrain.number;
		pthread_mutex_unlock(&track);
		printf("00:00:0%0.1f Train %2d is ON the main track going %4s\n", interval(), currentTrain.number, currentTrain.direction);
		usleep(currentTrain.crossTime * 1E5F);
		printf("00:00:0%0.1f Train %2d is OFF the main track going %4s\n", interval(), currentTrain.number, currentTrain.direction);
	}
	
	pthread_mutex_lock(&track);
	pthread_mutex_lock(&queue);
	//delelte the root node in the queue. 
	if(queueRoot->next == NULL){
		onTrack = -1;
	}else{
	//delete the currentTrain node
		struct queueNode *deleteNode, *preDeleteNode;
		if(queueRoot->aTrain.number == currentTrain.number){
			deleteNode = queueRoot;
			queueRoot = queueRoot->next;
			free(deleteNode);
			onTrack = queueRoot->aTrain.number;
		}else{
			deleteNode = queueRoot;
			while(deleteNode != NULL){
				if(deleteNode->aTrain.number == currentTrain.number){
					preDeleteNode->next = deleteNode->next;
					free(deleteNode);
					onTrack = queueRoot->aTrain.number;
					break;
				}else{
					preDeleteNode = deleteNode;
					deleteNode = deleteNode->next;
				}
			}
		}
		//printf("ROOT track number: %d\n", queueRoot->aTrain.number);
		//printf("On track number: %d\n", onTrack);
	}
	
	pthread_mutex_unlock(&queue);
	pthread_mutex_unlock(&track);
	
	
	pthread_exit(NULL);
}



int main(int argc, char* argv[]){
	FILE *inputFile;
	char buffer[10];
	int i = 0;
	if(argc < 3)
	{
		fprintf(stderr, "Please enter two parameters. One for the input file, another one for the number of trains\n");
		return 0;
	}
	else
	{
		inputFile = fopen(argv[1], "r");
	}

	if(inputFile == NULL)
	{
		fprintf(stderr,"can not open the file\n");
		return 0;
	}

	long number_train = atoi(argv[2]);
	pthread_t tid[number_train];
	pthread_t scheduler;
	char *trainBuffer[number_train];
	struct train trainList[number_train];
	queueRoot = malloc(sizeof(struct queueNode));
	queueRoot = NULL;

	for(i = 0; i < number_train; i++)
	{
		fgets(buffer,10,inputFile);
		trainBuffer[i] = (char*)malloc(sizeof(char) * 20);
		strcpy(trainBuffer[i],buffer);
		strcat(trainBuffer[i], ",");
		char noTrain[3];
		sprintf(noTrain, "%d", i);
		strcat(trainBuffer[i], noTrain);
		trainList[i] = tokenize(trainBuffer[i]);
	}

	fclose(inputFile);
	

	gettimeofday(&start, NULL);

	for(i = 0; i < number_train; i++)
	{
		pthread_create(&tid[i], NULL, mainTrack, &trainList[i]);
	}

	for(i = 0; i < number_train; i++)
	{
		pthread_join(tid[i], NULL);
		
	}


	free(queueRoot);


	//the following part used for debugging.
	//To print out the waiting queue. 
	if(0){
		struct queueNode *conductor;
		conductor = queueRoot;
		int i = 0;
		for(i = 0; i < number_train; i++){
			printf("%d: %d, %d\n", conductor->aTrain.number, conductor->aTrain.loadTime, conductor->aTrain.crossTime);
			conductor = conductor->next;
		}
	}
	
	
	return 0;
}