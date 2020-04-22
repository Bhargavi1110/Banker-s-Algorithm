#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<windows.h>

//global resources
int i = 0;
int j = 0;

//data structures to hold information about resources allocated to a process
int noofResources;
int noofProcesses;
int *availResourceVector;
int **allocMatrix;
int **maxMatrix;
int **needMatrix;
pthread_mutex_t mutex; //mutex lock for access to global variable

//function declaration
int requestResource(int processID,int requestVector[]);
int releaseResource(int processID,int releaseVector[]);
int ifGreaterThanNeed(int processID,int requestVector[]);
int ifEnoughToRelease(int processId, int releaseVector[]);
int ifInSafeMode();
int ifEnoughToAlloc(int []);
void printNeedMatrix();
void printAllocMatrix();
void printAvailable();
void printReqOrRelVector(int vec[]);
void getInputs();
void scanReqorRelVector(int *vec);
void *master(void* ID);

int main(){//main function

    getInputs();

    printf("Initial Available Vector:\n");
	printAvailable();
	printf("Initial Allocation Matrix:\n");
	printAllocMatrix();
	printf("Initial Need Matrix:\n");
	printNeedMatrix();

	//Multi Threading Setup
	pthread_mutex_init(&mutex,NULL);
	pthread_attr_t attrDefault;
	pthread_attr_init(&attrDefault);
	pthread_t *tid = (pthread_t*)malloc(sizeof(pthread_t) * noofProcesses);
	int *pid = (int*)malloc(sizeof(int)*noofProcesses);//Process's ID
	//initialize pid and create threads
	for(i = 0; i < noofProcesses; i++){
		*(pid + i) = i;
		pthread_create((tid+i), &attrDefault, master, (pid+i));
	}
	//join threads
	for(i = 0; i < noofProcesses; i++){
		pthread_join(*(tid+i),NULL);
	}
	return 0;
}

void *master(void* ID){//function that simualtes request and release of resorces
	int processID = *(int*)ID;//get process id
	int counter = 2;
	while(counter--){
		Sleep(1);
		int requestVector[noofResources];//request random number of resources
		pthread_mutex_lock(&mutex);//lock mutex for accessing global variable and printf
        printf("\nEnter resources to be requested by Process %d: ", processID);
        scanReqorRelVector(requestVector);
		printf("Process %d Requesting Resources:\n",processID);
		printReqOrRelVector(requestVector);
		requestResource(processID,requestVector);
		pthread_mutex_unlock(&mutex);//unlock mutex

		//release random number of resources
		Sleep(1);
		int releaseVector[noofResources];
		//lock mutex for accessing global variable and printf
		pthread_mutex_lock(&mutex);
		//initialize releaseVector
		for(i = 0; i < noofResources; i++){
			if(allocMatrix[processID][i] != 0){
				releaseVector[i] = rand() % allocMatrix[processID][i];
			}
			else{
				releaseVector[i] = 0;
			}
		}
		printf("\nProcess %d Releasing Resources:\n",processID);
		printReqOrRelVector(releaseVector);
		releaseResource(processID,releaseVector);

		pthread_mutex_unlock(&mutex);//unlock mutex
	}
}

int requestResource(int processID,int requestVector[]){//allocate resources to a process
	//whether request number of resources is greater than needed
	if (ifGreaterThanNeed(processID,requestVector) == -1){
		printf("requested resources bigger than needed.\n");
		return -1;
	}
	printf("Simulating Allocation..\n");

	//whether request number of resources is greater than needed
	if(ifEnoughToAlloc(requestVector) == -1){
		printf("Not enough resources\n");
		return -1;
	}

	//pretend allocated
	for (i = 0; i < noofResources; ++i){
		needMatrix[processID][i] -= requestVector[i];
		allocMatrix[processID][i] += requestVector[i];
		availResourceVector[i] -= requestVector[i];
	}

	//check if still in safe status
	if (ifInSafeMode() == 0){
		printf("SAFE. ALLOCATED SUCCESSFULLY.\nAvailable Resources:\n");
		printAvailable();
		printf("Allocated matrix:\n");
		printAllocMatrix();
		printf("Need matrix:\n");
		printNeedMatrix();
		return 0;
	}
	else{
		printf("ROLLING BACK\n");
		for (i = 0; i < noofResources; ++i){
			needMatrix[processID][i] += requestVector[i];
			allocMatrix[processID][i] -= requestVector[i];
			availResourceVector[i] += requestVector[i];
		}
		return -1;
	}
}

int releaseResource(int processID,int releaseVector[]){//release the resources allocated
	if(ifEnoughToRelease(processID,releaseVector) == -1){//to a process
		printf("Process do not own enough resources\n");
		return -1;
	}

	//enough to release
	for(i = 0; i < noofResources; i++){
		allocMatrix[processID][i] -= releaseVector[i];
		needMatrix[processID][i] += releaseVector[i];
		availResourceVector[i] += releaseVector[i];
	}
	printf("Release successfully.\nAvailable Resources:\n");
	printAvailable();
	printf("Allocated matrix:\n");
	printAllocMatrix();
	printf("Need matrix:\n");
	printNeedMatrix();
	return 0;
}

int ifEnoughToRelease(int processID,int releaseVector[]){//check if released resources
	for (i = 0; i < noofResources; ++i){				 //are more than allocated
		if (releaseVector[i] <= allocMatrix[processID][i]){
			continue;
		}
		else{
			return -1;
		}
	}
	return 0;
}

int ifGreaterThanNeed(int processID,int requestVector[]){//check if the requested resorces
	for (i = 0; i < noofResources; ++i){				 //are more than needed
		if (requestVector[i] <= needMatrix[processID][i]){
			continue;
		}
		else{
			return -1;
		}
	}
	return 0;
}

int ifEnoughToAlloc(int requestVector[]){//check if there are enough resources to allocate
	for (i = 0; i < noofResources; ++i){
		if (requestVector[i] <= availResourceVector[i]){
			continue;
		}
		else{
			return -1;
		}
	}
	return 0;
}

void getInputs() {

    printf("Enter number of unique processes and resources: \n");
    printf("Processes: ");
    scanf("%d", &noofProcesses);
    printf("Resources: ");
    scanf("%d", &noofResources);

    availResourceVector  = (int *)  malloc(noofResources*sizeof(int));
    allocMatrix          = (int **) malloc(noofProcesses*sizeof(int *));
    maxMatrix            = (int **) malloc(noofProcesses*sizeof(int *));
    needMatrix           = (int **) malloc(noofProcesses*sizeof(int *));

    for(int i = 0;i < noofProcesses;i++) {
        allocMatrix[i]       = (int *) malloc(noofResources*sizeof(int));
        maxMatrix[i]         = (int *) malloc(noofResources*sizeof(int));
        needMatrix[i]        = (int *) malloc(noofResources*sizeof(int));
    }

    printf("\nEnter the number of available entities of each resource: \n");
    for(int i=0; i<noofResources; i++) {
        printf("Resource %d: ", i+1);
        scanf("%d", &availResourceVector[i]);
    }

    printf("\n\nEnter the number of resources of each type MAXIMUM NEEDED for each process: \n");
    for(int i=0; i<noofProcesses; i++){
        printf("Process %d: \n", i);
        for(int j=0; j<noofResources; j++) {
            printf("Resource %d: ", j+1);
            scanf("%d", &maxMatrix[i][j]);
        }
        printf("\n");
    }

    printf("\n\nEnter the number of resources of each type ALLOCATED for each process: \n");
    for(int i=0; i<noofProcesses; i++){
        printf("Process %d: \n", i);
        for(int j=0; j<noofResources; j++) {
            printf("Resource %d: ", j+1);
            scanf("%d", &allocMatrix[i][j]);
        }
        printf("\n");
    }

	//initialize needMatrix
	for (i = 0; i < noofProcesses; ++i){
		for (j = 0; j < noofResources; ++j){
			needMatrix[i][j] = maxMatrix[i][j] - allocMatrix[i][j];
		}
	}

}

void printNeedMatrix(){//print need matrix
	for (i = 0; i < noofProcesses; ++i){
		printf("{ ");
		for (j = 0; j < noofResources; ++j){
			printf("%d, ", needMatrix[i][j]);
		}
		printf("}\n");
	}
	return;
}

void printAllocMatrix(){//print Allocation Matrix
	for (i = 0; i < noofProcesses; ++i){
		printf("{ ");
		for (j = 0; j < noofResources; ++j){
			printf("%d, ", allocMatrix[i][j]);
		}
		printf("}\n");
	}
	return;
}

void printAvailable(){//print number of available resources
    printf("{ ")
	for (i = 0; i < noofResources; ++i){
		printf("%d, ",availResourceVector[i]);
	}
	printf("}\n");
	return;
}

void printReqOrRelVector(int vec[]){//print Request/Release Vector
    printf("{ ");
	for (i = 0; i < noofResources; ++i){
		printf("%d, ",vec[i]);
	}
	printf("}\n");
	return;
}

void scanReqorRelVector(int* vec) {

    for(i = 0; i < noofResources; i++) {
        printf("Resource %d: ", i+1);
        scanf("%d", vec+i);
    }

}

int ifInSafeMode(){//checks if a safe sequence exists if resource allocation occurs
	int *ifFinish, k;
    ifFinish = (int *) malloc(noofProcesses*sizeof(int));
    memset(ifFinish, 0, noofProcesses*sizeof(int));
	int work[noofResources];//temporary available resources vector
	for(i = 0; i < noofResources; i++)
		work[i] = availResourceVector[i];
	for(i = 0; i < noofProcesses; i++)
		if (ifFinish[i] == 0)
			for(j = 0; j < noofResources; j++)
				if(needMatrix[i][j] <= work[j])
					if(j == noofResources - 1){
						ifFinish[i] = 1;
						for (k = 0; k < noofResources; ++k)
							work[k] += allocMatrix[i][k];
							//execute and release resources
						i = -1;
						break;
					}
					else
						continue;
				else
					break;
		else
			continue;
	for(i = 0; i < noofProcesses; i++)
		if (ifFinish[i] == 0)
			return -1;
		else
			continue;
	return 0;
}
