#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <limits.h>

#define CAPS 4

int m;
int *authStatus;


typedef struct check{
    int pick;
    int drop;
}check;

typedef struct allRequests{
    int id;
    int start;
    int end;
    int elevator;
    int picked;
    int completed;
}allRequests;

typedef struct Node{
    allRequests data;
    struct Node *next;
}Node;

typedef struct Node2{
    int data;
    struct Node2 *next;
}Node2;

Node **masterMap;
Node2 **pickMap;
Node2 **dropMap;
int masterMapSize, pickMapSize, dropMapSize;

Node* createMasterNode(allRequests value){
    Node* newNode=(Node*)malloc(sizeof(Node));
    newNode->data=value;
    newNode->next=NULL;
    return newNode;
}

Node2* createNode(int value){
    Node2* newNode=(Node2*)malloc(sizeof(Node2));
    newNode->data=value;
    newNode->next=NULL;
    return newNode;
}

void initializeMasterMap(int size){
    masterMapSize=size;
    masterMap=(Node **)malloc(masterMapSize*sizeof(Node *));
    for(int i=0; i<masterMapSize; i++){
        masterMap[i]=NULL;
    }
}

void initializePickMap(int size){
    pickMapSize=size;
    pickMap=(Node2 **)malloc(pickMapSize*sizeof(Node2 *));
    for(int i=0; i<pickMapSize; i++){
        pickMap[i]=NULL;
    }
}

void initializeDropMap(int size){
    dropMapSize=size;
    dropMap=(Node2 **)malloc(dropMapSize*sizeof(Node2 *));
    for(int i=0; i<dropMapSize; i++){
        dropMap[i]=NULL;
    }
}

void insertMasterMap(allRequests value){
    int key=value.id%masterMapSize;
    Node* newNode=createMasterNode(value);
    newNode->next=masterMap[key];
    masterMap[key]=newNode;
}

void insertPickMap(int key, int value){
    Node2* newNode=createNode(value);
    newNode->next=pickMap[key];
    pickMap[key]=newNode;
}

void insertDropMap(int key, int value){
    Node2* newNode=createNode(value);
    newNode->next=dropMap[key];
    dropMap[key]=newNode;
}

void deleteMasterMap(int id) {
    int key=id%masterMapSize;
    Node* current=masterMap[key];
    Node* previous=NULL;

    while(current!=NULL && current->data.id!=id){
        previous=current;
        current=current->next;
    }

    if(current==NULL){
        return;
    }

    if(previous==NULL){
        masterMap[key]=current->next;
    }else{
        previous->next=current->next;
    }

    free(current);
}

void deletePickMap(int key, int value){
    Node2* current=pickMap[key];
    Node2* previous=NULL;

    while(current!=NULL && current->data!=value){
        previous=current;
        current=current->next;
    }

    if(current==NULL){
        return;
    }

    if(previous==NULL){
        pickMap[key]=current->next;
    }else{
        previous->next=current->next;
    }

    free(current);
}

void deleteDropMap(int key, int value){
    Node2* current=dropMap[key];
    Node2* previous=NULL;

    while(current!=NULL && current->data!=value){
        previous=current;
        current=current->next;
    }

    if(current==NULL){
        return;
    }

    if(previous==NULL){
        dropMap[key]=current->next;
    }else{
        previous->next=current->next;
    }

    free(current);
}


typedef struct PassengerRequest{
    int requestId;
    int startFloor;
    int requestedFloor;
}PassengerRequest;

typedef struct MainSharedMemory{
    char authStrings[100][21];
    char elevatorMovementInstructions[100];
    PassengerRequest newPassengerRequests[30];
    int elevatorFloors[100];
    int droppedPassengers[1000];
    int pickedUpPassengers[1000][2];
}MainSharedMemory;

typedef struct SolverRequest{
    long mtype;
    int elevatorNumber;
    char authStringGuess[21];
}SolverRequest;

typedef struct SolverResponse{
    long mtype;
    int guessIsCorrect;
}SolverResponse;

typedef struct TurnChangeResponse{
    long mtype;
    int turnNumber;
    int newPassengerRequestCount;
    int errorOccured;
    int finished;
}TurnChangeResponse;

typedef struct TurnChangeRequest{
    long mtype;
    int droppedPassengersCount;
    int pickedUpPassengersCount;
}TurnChangeRequest;


void dropPassengers(int key, int elevator, MainSharedMemory *mainShmPtr, int *passengerCount, TurnChangeRequest *turnRequest, check **stops){
    Node2 *current=dropMap[key];

    while(current!=NULL){
        int id=current->data;
        int reset=0;

        int masterKey=id%masterMapSize;
        Node *masterCurrent=masterMap[masterKey];    

        while(masterCurrent!=NULL){
            if(masterCurrent->data.id==id && masterCurrent->data.elevator==elevator){

                mainShmPtr->droppedPassengers[turnRequest->droppedPassengersCount]=id;

                turnRequest->droppedPassengersCount++;
                passengerCount[elevator]--;
                stops[elevator][key].drop--;

                deleteDropMap(masterCurrent->data.end, id);
                deleteMasterMap(id);

                current=dropMap[key];
                reset=1;
                break;
            }
            masterCurrent=masterCurrent->next;
        }

        if(reset==0){
            current=current->next;
        }
    }
}



void pickPassengers(int key, int elevator, MainSharedMemory *mainShmPtr, int *passengerCount, TurnChangeRequest *turnRequest, check **stops){
    Node2 *current=pickMap[key];

    while(current!=NULL){
        if(passengerCount[elevator]==CAPS){
            return;
        }

        int id=current->data;
        int reset=0;

        int masterKey=id%masterMapSize;
        Node *masterCurrent=masterMap[masterKey];

        while(masterCurrent!=NULL){
            if(masterCurrent->data.id==id && masterCurrent->data.elevator==elevator){

                mainShmPtr->pickedUpPassengers[turnRequest->pickedUpPassengersCount][0]=id;
                mainShmPtr->pickedUpPassengers[turnRequest->pickedUpPassengersCount][1]=elevator;

                turnRequest->pickedUpPassengersCount++;
                passengerCount[elevator]++;
                stops[elevator][masterCurrent->data.end].drop++;
                stops[elevator][key].pick--;
                
                insertDropMap(masterCurrent->data.end, id);
                deletePickMap(masterCurrent->data.start, id);
                
                current=pickMap[key];
                reset=1;
                break;
            }
            masterCurrent=masterCurrent->next;
        }

        if(reset==0){
            current=current->next;
        }
    }
}



void genauthstring(char *authstring, int length, int factor){
    for(int i=0; i<length; i++){
        authstring[i]='a'+(factor%6);
        factor/=6;
    }
    authstring[length]='\0';
}



void authorization(int index, int length, int solverKey, MainSharedMemory *mainShmPtr){
    
    int solverNumber=index%m;
    
    SolverRequest solRequest;
    SolverResponse solResponse;

    char stringGuess[25];
    int factor=0;

    solRequest.mtype=2;
    solRequest.elevatorNumber=index;
    msgsnd(solverKey, &solRequest, sizeof(solRequest)-sizeof(solRequest.mtype), 0);

    while(1){
        genauthstring(stringGuess, length, factor);

        solRequest.mtype=3;
        strcpy(solRequest.authStringGuess, stringGuess);
        msgsnd(solverKey, &solRequest, sizeof(solRequest)-sizeof(solRequest.mtype), 0);
        msgrcv(solverKey, &solResponse, sizeof(solRequest)-sizeof(solRequest.mtype), 4, 0);
        
        if(solResponse.guessIsCorrect!=0){
            strcpy(mainShmPtr->authStrings[index], stringGuess);
            break;
        }

        factor++;
    }
}



typedef struct ThreadData{
    int elevator;
    int *passengerCount;
    int *passengerCountLag;
    char *direction;
    check **stops;
    MainSharedMemory *mainShmPtr;
    TurnChangeRequest *turnRequest;
    int solverKey;
    int k;
}ThreadData;



void elevatorTask(ThreadData *data){
    int elevator=data->elevator;
    int *passengerCount=data->passengerCount;
    int *passengerCountLag=data->passengerCountLag;
    char *direction=data->direction;
    check **stops=data->stops;
    MainSharedMemory *mainShmPtr=data->mainShmPtr;
    TurnChangeRequest *turnRequest=data->turnRequest;
    int solverKey=data->solverKey;
    int k=data->k;

    dropPassengers(mainShmPtr->elevatorFloors[elevator], elevator, mainShmPtr, passengerCount, turnRequest, stops);
    pickPassengers(mainShmPtr->elevatorFloors[elevator], elevator, mainShmPtr, passengerCount, turnRequest, stops);

    int upRequired=0;
    int downRequired=0;
    int upDist=INT_MAX;
    int downDist=INT_MAX;
    int currentFloor=mainShmPtr->elevatorFloors[elevator];


    if(passengerCount[elevator]<CAPS){
        for(int j=currentFloor+1; j<k; j++){
            if(stops[elevator][j].pick>0 || stops[elevator][j].drop>0){
                upRequired=1;
                upDist=j-currentFloor;
                break;
            }
        }

        for(int j=currentFloor-1; j>=0; j--){
            if(stops[elevator][j].pick>0 || stops[elevator][j].drop>0){
                downRequired=1;
                downDist=currentFloor-j;
                break;
            }
        }
    }
    else{
        for(int j=currentFloor+1; j<k; j++){
            if(stops[elevator][j].drop>0){
                upRequired=1;
                upDist=j-currentFloor;
                break;
            }
        }

        for(int j=currentFloor-1; j>=0; j--){
            if(stops[elevator][j].drop>0){
                downRequired=1;
                downDist=currentFloor-j;
                break;
            }
        }
    }


    if(upRequired==0 && downRequired==0){
        direction[elevator]='s';
    }
    else if(upRequired==1 && downRequired==0){
        direction[elevator]='u';
    }
    else if(upRequired==0 && downRequired==1){
        direction[elevator]='d';
    }

    mainShmPtr->elevatorMovementInstructions[elevator]=direction[elevator];

    int length=passengerCountLag[elevator];

    if(mainShmPtr->elevatorMovementInstructions[elevator]!='s' && length>0){
        authorization(elevator, length, solverKey, mainShmPtr);
        authStatus[elevator]=1;
    }
    else{
        authStatus[elevator]=1;
    }
}


int main(){

    FILE *input_pointer=fopen("input.txt", "r");
    int n, k, t, key_shm, key_msg;
    fscanf(input_pointer, "%d %d %d %d %d %d", &n, &k, &m, &t, &key_shm, &key_msg);

    int solverkeys[m];
    for(int i=0; i<m; i++){
        fscanf(input_pointer, "%d", &solverkeys[i]);
    }
    fclose(input_pointer);

    MainSharedMemory *mainShmPtr;
    int shm_id=shmget((key_t)key_shm, sizeof(MainSharedMemory), 0666);
    mainShmPtr=shmat(shm_id, NULL, 0);

    int msg_main_id=msgget((key_t)key_msg, 0666);

    int msg_solver_id[m];
    for(int i=0; i<m; i++){
        msg_solver_id[i]=msgget((key_t)solverkeys[i], 0666);
    }

    
    initializeMasterMap(t);
    initializePickMap(k);
    initializeDropMap(k);

    int passengerCount[n];
    int passengerCountLag[n];
    char direction[n];
    check **stops=(check **)calloc(n,sizeof(check *));

    for(int i=0; i<n; i++){
        passengerCount[i]=0;
        passengerCountLag[i]=0;
        direction[i]='s';
        stops[i]=(check *)calloc(k,sizeof(check));
    }

    TurnChangeResponse turnResponse;
    TurnChangeRequest turnRequest;
    
    authStatus=(int *)malloc(n*sizeof(int));
    

    while(1){
        msgrcv(msg_main_id, &turnResponse, sizeof(turnResponse)-sizeof(turnResponse.mtype), 2, 0);
        
        if(turnResponse.errorOccured==1 || turnResponse.finished==1){
            break;
        }
 
        for(int i=0; i<n; i++){
            passengerCountLag[i]=passengerCount[i];
            authStatus[i]=0;
        }


        for(int i=0; turnResponse.turnNumber<=t && i<turnResponse.newPassengerRequestCount; i++){
            PassengerRequest request=mainShmPtr->newPassengerRequests[i];

            int elev=i%n;
            allRequests req={request.requestId, request.startFloor, request.requestedFloor, elev, 0, 0};

            insertMasterMap(req);

            insertPickMap(request.startFloor, request.requestId);
            stops[elev][request.startFloor].pick++;
        }


        turnRequest.mtype=1;
        turnRequest.droppedPassengersCount=0;
        turnRequest.pickedUpPassengersCount=0;

        for(int i=0; i<n; i++){

            ThreadData data;
            data.elevator=i;
            data.passengerCount=passengerCount;
            data.passengerCountLag=passengerCountLag;
            data.direction=direction;
            data.stops=stops;
            data.mainShmPtr=mainShmPtr;
            data.turnRequest=&turnRequest;
            data.solverKey=msg_solver_id[i%m];
            data.k=k;

            elevatorTask(&data);
        }

        msgsnd(msg_main_id, &turnRequest, sizeof(turnRequest)-sizeof(turnRequest.mtype), 0);
    }

    for(int i=0; i<n; i++){
        free(stops[i]);
    }
    free(stops);
    shmdt(mainShmPtr);
    
    return 0;
}