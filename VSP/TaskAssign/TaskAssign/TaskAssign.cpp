#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#define MAXNODENUM 5000
#define MAXENGINENUM 5
#define MAXTASKNUM 15
#define TRUE 1
#define FALSE 0
FILE *engineStream;
FILE *taskStream;
FILE *outputStream;
//typedef struct timeval{
 // long tv_sec;
 // long tv_usec;
//} timeval;
typedef struct engine{
	char engineName[5];
	int speed;
	int mem_size;
	int engineNo;
}engine;
typedef struct task{
	char taskName;
	int reqInstruction;
	int reqMem;
	int outputSize;
	char *preReqTask;
	int taskNo;
}task;
typedef struct mapNode{
	int taskOrder[MAXTASKNUM];
	int nodeNo;
	int childNo[MAXENGINENUM];
	float nodeETCost[MAXENGINENUM];
	float nodeET;
	int OCost;
	float HETCost;
	int HOCost;
	int assignOrder[MAXTASKNUM];
	int goal;
	int explored;
	int parentNo;
}mapNode;
//typedef struct mapNodeCost{
//	int HOCost;
//	float HETCost;
//	int OCost;
//	float nodeETCost[MAXENGINENUM];
//	float nodeET;
//	int nodeNo;
//}mapNodeCost;
typedef struct availTask{
	int availTaskNo;
	int availEngineNo[MAXENGINENUM];
}availTask;
//int adjMatrix[MAXNODENUM][MAXNODENUM];
//task *myTaskP;
//engine *myEngineP;
//map *mymapP;
//int dependNum[MAXTASKNUM];
int engineNum = 0,taskNum = 0;

int UCSBYET(const engine *, const task *,mapNode *);
int UCSBYO(const engine *, const task *,mapNode *);
int GSBYET(const engine *, const task *,mapNode *);
int GSBYO(const engine *, const task *,mapNode *);
int ASBYO(const engine *, const task *,mapNode *);
int ASBYET(const engine *, const task *,mapNode *);

void NodeSearch(const engine *, const task *,mapNode *,int ,int *);
void CalculateCost(const engine *, const task *,mapNode *,int);
void HFunction(const engine *, const task *,mapNode *,int);
void PrintResult(const engine *, const task *,mapNode *,int,int,int);
void CleanUp(const engine *, const task *,mapNode *,int,int);
int InitialChecking(const engine *, const task *);

int main(int argc,char* argv[] )
{
char *list1 ,*list1Head,*list2,*list2Head; 
char tempC; 
int length1 = 0,length2 = 0;//input file length
char *sourceTask,*sourceTaskHead;
char destTask;
int comment = 0; //comment part if comment=1,number of its required tasks for each task
int enginePart=0,taskPart=0,dependPart = 0,sourcePart=0; //engine desc part if enginePart =1 ,count the total number for the engine
int taskNameIndex = 0,taskDataIndex = 0,engineNameIndex = 0,engineDataIndex=0; //index for putting data into engine struct
clock_t clock1,clock2;
float runtimeUCSO,runtimeUCSET,runtimeGSO,runtimeGSET,runtimeASO,runtimeASET;
int memCostUCSO,memCostUCSET,memCostGSO,memCostGSET,memCostASO,memCostASET;
int pass;
//full path,command.exe,input#1,input#2,input#3
//for UNIX
/*
if (argc != 4){
	printf("\nPlease input valid command: engineFile taskFile outputFile\n");
	return -1;
}

char *engineFile,*taskFile,*outputFile;
engineFile = argv[1];
taskFile = argv[2];
outputFile = argv[3];
*/
 
 //FOR windows

if (argc != 5){
	printf("Please input valid command: engineFile taskFile outputFile");
	return -1;
}
char *engineFile,*taskFile,*outputFile;
engineFile = argv[2];
taskFile = argv[3];
outputFile = argv[4];


sourceTask = (char*)malloc(sizeof(char)*(MAXTASKNUM));
sourceTaskHead = sourceTask;
for (int i=0;i<MAXTASKNUM;i++){
	*(sourceTask+i) = '\0';
}

engine myEngine[MAXENGINENUM];
task myTask[MAXTASKNUM];
mapNode myMapNode[MAXNODENUM];

for (int i =0; i<MAXENGINENUM;i++){
	strcpy(myEngine[i].engineName,"\0");
	myEngine[i].speed = 0;
	myEngine[i].mem_size = 0;
	myEngine[i].engineNo = i;
}
for (int i=0;i<MAXTASKNUM;i++){
	myTask[i].taskName=NULL;
	myTask[i].reqInstruction =0;
	myTask[i].reqMem = 0;
	myTask[i].outputSize = 0;
	myTask[i].taskNo = i;
	myTask[i].preReqTask = (char*)malloc(sizeof(char)*(MAXTASKNUM));
	for (int j=0;j<MAXTASKNUM;j++)
		*(myTask[i].preReqTask+j) = '\0';
}


//Read engine file-------------------------------------------
if( (engineStream= fopen( engineFile, "r" )) == NULL )
{
	printf( "trying open'%s'\n",engineFile);
	printf( "open '%s' failure\n",engineFile );
}
else{
	//open file to go through the file get the length
	printf( "'%s' successfully opened\n",engineFile );
	do {
		tempC = fgetc(engineStream);
		length1 ++;
	}while(tempC != -1);
	rewind(engineStream);

	//initialize the string pointer
	list1 = (char*)malloc(length1*sizeof(char));
	list1Head = list1;
	for (int i =0;i<length1;i++){
		*list1 = '\0';
		list1++;
	}
	list1 = list1Head;

	//Remove comment
	do {
		tempC = fgetc(engineStream);
		if (tempC == '#')
			comment = 1;
		if (tempC == '\n')
			comment = 0;
		if(comment == 0 && tempC != EOF){
			*list1=tempC;
			list1 ++;
		}
	}while(tempC != -1);
	list1 = list1Head;
	//printf("%s\nFile length1 = %d\n",list1,strlen(list1Head));

	for (int i = 0; i< strlen(list1Head);i++){
		if (*list1 == ']' && *(list1-1) == 'e' && *(list1-2) == 'n')
			enginePart =1;
		if (*list1 == '[' && *(list1+1) == 'b' && *(list1+2) == 'a')
			enginePart =0;
		
		if(enginePart == 1 && *list1 == 'e' && *(list1+1) == '-') {
			sscanf(list1,"%s",myEngine[engineNameIndex].engineName);
			//printf("%s\n",myEngine[nameIndex].engineName);
			engineNameIndex ++;
			engineNum++;
		}
		if (*list1 == '[' && (*(list1+1) >=48 && *(list1+1) <= 57) && enginePart == 1 ){
			sscanf(list1,"[%d,%d]",&myEngine[engineDataIndex].speed,&myEngine[engineDataIndex].mem_size);
			engineDataIndex ++;
			//printf("index = %d\n",engineDataIndex);
		}
		list1++;//in for loop increase pointer
	}
	list1 =list1Head;

}

//Read task file------------------------------------------------
if( (taskStream= fopen(taskFile, "r" )) == NULL )
{
	printf( "trying open'%s'\n",taskFile);
	printf( "open '%s' failure\n",taskFile );
}
else{
	//打开成功
	//open file to go through the file get the length1
	printf( "'%s'successfully opened\n",taskFile );
	do {
		tempC = fgetc(taskStream);
		length2 ++;
	}while(tempC != -1);
	rewind(taskStream);

	//initialize the string pointer
	list2 = (char*)malloc(length2*sizeof(char));
	list2Head = list2;
	for (int i =0;i<length2;i++){
		*list2 = '\0';
		list2++;
	}
	//printf("%s",list2);
	list2 = list2Head;

	//Remove comment
	do {
		tempC = fgetc(taskStream);
		if (tempC == '#')
			comment = 1;
		if (tempC == '\n')
			comment = 0;
		if(comment == 0 && tempC != EOF){
			*list2=tempC;
			list2 ++;
		}
	}while(tempC != -1);
	list2 = list2Head;
	//printf("%s\nFile length2 = %d\n",list2,strlen(list2Head));

	for (int i = 0; i< strlen(list2Head);i++){
		if (*list2 == ']' && *(list2-1) == 'k' && *(list2-2) == 's')
			taskPart =1;
		if (*list2 == '[' && *(list2+1) == 'd' && *(list2+2) == 'e')
			taskPart =0;
		
		if(taskPart == 1 && (*list2 >= 65 && *list2 <= 90)) {
			myTask[taskNameIndex].taskName = *list2;
			taskNameIndex ++;
			taskNum++;
		}
		if (*list2 == '[' && (*(list2+1) >=48 && *(list2+1) <= 57) && taskPart == 1 ){
			sscanf(list2,"[%d,%d,%d]",&myTask[taskDataIndex].reqInstruction,&myTask[taskDataIndex].reqMem,&myTask[taskDataIndex].outputSize);
			taskDataIndex ++;
			//printf("index = %d\n",dataIndex);
		}
		if (*list2 == ']' && *(list2-1) == 'y'&& *(list2-2)=='c')
			dependPart = 1;
		if (*list2 == EOF)
			dependPart = 0;
		if (dependPart == 1 && *list2 == '(' && *(list2-1) =='(' )
			sourcePart = 1;
		//assume a dest after source
		if (sourcePart == 1 && (*list2 >= 65 && *list2 <= 90 )){
			*sourceTask = *list2 ;
			sourceTask++;
		}
		if (dependPart == 1 && *list2 == ')' && *(list2+1) ==',' ){
			sourcePart = 0;
			destTask = *(list2 + 2);
			sourceTask = sourceTaskHead;
			for(int i=0;i<taskNum;i++){
				if(myTask[i].taskName == destTask){
					strcpy(myTask[i].preReqTask,sourceTask);
					//dependNum[i] = strlen(sourceTask);
				}
			}
			//sourceTask = sourceTaskHead;
			while(*sourceTask != '\0'){
				*sourceTask='\0';
				sourceTask++;
			}
			sourceTask = sourceTaskHead;
		}
		list2++;//in for loop increase pointer
	}
	list2 =list2Head;
}

	/*
for(int i=0;i<engineNum;i++){
	printf("engine name is %s\n",engineInfoP[i].engineName);
	printf("engine speed is %d\n",engineInfoP[i].speed);
	printf("engine memory size is %d\n",engineInfoP[i].mem_size);
}
for(int i=0;i<taskNum;i++){
	printf("task name is %c\n",(taskInfoP+i)->taskName);
	printf("task required I is %d\n",(taskInfoP+i)->reqInstruction);
	printf("task mem Size is %d\n",taskInfoP[i].reqMem);
	printf("task Output is %d\n",taskInfoP[i].outputSize);
	//printf("dependNum %d = %d \n",i,dependNum[i]);
	printf("linking to %s\n",taskInfoP[i].preReqTask);
}
*/

/*
	pass = InitialChecking()
	//check depend
	//check memory
	//check MAX NUM
	*/
	pass = InitialChecking(&myEngine[0],&myTask[0]);
	if(pass != TRUE){
		return -1;
	}

//--------------------------START SEARCHING---------------------
//gettimeofday(&time1, NULL);
clock1 = clock();
memCostUCSO = UCSBYO(&myEngine[0],&myTask[0],&myMapNode[0]);
clock2 = clock();
runtimeUCSO=(float)(clock2 - clock1)*(1/CLOCKS_PER_SEC) ;


clock1 = clock();
memCostUCSET = UCSBYET(&myEngine[0],&myTask[0],&myMapNode[0]);
clock2 = clock();
runtimeUCSET=(float)(clock2 - clock1)*(1/CLOCKS_PER_SEC) ;


clock1 = clock();
memCostGSO = GSBYO(&myEngine[0],&myTask[0],&myMapNode[0]);
clock2 = clock();
runtimeGSO=(float)(clock2 - clock1)*(1/CLOCKS_PER_SEC) ;

clock1 = clock();
memCostGSET = GSBYET(&myEngine[0],&myTask[0],&myMapNode[0]);
clock2 = clock();
runtimeGSET=(float)(clock2 - clock1)*(1/CLOCKS_PER_SEC) ;

clock1 = clock();
memCostASO = ASBYO(&myEngine[0],&myTask[0],&myMapNode[0]);
clock2 = clock();
runtimeASO=(float)(clock2 - clock1)*(1/CLOCKS_PER_SEC) ;

clock1 = clock();
memCostASET = ASBYET(&myEngine[0],&myTask[0],&myMapNode[0]);
clock2 = clock();
runtimeASET=(float)(clock2 - clock1)*(1/CLOCKS_PER_SEC) ;

//=========================SEARCHING END========================


printf("===================UCS-O========================\n");
printf("Running time for UCS-O is %f\n",runtimeUCSO);
printf("Memory requirement for UCS-O is %d\n",memCostUCSO);

printf("===================UCS-ET========================\n");
printf("Running time for UCS-ET is %f\n",runtimeUCSET);
printf("Memory requirement for UCS-ET is %d\n",memCostUCSET);

printf("===================GS-O========================\n");
printf("Running time for GS-O is %f\n",runtimeGSO);
printf("Memory requirement for GS-O is %d\n",memCostGSO);

printf("===================GS-ET========================\n");
printf("Running time for GS-ET is %f\n",runtimeGSET);
printf("Memory requirement for GS-ET is %d\n",memCostGSET);

printf("===================AS-O========================\n");
printf("Running time for AS-O is %f\n",runtimeASO);
printf("Memory requirement for AS-O is %d\n",memCostASO);

printf("===================AS-ET========================\n");
printf("Running time for AS-ET is %f\n",runtimeASET);
printf("Memory requirement for AS-ET is %d\n",memCostASET);


if( (outputStream = fopen( outputFile, "w+" )) == NULL ){
	printf( "'%s' writing failure\n",outputFile );
}
else
{
	printf( "writing to '%s' sucessfully\n",outputFile );
	
	fprintf(outputStream,"===================UCS-O========================\n");
	fprintf(outputStream,"Running time for UCS-O is %f\n",runtimeUCSO);
	fprintf(outputStream,"Memory requirement for UCS-O is %d\n",memCostUCSO);

	fprintf(outputStream,"===================UCS-ET========================\n");
	fprintf(outputStream,"Running time for UCS-ET is %f\n",runtimeUCSET);
	fprintf(outputStream,"Memory requirement for UCS-ET is %d\n",memCostUCSET);

	fprintf(outputStream,"===================GS-O========================\n");
	fprintf(outputStream,"Running time for GS-O is %f\n",runtimeGSO);
	fprintf(outputStream,"Memory requirement for GS-O is %d\n",memCostGSO);

	fprintf(outputStream,"===================GS-ET========================\n");
	fprintf(outputStream,"Running time for GS-ET is %f\n",runtimeGSET);
	fprintf(outputStream,"Memory requirement for GS-ET is %d\n",memCostGSET);

	fprintf(outputStream,"===================AS-O========================\n");
	fprintf(outputStream,"Running time for AS-O is %f\n",runtimeASO);
	fprintf(outputStream,"Memory requirement for AS-O is %d\n",memCostASO);

	fprintf(outputStream,"===================AS-ET========================\n");
	fprintf(outputStream,"Running time for AS-ET is %f\n",runtimeASET);
	fprintf(outputStream,"Memory requirement for AS-ET is %d\n",memCostASET);

}

free(list2);
free(list1);
/*
if (engineStream!=NULL){
	printf( " engineStream closed\n" );
	fclose(outputStream);
}
else
	printf("engineStream exist\n");
if (taskStream!=NULL){
	printf( " taskStream closed\n" );
	fclose(outputStream);
}
else
	printf("taskStream exist\n");
if (outputStream!=NULL){
	printf( " outputStream closed\n" );
	fclose(outputStream);
}
else
	printf("outputStream not exist\n");
	*/
//_getch();
}



//--------------------------------------------------------------
//-----------------------UCSBYO---------------------------------
//--------------------------------------------------------------


int UCSBYO(const engine *engineInfoP, const task *taskInfoP,mapNode *myMapNodeP)
{
	//char *dependSearchP;
	int lowestEdge=10000;
	int lowestNodeNo=-1;
	int nextEmptyIndex=0;
	int finish=FALSE,goalNodeNo = -1;
	int largestBranchNode=-1;
	//int exploredSet[MAXNODENUM];
	
	//initialization structure-------------------------
	for (int i=0;i<MAXNODENUM;i++){
		myMapNodeP[i].nodeNo = i;
		myMapNodeP[i].OCost = -1;
		myMapNodeP[i].HETCost = -1;
		myMapNodeP[i].HOCost = -1;
		myMapNodeP[i].nodeET = -1;
		myMapNodeP[i].goal = FALSE;
		myMapNodeP[i].parentNo = -1;
		myMapNodeP[i].explored = FALSE;
		for (int j=0;j<MAXENGINENUM;j++){
			myMapNodeP[i].nodeETCost[j] = -1;
		}
		for (int j=0;j<MAXENGINENUM;j++)
			myMapNodeP[i].childNo[j] = -1;
		for (int j=0;j<MAXTASKNUM;j++){
			myMapNodeP[i].assignOrder[j] = -1;
			myMapNodeP[i].taskOrder[j] = -1;
		}
	}
	//initialization END-------------------------
	
	//explore the node#0
	myMapNodeP[0].nodeET=0;
	myMapNodeP[0].OCost=0;
	myMapNodeP[0].nodeNo=0;
	for (int i=0;i<engineNum;i++){
		myMapNodeP[0].nodeETCost[i]=0;
	}
	nextEmptyIndex++;
	NodeSearch(engineInfoP,taskInfoP,myMapNodeP,0,&nextEmptyIndex);
	
	//for(int a=0;a<10;a++){ //how much node do we explore
	while( finish == FALSE){
		//check for all explored or calculated node
		for (int i=0;i<nextEmptyIndex;i++){
			//leaf node which need to be calculated
			if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].OCost == -1){
				CalculateCost(engineInfoP,taskInfoP,myMapNodeP,i);
				if(lowestEdge>myMapNodeP[i].OCost){
					//printf("node %d cost got calculate, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].OCost;
					lowestNodeNo = i;
					//printf("node %d cost got calculate, update the lowestedge %f\n",i,lowestEdge);
				}
			}
			//leaf node which already calculated
			else if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].OCost != -1){
				if(lowestEdge>myMapNodeP[i].OCost){
					//printf("node %d cost got examed, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].OCost;
					lowestNodeNo = i;
					//printf("node %d cost got examed, update the lowestedge %f\n",i,lowestEdge);				
					//remeber to reset lowestNodeNo
				}
			}
			else if(myMapNodeP[i].explored == TRUE && myMapNodeP[i].nodeNo>largestBranchNode){//not leaf node
				largestBranchNode = myMapNodeP[i].nodeNo;
			}
		}
		NodeSearch(engineInfoP,taskInfoP,myMapNodeP,lowestNodeNo,&nextEmptyIndex);
		
		if(myMapNodeP[lowestNodeNo].goal == TRUE){
			finish = TRUE;
			goalNodeNo = lowestNodeNo;
		}
		lowestEdge = 10000;
		lowestNodeNo =-1;
	}

	//printf part
	/*
	for (int i =0;i<=goalNodeNo;i++){
		printf("node[%d] nodeNo is %d\n",i,myMapNodeP[i].nodeNo);
		printf("node[%d] OCost is %d\n",i,myMapNodeP[i].OCost);
		printf("node[%d] HETCost is %f\n",i,myMapNodeP[i].HETCost);
		printf("node[%d] HOCost is %d\n",i,myMapNodeP[i].HOCost);	
		printf("node[%d] nodeET is %f\n",i,myMapNodeP[i].nodeET);
		printf("node[%d] goal is %d\n",i,myMapNodeP[i].goal);
		printf("node[%d] explored is %d\n",i,myMapNodeP[i].explored);
		printf("node[%d] parentNo. is %d\n",i,myMapNodeP[i].parentNo);
		printf("node[%d] childNo is \n",i);
		for (int j=0;j<engineNum;j++){
			printf("%d ",myMapNodeP[i].childNo[j]);		
		}
		printf("\nnode[%d] nodeETCost is\n",i);
		for (int j=0;j<engineNum;j++){
			printf(" %f ",myMapNodeP[i].nodeETCost[j]);		
		}
		printf("\nnode[%d] assignOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].assignOrder[j]);		
		printf("\nnode[%d] taskOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].taskOrder[j]);		
		printf("\n");
	}
	
	if(finish == TRUE){
		printf("---------!found the goal node %d!------------\n",goalNodeNo);
		printf("total searched Num = %d",nextEmptyIndex);
	}
	*/
	PrintResult(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,0,nextEmptyIndex);
	return nextEmptyIndex;

}

//--------------------------------------------------------------
//-----------------------UCSBYET---------------------------------
//--------------------------------------------------------------

int UCSBYET(const engine *engineInfoP, const task *taskInfoP,mapNode *myMapNodeP)
{
	//char *dependSearchP;
	float lowestEdge=10000;
	int lowestNodeNo=-1;
	int nextEmptyIndex=0;
	int finish=FALSE,goalNodeNo = -1;
	int largestBranchNode=-1;
	int lowestGoalCost = 10000;
	
	
	//initialization structure-------------------------
	for (int i=0;i<MAXNODENUM;i++){
		myMapNodeP[i].nodeNo = i;
		myMapNodeP[i].OCost = -1;
		myMapNodeP[i].HETCost = -1;
		myMapNodeP[i].HOCost = -1;
		myMapNodeP[i].nodeET = -1;
		myMapNodeP[i].goal = FALSE;
		myMapNodeP[i].parentNo = -1;
		myMapNodeP[i].explored = FALSE;
		for (int j=0;j<MAXENGINENUM;j++){
			myMapNodeP[i].nodeETCost[j] = -1;
		}
		for (int j=0;j<MAXENGINENUM;j++)
			myMapNodeP[i].childNo[j] = -1;
		for (int j=0;j<MAXTASKNUM;j++){
			myMapNodeP[i].assignOrder[j] = -1;
			myMapNodeP[i].taskOrder[j] = -1;
		}
	}
	//initialization END-------------------------
	
	//explore the node#0
	myMapNodeP[0].nodeET=0;
	myMapNodeP[0].OCost=0;
	myMapNodeP[0].nodeNo=0;
	for (int i=0;i<engineNum;i++){
		myMapNodeP[0].nodeETCost[i]=0;
	}
	nextEmptyIndex++;
	NodeSearch(engineInfoP,taskInfoP,myMapNodeP,0,&nextEmptyIndex);
	
	//for(int a=0;a<10;a++){ //how much node do we explore
	while( finish == FALSE){
		//check for all explored or calculated node
		for (int i=0;i<nextEmptyIndex;i++){
			//leaf node which need to be calculated
			if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].nodeET == -1){
				CalculateCost(engineInfoP,taskInfoP,myMapNodeP,i);
				if(lowestEdge>myMapNodeP[i].nodeET){
					//printf("node %d cost got calculate, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].nodeET;
					lowestNodeNo = i;
					//printf("node %d cost got calculate, update the lowestedge %f\n",i,lowestEdge);
				}
			}
			//leaf node which already calculated
			else if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].nodeET != -1){
				if(lowestEdge>myMapNodeP[i].nodeET){
					//printf("node %d cost got examed, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].nodeET;
					lowestNodeNo = i;
					//printf("node %d cost got examed, update the lowestedge %f\n",i,lowestEdge);				
					//remeber to reset lowestNodeNo
				}
			}
			else if(myMapNodeP[i].nodeNo>largestBranchNode){//not leaf node
				largestBranchNode = myMapNodeP[i].nodeNo;
			}
		}
		NodeSearch(engineInfoP,taskInfoP,myMapNodeP,lowestNodeNo,&nextEmptyIndex);
		
		if(myMapNodeP[lowestNodeNo].goal == TRUE){
			finish = TRUE;
			goalNodeNo = lowestNodeNo;
		}
		lowestEdge = 10000;
		lowestNodeNo =-1;
	}

	//printf part
	/*
	for (int i =0;i<=goalNodeNo;i++){
		printf("node[%d] nodeNo is %d\n",i,myMapNodeP[i].nodeNo);
		printf("node[%d] OCost is %d\n",i,myMapNodeP[i].OCost);
		printf("node[%d] HETCost is %f\n",i,myMapNodeP[i].HETCost);
		printf("node[%d] HOCost is %d\n",i,myMapNodeP[i].HOCost);	
		printf("node[%d] nodeET is %f\n",i,myMapNodeP[i].nodeET);
		printf("node[%d] goal is %d\n",i,myMapNodeP[i].goal);
		printf("node[%d] explored is %d\n",i,myMapNodeP[i].explored);
		printf("node[%d] parentNo. is %d\n",i,myMapNodeP[i].parentNo);
		printf("node[%d] childNo is \n",i);
		for (int j=0;j<engineNum;j++){
			printf("%d ",myMapNodeP[i].childNo[j]);		
		}
		printf("\nnode[%d] nodeETCost is\n",i);
		for (int j=0;j<engineNum;j++){
			printf(" %f ",myMapNodeP[i].nodeETCost[j]);		
		}
		printf("\nnode[%d] assignOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].assignOrder[j]);		
		printf("\nnode[%d] taskOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].taskOrder[j]);		
		printf("\n");
	}
	
	if(finish == TRUE){
		printf("---------!found the goal node %d!------------\n",goalNodeNo);
		printf("total searched Num = %d\n",nextEmptyIndex);
	}
	*/
	PrintResult(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,1,nextEmptyIndex);
	return nextEmptyIndex;
	
}


//--------------------------------------------------------------
//-----------------------GSBYO---------------------------------
//--------------------------------------------------------------
int GSBYO(const engine *engineInfoP, const task *taskInfoP,mapNode *myMapNodeP)
{
	//char *dependSearchP;
	int lowestEdge=10000;
	int lowestNodeNo=-1;
	int nextEmptyIndex=0;
	int finish=FALSE,goalNodeNo = -1;
	int largestBranchNode=-1;
	int lowestGoalCost = 10000;
	
	
	//initialization structure-------------------------
	for (int i=0;i<MAXNODENUM;i++){
		myMapNodeP[i].nodeNo = i;
		myMapNodeP[i].OCost = -1;
		myMapNodeP[i].HETCost = -1;
		myMapNodeP[i].HOCost = -1;
		myMapNodeP[i].nodeET = -1;
		myMapNodeP[i].goal = FALSE;
		myMapNodeP[i].parentNo = -1;
		myMapNodeP[i].explored = FALSE;
		for (int j=0;j<MAXENGINENUM;j++){
			myMapNodeP[i].nodeETCost[j] = -1;
		}
		for (int j=0;j<MAXENGINENUM;j++)
			myMapNodeP[i].childNo[j] = -1;
		for (int j=0;j<MAXTASKNUM;j++){
			myMapNodeP[i].assignOrder[j] = -1;
			myMapNodeP[i].taskOrder[j] = -1;
		}
	}
	//initialization END-------------------------
	
	//explore the node#0
	myMapNodeP[0].nodeET=0;
	myMapNodeP[0].OCost=0;
	myMapNodeP[0].nodeNo=0;
	for (int i=0;i<engineNum;i++){
		myMapNodeP[0].nodeETCost[i]=0;
	}
	nextEmptyIndex++;
	NodeSearch(engineInfoP,taskInfoP,myMapNodeP,0,&nextEmptyIndex);
	
	//for(int a=0;a<10;a++){ //how much node do we explore
	while( finish == FALSE){
		//check for all explored or calculated node
		//first check if there is a goal
		for (int i=0;i<nextEmptyIndex;i++){
			if(myMapNodeP[i].goal == TRUE && finish == FALSE){
				finish = TRUE;
				goalNodeNo = i;
				//printf("we find the GOAL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!finish is %d,goalNode=%d\n",finish,goalNodeNo);
			}
			//leaf node which need to be calculated
			else if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].HOCost == -1){
				CalculateCost(engineInfoP,taskInfoP,myMapNodeP,i);
				if(lowestEdge>myMapNodeP[i].HOCost){
					//printf("node %d cost got calculate, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].HOCost;
					lowestNodeNo = i;
					//printf("node %d cost got calculate, update the lowestedge %f\n",i,lowestEdge);
				}
			}
			//leaf node which already calculated
			else if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].HOCost != -1){
				if(lowestEdge>myMapNodeP[i].HOCost){
					//printf("node %d cost got examed, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].HOCost;
					lowestNodeNo = i;
					//printf("node %d cost got examed, update the lowestedge %f\n",i,lowestEdge);				
					//remeber to reset lowestNodeNo
				}
			}
			else if(myMapNodeP[i].nodeNo>largestBranchNode){//not leaf node
				largestBranchNode = myMapNodeP[i].nodeNo;
			}
		}
		if(finish == FALSE)
			NodeSearch(engineInfoP,taskInfoP,myMapNodeP,lowestNodeNo,&nextEmptyIndex);	
		else{
			NodeSearch(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,&nextEmptyIndex);
			CalculateCost(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo);				
		}
		
		/*
		for(int i=0;i<engineNum;i++){
			if(myMapNodeP[myMapNodeP[lowestNodeNo].childNo[i]].goal == TRUE){
				finish = TRUE;
				goalNodeNo = myMapNodeP[lowestNodeNo].childNo[i];
				printf("we find the GOAL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!finish is %d,goalNode=%d\n",finish,goalNodeNo);
				NodeSearch(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,&nextEmptyIndex);
				CalculateCost(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo);
			}
		}
		
		if(myMapNodeP[lowestNodeNo].goal == TRUE){
			finish = TRUE;
			goalNodeNo = lowestNodeNo;
		}
		*/
		lowestEdge = 10000;
		lowestNodeNo =-1;
	}

	//printf part
	/*
	for (int i =0;i<=goalNodeNo;i++){
		printf("node[%d] nodeNo is %d\n",i,myMapNodeP[i].nodeNo);
		printf("node[%d] OCost is %d\n",i,myMapNodeP[i].OCost);
		printf("node[%d] HETCost is %f\n",i,myMapNodeP[i].HETCost);
		printf("node[%d] HOCost is %d\n",i,myMapNodeP[i].HOCost);	
		printf("node[%d] nodeET is %f\n",i,myMapNodeP[i].nodeET);
		printf("node[%d] goal is %d\n",i,myMapNodeP[i].goal);
		printf("node[%d] explored is %d\n",i,myMapNodeP[i].explored);
		printf("node[%d] parentNo. is %d\n",i,myMapNodeP[i].parentNo);
		printf("node[%d] childNo is \n",i);
		for (int j=0;j<engineNum;j++){
			printf("%d ",myMapNodeP[i].childNo[j]);		
		}
		printf("\nnode[%d] nodeETCost is\n",i);
		for (int j=0;j<engineNum;j++){
			printf(" %f ",myMapNodeP[i].nodeETCost[j]);		
		}
		printf("\nnode[%d] assignOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].assignOrder[j]);		
		printf("\nnode[%d] taskOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].taskOrder[j]);		
		printf("\n");
	}
	
	if(finish == TRUE){
		printf("---------!found the goal node %d!------------\n",goalNodeNo);
		printf("total searched Num = %d\n",nextEmptyIndex);
		
	}
	*/
	PrintResult(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,2,nextEmptyIndex);
	return nextEmptyIndex;

}

//--------------------------------------------------------------
//-----------------------GSBYET---------------------------------
//--------------------------------------------------------------

int GSBYET(const engine *engineInfoP, const task *taskInfoP,mapNode *myMapNodeP)
{
	//char *dependSearchP;
	float lowestEdge=10000;
	int lowestNodeNo=-1;
	int nextEmptyIndex=0;
	int finish=FALSE,goalNodeNo = -1;
	int largestBranchNode=-1;
	int lowestGoalCost = 10000;
	
	
	//initialization structure-------------------------
	for (int i=0;i<MAXNODENUM;i++){
		myMapNodeP[i].nodeNo = i;
		myMapNodeP[i].OCost = -1;
		myMapNodeP[i].HETCost = -1;
		myMapNodeP[i].HOCost = -1;
		myMapNodeP[i].nodeET = -1;
		myMapNodeP[i].goal = FALSE;
		myMapNodeP[i].parentNo = -1;
		myMapNodeP[i].explored = FALSE;
		for (int j=0;j<MAXENGINENUM;j++){
			myMapNodeP[i].nodeETCost[j] = -1;
		}
		for (int j=0;j<MAXENGINENUM;j++)
			myMapNodeP[i].childNo[j] = -1;
		for (int j=0;j<MAXTASKNUM;j++){
			myMapNodeP[i].assignOrder[j] = -1;
			myMapNodeP[i].taskOrder[j] = -1;
		}
	}
	//initialization END-------------------------
	
	//explore the node#0
	myMapNodeP[0].nodeET=0;
	myMapNodeP[0].OCost=0;
	myMapNodeP[0].nodeNo=0;
	for (int i=0;i<engineNum;i++){
		myMapNodeP[0].nodeETCost[i]=0;
	}
	nextEmptyIndex++;
	NodeSearch(engineInfoP,taskInfoP,myMapNodeP,0,&nextEmptyIndex);
	
	//for(int a=0;a<10;a++){ //how much node do we explore
	while( finish == FALSE){
		//check for all explored or calculated node
		//first check if there is a goal
		for (int i=0;i<nextEmptyIndex;i++){
			if(myMapNodeP[i].goal == TRUE && finish == FALSE){
				finish = TRUE;
				goalNodeNo = i;
				//printf("we find the GOAL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!finish is %d,goalNode=%d\n",finish,goalNodeNo);
			}
			//leaf node which need to be calculated
			else if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].HETCost == -1){
				CalculateCost(engineInfoP,taskInfoP,myMapNodeP,i);
				if(lowestEdge>myMapNodeP[i].HETCost){
					//printf("node %d cost got calculate, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].HETCost;
					lowestNodeNo = i;
					//printf("node %d cost got calculate, update the lowestedge %f\n",i,lowestEdge);
				}
			}
			//leaf node which already calculated
			else if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].HETCost != -1){
				if(lowestEdge>myMapNodeP[i].HETCost){
					//printf("node %d cost got examed, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].HETCost;
					lowestNodeNo = i;
					//printf("node %d cost got examed, update the lowestedge %f\n",i,lowestEdge);				
					//remeber to reset lowestNodeNo
				}
			}
			else if(myMapNodeP[i].nodeNo>largestBranchNode){//not leaf node
				largestBranchNode = myMapNodeP[i].nodeNo;
			}
		}
		if(finish == FALSE)
			NodeSearch(engineInfoP,taskInfoP,myMapNodeP,lowestNodeNo,&nextEmptyIndex);	
		else{
			NodeSearch(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,&nextEmptyIndex);
			CalculateCost(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo);				
		}
		
		/*
		for(int i=0;i<engineNum;i++){
			if(myMapNodeP[myMapNodeP[lowestNodeNo].childNo[i]].goal == TRUE){
				finish = TRUE;
				goalNodeNo = myMapNodeP[lowestNodeNo].childNo[i];
				printf("we find the GOAL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!finish is %d,goalNode=%d\n",finish,goalNodeNo);
				NodeSearch(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,&nextEmptyIndex);
				CalculateCost(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo);
			}
		}
		
		if(myMapNodeP[lowestNodeNo].goal == TRUE){
			finish = TRUE;
			goalNodeNo = lowestNodeNo;
		}
		*/
		lowestEdge = 10000;
		lowestNodeNo =-1;
	}

	//printf part
	/*
	for (int i =0;i<=goalNodeNo;i++){
		printf("node[%d] nodeNo is %d\n",i,myMapNodeP[i].nodeNo);
		printf("node[%d] OCost is %d\n",i,myMapNodeP[i].OCost);
		printf("node[%d] HETCost is %f\n",i,myMapNodeP[i].HETCost);
		printf("node[%d] HOCost is %d\n",i,myMapNodeP[i].HOCost);	
		printf("node[%d] nodeET is %f\n",i,myMapNodeP[i].nodeET);
		printf("node[%d] goal is %d\n",i,myMapNodeP[i].goal);
		printf("node[%d] explored is %d\n",i,myMapNodeP[i].explored);
		printf("node[%d] parentNo. is %d\n",i,myMapNodeP[i].parentNo);
		printf("node[%d] childNo is \n",i);
		for (int j=0;j<engineNum;j++){
			printf("%d ",myMapNodeP[i].childNo[j]);		
		}
		printf("\nnode[%d] nodeETCost is\n",i);
		for (int j=0;j<engineNum;j++){
			printf(" %f ",myMapNodeP[i].nodeETCost[j]);		
		}
		printf("\nnode[%d] assignOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].assignOrder[j]);		
		printf("\nnode[%d] taskOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].taskOrder[j]);		
		printf("\n");
	}
	
	if(finish == TRUE){
		printf("---------!found the goal node %d!------------\n",goalNodeNo);
		printf("total searched Num = %d\n",nextEmptyIndex);
		
	}
	*/
		PrintResult(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,3,nextEmptyIndex);
		return nextEmptyIndex;

}



//===============================================================
//============================ASBYO==============================
//===============================================================

int ASBYO(const engine *engineInfoP, const task *taskInfoP,mapNode *myMapNodeP)
{
	//char *dependSearchP;
	int lowestEdge=10000;
	int lowestNodeNo=-1;
	int nextEmptyIndex=0;
	int finish=FALSE,goalNodeNo = -1;
	int largestBranchNode=-1;
	int lowestGoalCost = 10000;
	
	
	//initialization structure-------------------------
	for (int i=0;i<MAXNODENUM;i++){
		myMapNodeP[i].nodeNo = i;
		myMapNodeP[i].OCost = -1;
		myMapNodeP[i].HETCost = -1;
		myMapNodeP[i].HOCost = -1;
		myMapNodeP[i].nodeET = -1;
		myMapNodeP[i].goal = FALSE;
		myMapNodeP[i].parentNo = -1;
		myMapNodeP[i].explored = FALSE;
		for (int j=0;j<MAXENGINENUM;j++){
			myMapNodeP[i].nodeETCost[j] = -1;
		}
		for (int j=0;j<MAXENGINENUM;j++)
			myMapNodeP[i].childNo[j] = -1;
		for (int j=0;j<MAXTASKNUM;j++){
			myMapNodeP[i].assignOrder[j] = -1;
			myMapNodeP[i].taskOrder[j] = -1;
		}
	}
	//initialization END-------------------------
	
	//explore the node#0
	myMapNodeP[0].nodeET=0;
	myMapNodeP[0].OCost=0;
	myMapNodeP[0].nodeNo=0;
	for (int i=0;i<engineNum;i++){
		myMapNodeP[0].nodeETCost[i]=0;
	}
	nextEmptyIndex++;
	NodeSearch(engineInfoP,taskInfoP,myMapNodeP,0,&nextEmptyIndex);
	
	//for(int a=0;a<10;a++){ //how much node do we explore
	while( finish == FALSE){
		//check for all explored or calculated node
		for (int i=0;i<nextEmptyIndex;i++){
			//leaf node which need to be calculated
			if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].HOCost == -1){
				CalculateCost(engineInfoP,taskInfoP,myMapNodeP,i);
				if(lowestEdge>myMapNodeP[i].HOCost){
					//printf("node %d cost got calculate, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].HOCost;
					lowestNodeNo = i;
					//printf("node %d cost got calculate, update the lowestedge %f\n",i,lowestEdge);
				}
			}
			//leaf node which already calculated
			else if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].HOCost != -1){
				if(lowestEdge>myMapNodeP[i].HOCost){
					//printf("node %d cost got examed, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].HOCost;
					lowestNodeNo = i;
					//printf("node %d cost got examed, update the lowestedge %f\n",i,lowestEdge);				
					//remeber to reset lowestNodeNo
				}
			}
			else if(myMapNodeP[i].nodeNo>largestBranchNode){//not leaf node
				largestBranchNode = myMapNodeP[i].nodeNo;
			}
		}
		NodeSearch(engineInfoP,taskInfoP,myMapNodeP,lowestNodeNo,&nextEmptyIndex);
		
		if(myMapNodeP[lowestNodeNo].goal == TRUE){
			finish = TRUE;
			goalNodeNo = lowestNodeNo;
		}
		lowestEdge = 10000;
		lowestNodeNo =-1;
	}

	//printf part
	/*
	for (int i =0;i<=goalNodeNo;i++){
		printf("node[%d] nodeNo is %d\n",i,myMapNodeP[i].nodeNo);
		printf("node[%d] OCost is %d\n",i,myMapNodeP[i].OCost);
		printf("node[%d] HETCost is %f\n",i,myMapNodeP[i].HETCost);
		printf("node[%d] HOCost is %d\n",i,myMapNodeP[i].HOCost);	
		printf("node[%d] nodeET is %f\n",i,myMapNodeP[i].nodeET);
		printf("node[%d] goal is %d\n",i,myMapNodeP[i].goal);
		printf("node[%d] explored is %d\n",i,myMapNodeP[i].explored);
		printf("node[%d] parentNo. is %d\n",i,myMapNodeP[i].parentNo);
		printf("node[%d] childNo is \n",i);
		for (int j=0;j<engineNum;j++){
			printf("%d ",myMapNodeP[i].childNo[j]);		
		}
		printf("\nnode[%d] nodeETCost is\n",i);
		for (int j=0;j<engineNum;j++){
			printf(" %f ",myMapNodeP[i].nodeETCost[j]);		
		}
		printf("\nnode[%d] assignOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].assignOrder[j]);		
		printf("\nnode[%d] taskOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].taskOrder[j]);		
		printf("\n");
	}
	
	if(finish == TRUE){
		printf("---------!found the goal node %d!------------\n",goalNodeNo);
		printf("total searched Num = %d",nextEmptyIndex);
		
	}
	*/
	PrintResult(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,4,nextEmptyIndex);
	return nextEmptyIndex;
}

//===============================================================
//============================ASBYET==============================
//===============================================================
int ASBYET(const engine *engineInfoP, const task *taskInfoP,mapNode *myMapNodeP)
{
	//char *dependSearchP;
	float lowestEdge=10000;
	int lowestNodeNo=-1;
	int nextEmptyIndex=0;
	int finish=FALSE,goalNodeNo = -1;
	int largestBranchNode=-1;
	//int exploredSet[MAXNODENUM];
	
	//initialization structure-------------------------
	for (int i=0;i<MAXNODENUM;i++){
		myMapNodeP[i].nodeNo = i;
		myMapNodeP[i].OCost = -1;
		myMapNodeP[i].HETCost = -1;
		myMapNodeP[i].HOCost = -1;
		myMapNodeP[i].nodeET = -1;
		myMapNodeP[i].goal = FALSE;
		myMapNodeP[i].parentNo = -1;
		myMapNodeP[i].explored = FALSE;
		for (int j=0;j<MAXENGINENUM;j++){
			myMapNodeP[i].nodeETCost[j] = -1;
		}
		for (int j=0;j<MAXENGINENUM;j++)
			myMapNodeP[i].childNo[j] = -1;
		for (int j=0;j<MAXTASKNUM;j++){
			myMapNodeP[i].assignOrder[j] = -1;
			myMapNodeP[i].taskOrder[j] = -1;
		}
	}
	//initialization END-------------------------
	
	//explore the node#0
	myMapNodeP[0].nodeET=0;
	myMapNodeP[0].OCost=0;
	myMapNodeP[0].nodeNo=0;
	for (int i=0;i<engineNum;i++){
		myMapNodeP[0].nodeETCost[i]=0;
	}
	nextEmptyIndex++;
	NodeSearch(engineInfoP,taskInfoP,myMapNodeP,0,&nextEmptyIndex);
	
	//for(int a=0;a<10;a++){ //how much node do we explore
	while( finish == FALSE){
		//check for all explored or calculated node
		for (int i=0;i<nextEmptyIndex;i++){
			//leaf node which need to be calculated
			if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].HETCost == -1){
				CalculateCost(engineInfoP,taskInfoP,myMapNodeP,i);
				if(lowestEdge>myMapNodeP[i].HETCost){
					//printf("node %d cost got calculate, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].HETCost;
					lowestNodeNo = i;
					//printf("node %d cost got calculate, update the lowestedge %f\n",i,lowestEdge);
				}
			}
			//leaf node which already calculated
			else if(myMapNodeP[i].explored == FALSE && myMapNodeP[i].HETCost != -1){
				if(lowestEdge>myMapNodeP[i].HETCost){
					//printf("node %d cost got examed, update the lowestedge from %f\n",i,lowestEdge);
					lowestEdge = myMapNodeP[i].HETCost;
					lowestNodeNo = i;
					//printf("node %d cost got examed, update the lowestedge %f\n",i,lowestEdge);				
					//remeber to reset lowestNodeNo
				}
			}
			else if(myMapNodeP[i].explored == TRUE && myMapNodeP[i].nodeNo>largestBranchNode){//not leaf node
				largestBranchNode = myMapNodeP[i].nodeNo;
			}
		}
		NodeSearch(engineInfoP,taskInfoP,myMapNodeP,lowestNodeNo,&nextEmptyIndex);
		
		if(myMapNodeP[lowestNodeNo].goal == TRUE){
			finish = TRUE;
			goalNodeNo = lowestNodeNo;
		}
		lowestEdge = 10000;
		lowestNodeNo =-1;
	}

	//printf part
	/*
	for (int i =0;i<=goalNodeNo;i++){
		printf("node[%d] nodeNo is %d\n",i,myMapNodeP[i].nodeNo);
		printf("node[%d] OCost is %d\n",i,myMapNodeP[i].OCost);
		printf("node[%d] HETCost is %f\n",i,myMapNodeP[i].HETCost);
		printf("node[%d] HOCost is %d\n",i,myMapNodeP[i].HOCost);	
		printf("node[%d] nodeET is %f\n",i,myMapNodeP[i].nodeET);
		printf("node[%d] goal is %d\n",i,myMapNodeP[i].goal);
		printf("node[%d] explored is %d\n",i,myMapNodeP[i].explored);
		printf("node[%d] parentNo. is %d\n",i,myMapNodeP[i].parentNo);
		printf("node[%d] childNo is \n",i);
		for (int j=0;j<engineNum;j++){
			printf("%d ",myMapNodeP[i].childNo[j]);		
		}
		printf("\nnode[%d] nodeETCost is\n",i);
		for (int j=0;j<engineNum;j++){
			printf(" %f ",myMapNodeP[i].nodeETCost[j]);		
		}
		printf("\nnode[%d] assignOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].assignOrder[j]);		
		printf("\nnode[%d] taskOrder is\n",i);
		for (int j=0;j<taskNum;j++)
			printf(" %d ",myMapNodeP[i].taskOrder[j]);		
		printf("\n");
	}
	
	if(finish == TRUE){
		printf("---------!found the goal node %d!------------\n",goalNodeNo);
		printf("total searched Num = %d",nextEmptyIndex);
		
	}
	*/
	PrintResult(engineInfoP,taskInfoP,myMapNodeP,goalNodeNo,5,nextEmptyIndex);
	return nextEmptyIndex;
}


//--------------------------------------------------------------
//-----------------------NodeSearch---------------------------------
//--------------------------------------------------------------

void NodeSearch(const engine *engineInfoP,const task *taskInfoP,mapNode *searchNodeP,int offset, int* nextEmpty)
{
	int completedNum=0,availInEngine=FALSE;//completed show how much task has assigned for current
	//availIndex may cause overflow
	availTask myAvailTask[MAXTASKNUM];
	int availIndex=0,childIndex=0;	//point to next avail position/child for current node
	int nextEmptyAssign=0;
	int currentTaskIndex=0;//means the next task we gonna assigned
	mapNode *currentNodeP;
	currentNodeP = searchNodeP+offset;

	for(int i=0;i<MAXTASKNUM;i++){
		myAvailTask[i].availTaskNo = -1;
		for(int j=0;j<MAXENGINENUM;j++)
	 		myAvailTask[i].availEngineNo[j] = -1;//1 means avail
	}

	//printf("searchNodeP is No. %d\n",offset);
	//printf("next empty node is %d before searchign\n",*nextEmpty);
	currentNodeP->explored = TRUE;
	for (int i=0;i<taskNum;i++){
		if(currentNodeP->assignOrder[i] != -1)
			completedNum ++;
	}
	if(completedNum == taskNum)
		currentNodeP->goal = TRUE;
	if (currentNodeP ->goal == FALSE){
		//increase until we got the next task should be assigned
		while (currentNodeP->taskOrder[currentTaskIndex] != -1){
			currentTaskIndex++;
		}
		//currentNodeP->taskOrder[currentTaskIndex] = currentTaskIndex;
		for(int i=0;i<engineNum;i++){
			if(engineInfoP[i].mem_size >= taskInfoP[currentTaskIndex].reqMem){
				myAvailTask[currentTaskIndex].availEngineNo[i] = 1;
			}
		}
		for (int i=0;i<engineNum;i++){
			if(myAvailTask[currentTaskIndex].availEngineNo[i] == 1){
				currentNodeP->childNo[i]= *nextEmpty;
				searchNodeP[*nextEmpty].parentNo =currentNodeP->nodeNo;
				for(int k=0;k<taskNum;k++){
					if(currentNodeP->taskOrder[k] != -1){
						searchNodeP[*nextEmpty].taskOrder[k] =currentNodeP->taskOrder[k];
					}
					if(currentNodeP->assignOrder[k] != -1){
						searchNodeP[*nextEmpty].assignOrder[k] =currentNodeP->assignOrder[k];
					}
				}
				searchNodeP[*nextEmpty].assignOrder[currentTaskIndex] = i;
				searchNodeP[*nextEmpty].taskOrder[currentTaskIndex] = currentTaskIndex;
				if(currentTaskIndex == taskNum-1){
					searchNodeP[*nextEmpty].goal= TRUE;
				}
				//printf("create next child node %d\n",*nextEmpty);
				(*nextEmpty)++;
			}
		}
	}
	/*
	for(int i=0;i<taskNum;i++){
		printf("availtask[%d] -> available Task No: %d\n",i,i);
		for(int j=0;j<engineNum;j++)
			printf("availtask[%d] -> avail in Engine No: %d value = %d\n",i,j,myAvailTask[i].availEngineNo[j]);
	}
	*/
}


void CalculateCost(const engine *engineInfoP,const task *taskInfoP,mapNode *mapNodeP,int offset)
{

	mapNode *currentNodeP;
	char *dependTaskP;
	float highestET=0;
	int totalCostO = 0;
	//initialize
	currentNodeP = mapNodeP+offset;
	currentNodeP->nodeET =0;
	//currentNodeP->OCost =0;
	for(int i=0;i<engineNum;i++){
		currentNodeP->nodeETCost[i]=0;
	}
	//loop for all the assigned tasks to calculate ET
	for(int i=0;i<taskNum;i++){ 
		if(currentNodeP->assignOrder[i] != -1 && currentNodeP->taskOrder[i] != -1 ){
			currentNodeP->nodeETCost[currentNodeP->assignOrder[i]] += (float)(taskInfoP[currentNodeP->taskOrder[i]].reqInstruction)/(engineInfoP[currentNodeP->assignOrder[i]].speed);
			currentNodeP->nodeETCost[currentNodeP->assignOrder[i]] += (float)10/(engineInfoP[currentNodeP->assignOrder[i]].speed);
		}
	}
	//select the highest ET
	for(int i=0;i<taskNum;i++){
		if(currentNodeP->nodeETCost[i] != -1){
			if(currentNodeP->nodeETCost[i] > highestET){
				highestET = currentNodeP->nodeETCost[i];
			}
		}
	}
	currentNodeP->nodeET = highestET;
	
	//loop for all the assigned tasks to calculate O
	//i means the assigned task we are looking for, j is for its depend node assigned
	for(int i=0;i<taskNum;i++){ 
		if(currentNodeP->assignOrder[i] != -1 && currentNodeP->taskOrder[i] != -1 ){
			dependTaskP = taskInfoP[currentNodeP->taskOrder[i]].preReqTask;
			while(*dependTaskP != '\0'){
				//check all assigned tasks check whether this depend is assigned to different one
				for(int j=0;j<taskNum;j++){
					if(currentNodeP->taskOrder[j] == (((int)*dependTaskP) -64) && currentNodeP->taskOrder[j] != -1){
						if(currentNodeP->assignOrder[j] != currentNodeP->assignOrder[i]){
							totalCostO += taskInfoP[currentNodeP->taskOrder[j]].outputSize;
						}
					}
				}
				dependTaskP++;
			}
		}
	}
	currentNodeP->OCost = totalCostO;
	
	//call H function to get the H value
	HFunction(engineInfoP, taskInfoP, mapNodeP,offset);
}

void HFunction(const engine *engineInfoP, const task *taskInfoP,mapNode *mapNodeInfoP,int offset)
{
	mapNode *currentNodeP;
	currentNodeP = offset+mapNodeInfoP;
	int estimateO=-1,assignedNum=0;
	float uniEngineLoad[MAXENGINENUM];
	int availEngine[MAXENGINENUM];
	float timeLeft=0;
	float highestTime = 0,secondHighestTime = 0,lowestTime = 10000;
	float totalInstruction =0,totalSpeed=0,totalInstructionLeft=0;
	float bestTime=0,secondBestTime=0;
	float fastSpeed=0;
	int fastEngine = 0,addIndex = 0;
	int notEqual = FALSE;


	//initialize
	for (int i=0;i<MAXENGINENUM;i++){
		uniEngineLoad[i] = 0;
		availEngine[i] = 0;
	}

	//H for O
	if(currentNodeP->OCost != 0){
		estimateO = 10000;
	}
	else{
		for(int i=0;i<taskNum;i++){
			if(currentNodeP->taskOrder[i] != -1){
				assignedNum++;
			}
		}
		estimateO = taskNum - assignedNum;
	}
	currentNodeP->HOCost = estimateO;

	
	
	//H for ET (best time achieveable for current state)
	//get current assigned load & unassigned load
	for (int i=0;i<taskNum;i++){
		if(currentNodeP->taskOrder[i] != -1){//assigned taskNo i
			//uniEngineLoad[currentNodeP->assignOrder[i]] += 
			//	(float)(10+taskInfoP[currentNodeP->taskOrder[i]].reqInstruction)/(float)(engineInfoP[currentNodeP->assignOrder[i]].speed);
			totalInstruction += (taskInfoP[i].reqInstruction+10);
		}
		else{ //not assigned taskNo i
			totalInstructionLeft += (taskInfoP[i].reqInstruction+10);
			totalInstruction += (taskInfoP[i].reqInstruction+10);
			//printf("assigning task %c for node %d totalInstructionLeft is %d\n",(char)(i+65),offset,totalInstructionLeft);
		}
	}
	
	
	//Aother working H Algorithm
	/*
	for(int i=0;i<engineNum;i++){
		uniEngineLoad[i] = currentNodeP->nodeETCost[i];
		totalSpeed += engineInfoP[i].speed;
	}
	while(totalInstructionLeft != 0){		
		
			for(int j=0;j<engineNum;j++){
				if((uniEngineLoad[j] + (float)1/(float)engineInfoP[j].speed) < lowestTime ){
					lowestTime = (uniEngineLoad[j] + (float)1/(float)engineInfoP[j].speed); //sb be highest or no body is
					addIndex = j;
				}
			}
			
			//for(int j=0;j<engineNum;j++){
			//	if(uniEngineLoad[j] < highestTime){
					uniEngineLoad[addIndex] += (float)1/(float)engineInfoP[addIndex].speed;
			//	}
			//}
			notEqual = FALSE;
			lowestTime = 10000;
		
		totalInstructionLeft--;
	}

	for(int i=0;i<engineNum;i++){
		if(uniEngineLoad[i]>highestTime){
			highestTime = uniEngineLoad[i];	
		}
	}
	*/	
	
	//Original H Algorithm
	
	for(int i=0;i<engineNum;i++){
		uniEngineLoad[i] = currentNodeP->nodeETCost[i];
		totalSpeed += engineInfoP[i].speed;
		if(uniEngineLoad[i]>highestTime){
			highestTime = uniEngineLoad[i];
		}
	}
	
	for(int i=0;i<engineNum;i++){
		if(uniEngineLoad[i]<highestTime){
			if(totalInstructionLeft > ((highestTime - uniEngineLoad[i])*(float)engineInfoP[i].speed)){
				totalInstructionLeft -= ((highestTime - uniEngineLoad[i])*(float)engineInfoP[i].speed);
			}
			else{
				totalInstructionLeft = 0;
			}
		}
	}

	
	timeLeft = totalInstructionLeft/totalSpeed;
	bestTime = totalInstruction/totalSpeed;
	secondBestTime = highestTime + timeLeft;
	//printf("current highestTime fornode %d is %f totalInstructionLeft is %d\n",offset,highestTime,totalInstructionLeft);	
	
	//if(currentNodeP->nodeET > bestTime){
	//	currentNodeP->HETCost = highestTime + timeLeft + (currentNodeP->nodeET - bestTime);
	//}
	//else
	//	currentNodeP->HETCost = highestTime + timeLeft;

	//printf("node %d time left is %f\n",offset,timeLeft);
	//printf("node %d best time is %f\n",offset,bestTime);
	//printf("node %d actual H time is %f\n",offset,currentNodeP->nodeET);
	
		currentNodeP->HETCost = secondBestTime;
}


void PrintResult(const engine *engineInfoP, const task *taskInfoP,mapNode *myMapNodeP,int offset,int type,int nextEmpty)
{
	mapNode *currentNodeP;
	currentNodeP = offset+myMapNodeP;

	/*
	printf("==============FIND the GOAL NODE==============\n");
	switch(type){
		case 0:
			printf("========SEARCHING METHOD UCS-O========\n");
			break;
		case 1:
			printf("========SEARCHING METHOD UCS-ET========\n");
			break;
		case 2:
			printf("========SEARCHING METHOD GS-O=========\n");
			break;
		case 3:	
			printf("========SEARCHING METHOD GS-ET========\n");
			break;
		case 4:	
			printf("========SEARCHING METHOD AS-O=========\n");
			break;
		case 5:
			printf("========SEARCHING METHOD AS-ET========\n");
			break;
		default:
			printf("========ERROR TYPING CODE!!!==========\n");
			break;		
	}
	printf("=============================================\n");
	
	printf("GOAL NODE NO. is %d\n",offset);
	printf("TOTAL SEARCHING NUM is %d\n",nextEmpty);
	printf("\n");
	
	printf("GOAL NODE INFO:\n");
	printf("node[%d] OCost is %d\n",offset,myMapNodeP[offset].OCost);
	printf("node[%d] HETCost is %f\n",offset,myMapNodeP[offset].HETCost);
	printf("node[%d] HOCost is %d\n",offset,myMapNodeP[offset].HOCost);	
	printf("node[%d] nodeET is %f\n",offset,myMapNodeP[offset].nodeET);
	printf("node[%d] goal is %d\n",offset,myMapNodeP[offset].goal);
	printf("node[%d] explored is %d\n",offset,myMapNodeP[offset].explored);
	printf("node[%d] parentNo. is %d\n",offset,myMapNodeP[offset].parentNo);
	printf("node[%d] childNo is \n",offset);
	for (int j=0;j<engineNum;j++){
		printf("%d ",myMapNodeP[offset].childNo[j]);		
	}
	printf("\nnode[%d] nodeETCost is\n",offset);
	for (int j=0;j<engineNum;j++){
		printf(" %f ",myMapNodeP[offset].nodeETCost[j]);		
	}
	printf("\n");
	for(int i=0;i<engineNum;i++){
		printf("Engine %d :",i);
		for(int j=0;j<taskNum;j++){ //j is the taskNo
			if(myMapNodeP[offset].assignOrder[j]==i && myMapNodeP[offset].assignOrder[j] != -1)
				printf(" %c ",(char)(j+65));
		}
		printf("\n");
	}	
	
	*/
	CleanUp(engineInfoP,taskInfoP,myMapNodeP,offset,type);
	
}



void CleanUp(const engine *engineInfoP, const task *taskInfoP,mapNode *myMapNodeP,int offset,int type)
{	
	FILE *outputTXT;
	char *fileName;
	fileName = (char*)malloc(sizeof(char)*15);
	mapNode *currentNodeP;
	currentNodeP = myMapNodeP + offset;
	float totalSwitchingCost = 0;
	char *searchTaskP;
	int priorityList[MAXTASKNUM];//low mean less dependcy,taskNum is the highest value
	for(int i=0;i<MAXTASKNUM;i++){
		priorityList[i] = -1;
	}
	int selectedEngine = -1;//this is only useful in ET
	int depdendNo = -1;


	for(int i=0;i<engineNum;i++){
		if(currentNodeP->nodeET == currentNodeP->nodeETCost[i])
		selectedEngine = i;
	}

	for(int i=0;i<taskNum;i++){ //for all tasks give them Plist & calc switching cost
		searchTaskP = taskInfoP[i].preReqTask;
		if(*searchTaskP == '\0'){ //task X available without depend
			priorityList[i] = 0;
		}
		else{
			priorityList[i] = strlen(searchTaskP);
			while(*searchTaskP != '\0'){
				depdendNo = ((int)*searchTaskP) - 64;
				//outgoing arrow
				if(currentNodeP->assignOrder[depdendNo] == selectedEngine && currentNodeP->assignOrder[i] != selectedEngine){
					totalSwitchingCost += (float)20/(float)(engineInfoP[selectedEngine].speed);
				}
				//incoming arrow from currentNodeP->assignOrder[depdendNo]
				if(currentNodeP->assignOrder[depdendNo] != selectedEngine && currentNodeP->assignOrder[i] == selectedEngine){
					totalSwitchingCost += (float)20/(float)(engineInfoP[currentNodeP->assignOrder[depdendNo]].speed);
				}
				searchTaskP++;
			}
		}
	}
	
	/*
	printf("\n======================REORDER PART====================\n");
	printf("Adding Total Switching Cost for goal node %d is %f\n",offset,totalSwitchingCost);
	printf("Task Priority Order is: \n");
	for(int i=0;i<taskNum;i++){
		for(int j=0;j<taskNum;j++){
			if(priorityList[j] == i ){
				printf(" %c(%d)",(char)(j+65),priorityList[j]);
			}
		}
	}
	printf("\n");

	printf("ReOrdered Task assign order is: \n");
	for(int i=0;i<engineNum;i++){
		printf("Engine %d :",i);
		for(int k=0;k<taskNum;k++){//k is the priority from highest to lowest
			for(int j=0;j<taskNum;j++){ //j is the taskNo
				if(myMapNodeP[offset].assignOrder[j]==i && myMapNodeP[offset].assignOrder[j] != -1 && myMapNodeP[offset].assignOrder[j] == k){
					printf(" %c ",(char)(j+65));
				}
			}
		}
		printf("\n");
	}
	*/

	//File output

	switch(type){
		case 0:
			strcpy(fileName,"UCS-O.txt");
			break;
		case 1:
			strcpy(fileName,"UCS-ET.txt");
			break;
		case 2:
			strcpy(fileName,"GS-O.txt");
			break;
		case 3:	
			strcpy(fileName,"GS-ET.txt");
			break;
		case 4:	
			strcpy(fileName,"AS-O.txt");
			break;
		case 5:
			strcpy(fileName,"AS-ET.txt");
			break;
		default:
			printf("========ERROR TYPING CODE!!!==========\n");
			break;		
	}
	if((outputTXT = fopen(fileName,"w+")) == NULL){
		printf("===========================================\n");
		printf("OPEN FAILURE FOR WRITING TO %s \n",fileName);
		printf("===========================================\n");
	}
	else{
		printf("writing to file %s successfully\n",fileName);
		fprintf(outputTXT,"plan-01:\n");
		if (type == 0 || type == 4 ){
			fprintf(outputTXT,"Optimize O and O is Optimal\n");
		}
		else if(type == 2){
			fprintf(outputTXT,"Optimize O \n");
		}
		else if(type == 1 || type == 5){
			fprintf(outputTXT,"Optimize ET\n");
		}
		else{
			fprintf(outputTXT,"Optimize ET and ET is Optimal\n");
		}
		fprintf(outputTXT,"Cost for O is: %d\n",currentNodeP->OCost);
		if(type == 1 || type == 3 || type == 5)
			fprintf(outputTXT,"ET is :%f    (original cost %f + switching cost %f)\n",((currentNodeP->nodeET) + totalSwitchingCost),currentNodeP->nodeET,totalSwitchingCost);	
		
		fprintf(outputTXT,"Task Assignment Order is :\n");
		for(int i=0;i<engineNum;i++){
			fprintf(outputTXT,"Engine %d :",i);
			for(int k=0;k<taskNum;k++){//k is the priority from highest to lowest
				for(int j=0;j<taskNum;j++){ //j is the taskNo
					if(myMapNodeP[offset].assignOrder[j]==i && myMapNodeP[offset].assignOrder[j] != -1 && myMapNodeP[offset].assignOrder[j] == k){
						fprintf(outputTXT," %c ",(char)(j+65));
					}
				}
			}
			fprintf(outputTXT,"\n");
		}

	}
	
	/*
			for(int i=0;i<taskNum;i++){ 
			searchTaskP = taskInfoP[i].preReqTask;
			if(*searchTaskP == '\0'){ //task X available without depend
				for(int j=0;j<engineNum;j++){
					if(engineInfoP[j].mem_size >= taskInfoP[i].reqMem){
						availInEngine = TRUE;
						myAvailTask[availIndex].availEngineNo[j] = 1;
					}
				}
				if(availInEngine == TRUE){
					myAvailTask[availIndex].availTaskNo = taskInfoP[i].taskNo;
					availIndex++;
					availInEngine = FALSE;
				}
			}
			else{
				while(*searchTaskP != '\0'){//check next dependList until '\0'
					for(int j=0;j<taskNum;j++){//check each task assigned which already finished
						//if multiple occur it may cause problem
						if(((int)*searchTaskP)-64 == currentNodeP->taskOrder[j]){
			//printf("we find taskNo. %d. its depend task is %c with taskNo. %d we got a finished one which is \n",i,*searchTaskP,((int)*searchTaskP)-64,currentNodeP->taskOrder[j]);							
							for(int k=0;k<engineNum;k++){
								if(engineInfoP[k].mem_size >= taskInfoP[i].reqMem){
									availInEngine = TRUE;
									myAvailTask[availIndex].availEngineNo[k] = 1;
								}
							}
							if(availInEngine == TRUE){
								myAvailTask[availIndex].availTaskNo = taskInfoP[i].taskNo;
								availIndex++;
								availInEngine = FALSE;
							}
						}
					}
					searchTaskP++;
				}
			}
		}

	*/
	
}

int InitialChecking(const engine *engineInfoP, const task *taskInfoP){
	int hasBreakPoint = FALSE;
	int pass = TRUE;
	int taskAssignable[MAXTASKNUM];
	
	for(int i=0;i<MAXTASKNUM;i++)
		taskAssignable[i] = -1;

	
	if(taskNum>MAXTASKNUM){
		pass = FALSE;
		printf("Can not assign tasks since task number overflow the MAXTASKNUM\n");
	}
	if(engineNum>MAXENGINENUM){
		pass = FALSE;
		printf("Can not assign tasks since engine number overflow the MAXENGINENUM\n");
	}

	for(int i=0;i<taskNum;i++){
		if(*(taskInfoP[i].preReqTask) == '\0'){
			hasBreakPoint = TRUE;
		}
	}
	if(hasBreakPoint != TRUE){
		pass = FALSE;
		printf("Can not assign tasks since circular dependcy situation occurs\n");
	}
	
	for(int i=0;i<taskNum;i++){
		for(int j=0;j<engineNum;j++){
			if(taskInfoP[i].reqMem <= engineInfoP[j].mem_size){
				taskAssignable[i] = TRUE;
			}
		}
	}
	for(int i=0;i<taskNum;i++){
		if (taskAssignable[i] == FALSE){
			pass = FALSE;
			printf("Can not assign tasks since we can not assign task %d to any engine\n",i);
		}
	}
	
	for (int i=0;i<engineNum;i++){
		if (engineInfoP[i].speed == 0){
			pass = FALSE;
			printf("Can not assign tasks since engine %s Speed is 0\n",engineInfoP[i].engineName);
		}
	}

	if(pass == TRUE)
		return TRUE;
	else
		return FALSE;
}




