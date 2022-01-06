/*
 * ex3.c
 *
 *  Created on: May 20, 2021
 *      Author: braude
 */
/*315835645 & 316325927*/

#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define TOK_DELIM "\t\n"

/*structs declaration*/
///////////////////////////////////////////
typedef int Type;
typedef struct resources{
	Type resourceType;
	char* resourceName;
	int resourceAmount;
	sem_t numOfResources;
}resources,*presources;

typedef struct services{
	Type serviceType;
	char* serviceName;
	int hoursPerService;
	int numOfResourcesNeeded;
	Type* arrayOfResourcesNeeded;
}services,*pservices;

typedef struct request{
	int carNumber;
	int timeArrivalHour;
	int numOfServicesNeeded;
	Type* arrayOfServicesNeeded;
}request,*prequest;

typedef struct timeInTheDay{
	int time;
}timeInTheDay;
/////////////////////////////////////////////

/*function declaration*/
///////////////////////////////////////////////////////////////////
presources getResources(const char* filename,int* sizeOfDataBase);
pservices  getServices(const char* filename,int* sizeOfDataBase);
prequest  getRequests(const char* filename,int* sizeOfDataBase);
char** splitLine(char* lineToSplit);
void printErr(const char* msg);
void initTime(timeInTheDay* timer);
void* manageClock();
void* manageCarRequest(void* index);
///////////////////////////////////////////////////////////////////

/*global vars*/
/////////////////////////////////////
presources resourceDataBase;
pservices servicesDataBase;
prequest requestDataBase;
int sizeResourceDataBase=0;
int sizeServicesDataBase=0;
int sizeRequestDataBase=0;
timeInTheDay timer;
///////////////////////////////////////

int main()
{
	int i;
	/*get data from files*/
	////////////////////////////////////////////////////////////////////////
	resourceDataBase=getResources("resources.txt",&sizeResourceDataBase);
	servicesDataBase=getServices("services.txt",&sizeServicesDataBase);
	requestDataBase=getRequests("requests.txt",&sizeRequestDataBase);
	////////////////////////////////////////////////////////////////////////

	/*initialize time in the day*/
	/////////////////////////////////////////////////////////////////////////
	initTime(&timer);
	/////////////////////////////////////////////////////////////////////////


	/*create the threads for each car*/
	////////////////////////////////////////////////////////////////////////

	pthread_t requsted[sizeRequestDataBase],timerThread;
	int arrayOfIndexRequest[sizeRequestDataBase];
	for(i=0;i<sizeRequestDataBase;i++){//put inside the array the indexes
		arrayOfIndexRequest[i]=i;
	}
	pthread_create(&timerThread,NULL,(void*)manageClock,NULL);
	for(i=0;i<sizeRequestDataBase;i++){
	pthread_create(&requsted[i],NULL,(void*)manageCarRequest,&arrayOfIndexRequest[i]);
	}

	for(i=0;i<sizeRequestDataBase;i++){
		pthread_join(requsted[i],NULL);
	}

	exit(1);
	return 1;
}


char** splitLine(char* lineToSplit){
	char **listOfTokens;//this is a string array that will hold all the tokens
	char* token;
	int indexOfTokenInLine=0;
	listOfTokens=(char**)malloc( sizeof(char*));
	if(listOfTokens==NULL){
		printErr("Allocation failed for creating list of tokens from line\n");
	}
	 token = strtok(lineToSplit, TOK_DELIM);//split the line by SUPERSHELL_TOK_DELIM
	 while ((token != NULL)){
		 listOfTokens[indexOfTokenInLine++]=token;//save in the listOfTokens the data
		 if(strcmp(token,"\n")!=0)//if we got '\n'
		 {
		 listOfTokens = realloc(listOfTokens, (indexOfTokenInLine+1) * sizeof(char*));
		 	 if(listOfTokens==NULL)//failed reallocating
			 	 {
			 		 printErr("Allocation failed for creating list of tokens from line\n");
			 	 }
		 }
		 token = strtok(NULL, TOK_DELIM);
	 }
	 return listOfTokens;
}

void printErr(const char* msg){
	printf("%s",msg);
	exit(EXIT_FAILURE);
}

presources getResources(const char* filename,int* sizeOfDataBase)
{
	FILE* resourceFile=fopen(filename,"r");//open file for reading
	char** splitedLine;
	int indexAtArray=0,numToAlloc=1;
	if(resourceFile==NULL)//failed
	{
		printErr("Problem reading the file resources.txt!\n");
	}
	char *lineRead = NULL;
	size_t len = 0;
	ssize_t read;
	presources arrayOfResources;
	arrayOfResources=(presources)malloc(sizeof(resources));
	if(arrayOfResources==NULL){
		printErr("Allocation failed for creating a new arrayOfResources \n");
	}
	while ((read = getline(&lineRead, &len, resourceFile)) != -1){//on success getline returns how many characters have been read
		arrayOfResources = realloc(arrayOfResources, (numToAlloc) * sizeof(resources));
		splitedLine=splitLine(lineRead);//returns the splited line
		arrayOfResources[indexAtArray].resourceType=atoi(splitedLine[0]);
		arrayOfResources[indexAtArray].resourceName=(char*)malloc(sizeof(char)*(strlen(splitedLine[1])+1));
			if(arrayOfResources[indexAtArray].resourceName == NULL){
				printErr("Allocation failed for creating a new name in resource data base\n");
			}
			strcpy(arrayOfResources[indexAtArray].resourceName,splitedLine[1]);
			arrayOfResources[indexAtArray].resourceAmount=atoi(splitedLine[2]);
			sem_init(&(arrayOfResources[indexAtArray].numOfResources),0,arrayOfResources[indexAtArray].resourceAmount);
			indexAtArray++;
			(*sizeOfDataBase)++;
			numToAlloc++;
		}
	fclose(resourceFile);
	return arrayOfResources;
}

pservices  getServices(const char* filename,int* sizeOfDataBase){
	FILE* servicesFile=fopen(filename,"r");//open file for reading
		char** splitedLine;
		int indexAtArray=0,numToAlloc=1,i,startringIndexForArrayResources=4,temp,k,l;
		if(servicesFile==NULL)//failed
		{
			printErr("Problem reading the file services.txt!\n");
		}
		char *lineRead = NULL;
		size_t len = 0;
		ssize_t read;
		pservices arrayOfServices;
		arrayOfServices=(pservices)malloc(sizeof(services));
		if(arrayOfServices==NULL){
			printErr("Allocation failed for creating a new arrayOfServices \n");
		}

		while ((read = getline(&lineRead, &len, servicesFile)) != -1){//on success getline returns how many characters have been read
			arrayOfServices = realloc(arrayOfServices, (numToAlloc) * sizeof(services));
			if(arrayOfServices==NULL){
				printErr("Allocation failed for creating a new arrayOfServices \n");
			}
			splitedLine=splitLine(lineRead);//returns the splited line
			arrayOfServices[indexAtArray].serviceType=atoi(splitedLine[0]);
			arrayOfServices[indexAtArray].serviceName=(char*)malloc(sizeof(char)*(strlen(splitedLine[1])+1));
				if(arrayOfServices[indexAtArray].serviceName == NULL){
					printErr("Allocation failed for creating a new name in services data base\n");
				}
				strcpy(arrayOfServices[indexAtArray].serviceName,splitedLine[1]);
				arrayOfServices[indexAtArray].hoursPerService=atoi(splitedLine[2]);
				arrayOfServices[indexAtArray].numOfResourcesNeeded=atoi(splitedLine[3]);
				if(arrayOfServices[indexAtArray].numOfResourcesNeeded!=0){
					arrayOfServices[indexAtArray].arrayOfResourcesNeeded=(Type*)malloc(sizeof(Type)*arrayOfServices[indexAtArray].numOfResourcesNeeded);
					if(arrayOfServices[indexAtArray].arrayOfResourcesNeeded==NULL){
						printErr("Allocation failed for creating a new array for resources in services data base\n");
					}
					for(i=0;i<arrayOfServices[indexAtArray].numOfResourcesNeeded;i++){
						arrayOfServices[indexAtArray].arrayOfResourcesNeeded[i]=atoi(splitedLine[startringIndexForArrayResources]);
						startringIndexForArrayResources++;
					}
					//sort the arrayOfResourcesNeeded to avoid dead lock
					 for (k = 0; k <arrayOfServices[indexAtArray].numOfResourcesNeeded ; k++) {     
					        for (l = k+1; l < arrayOfServices[indexAtArray].numOfResourcesNeeded; l++) {     
					           if(arrayOfServices[indexAtArray].arrayOfResourcesNeeded[k] > arrayOfServices[indexAtArray].arrayOfResourcesNeeded[l]) {    
					               temp = arrayOfServices[indexAtArray].arrayOfResourcesNeeded[k];    
					               arrayOfServices[indexAtArray].arrayOfResourcesNeeded[k] = arrayOfServices[indexAtArray].arrayOfResourcesNeeded[l];    
					               arrayOfServices[indexAtArray].arrayOfResourcesNeeded[l] = temp;    
					           }     
					        }     
					    } 
				}
				(*sizeOfDataBase)++;
				indexAtArray++;
				numToAlloc++;
				startringIndexForArrayResources=4;
			}
		fclose(servicesFile);
		return arrayOfServices;
}

prequest  getRequests(const char* filename,int* sizeOfDataBase){
	FILE* requestFile=fopen(filename,"r");//open file for reading
			char** splitedLine;
			int indexAtArray=0,numToAlloc=1,i,startringIndexForArrayrequest=3;
			if(requestFile==NULL)//failed
			{
				printErr("Problem reading the file services.txt!\n");
			}
			char *lineRead = NULL;
			size_t len = 0;
			ssize_t read;
			prequest arrayOfrequest;
			arrayOfrequest=(prequest)malloc(sizeof(request));
			if(arrayOfrequest==NULL){
				printErr("Allocation failed for creating a new arrayOfServices \n");
			}
			while ((read = getline(&lineRead, &len, requestFile)) != -1){//on success getline returns how many characters have been read
				arrayOfrequest = realloc(arrayOfrequest, (numToAlloc) * sizeof(request));
				splitedLine=splitLine(lineRead);//returns the splited line
				arrayOfrequest[indexAtArray].carNumber=atoi(splitedLine[0]);
				arrayOfrequest[indexAtArray].timeArrivalHour=atoi(splitedLine[1]);
				arrayOfrequest[indexAtArray].numOfServicesNeeded=atoi(splitedLine[2]);
					if(arrayOfrequest[indexAtArray].numOfServicesNeeded!=0){
						arrayOfrequest[indexAtArray].arrayOfServicesNeeded=(Type*)malloc(sizeof(Type)*arrayOfrequest[indexAtArray].numOfServicesNeeded);
						if(arrayOfrequest[indexAtArray].arrayOfServicesNeeded==NULL){
							printErr("Allocation failed for creating a new array for services in request data base\n");
						}
						for(i=0;i<arrayOfrequest[indexAtArray].numOfServicesNeeded;i++){
							arrayOfrequest[indexAtArray].arrayOfServicesNeeded[i]=atoi(splitedLine[startringIndexForArrayrequest]);
							startringIndexForArrayrequest++;
						}
					}
					(*sizeOfDataBase)++;
					indexAtArray++;
					numToAlloc++;
					startringIndexForArrayrequest=3;
				}
			fclose(requestFile);
			return arrayOfrequest;
}

void initTime(timeInTheDay* timer){
	timer->time=0;
}
void* manageClock(){
	while(1){
		sleep(1);//counts 1 sec and than inc timer
		timer.time=(timer.time+1)%24;
	}
	return NULL;
}

void* manageCarRequest(void* index){
	int i,j,k,IndexOfServiceInDataBaseServices=0;
	int currentCarIndex=*(int*)index;//get the index
	prequest currentCar=&requestDataBase[currentCarIndex];
	while(1){//runs until its my time to do be in the garage
		if(requestDataBase[currentCarIndex].timeArrivalHour==timer.time){
		printf("car: %d time: %d arrived to the garage\n",requestDataBase[currentCarIndex].carNumber,requestDataBase[currentCarIndex].timeArrivalHour);
		printf("car: %d time: %d started request in the garage\n",requestDataBase[currentCarIndex].carNumber,timer.time);
		for(i=0;i<currentCar->numOfServicesNeeded;i++){//run all over the array of current car request
			//search in the services the needed service to find out which resources needed and available
			for(j=0;j<sizeServicesDataBase;j++){
				if(currentCar->arrayOfServicesNeeded[i]==servicesDataBase[j].serviceType){//found the relevant service
					IndexOfServiceInDataBaseServices=j;//found the needed index of service in the service data base and save it
				}
			}
			for(k=0;k<sizeResourceDataBase;k++){//look inside the resources data base
				for(j=0;j<servicesDataBase[IndexOfServiceInDataBaseServices].numOfResourcesNeeded;j++){//run all over the resource array
					if(resourceDataBase[k].resourceType==servicesDataBase[IndexOfServiceInDataBaseServices].arrayOfResourcesNeeded[j]){
						sem_wait(&resourceDataBase[k].numOfResources);
					}
				}
			}
			printf("car: %d time: %d started %s\n",requestDataBase[currentCarIndex].carNumber,timer.time,servicesDataBase[IndexOfServiceInDataBaseServices].serviceName);
			//go to sleep for k sec
			sleep(servicesDataBase[IndexOfServiceInDataBaseServices].hoursPerService);
			for(k=0;k<sizeResourceDataBase;k++){
				for(j=0;j<servicesDataBase[IndexOfServiceInDataBaseServices].numOfResourcesNeeded;j++){
					if(resourceDataBase[k].resourceType==servicesDataBase[IndexOfServiceInDataBaseServices].arrayOfResourcesNeeded[j]){
						sem_post(&resourceDataBase[k].numOfResources);//update resource is free for others to be used
					}
				}
			}
			printf("car: %d time: %d completed %s!\n",requestDataBase[currentCarIndex].carNumber,timer.time,servicesDataBase[IndexOfServiceInDataBaseServices].serviceName);
		}
		printf("car: %d time: %d service complete.\n",requestDataBase[currentCarIndex].carNumber,timer.time);
		break;
		}
	}
		return NULL;
}

