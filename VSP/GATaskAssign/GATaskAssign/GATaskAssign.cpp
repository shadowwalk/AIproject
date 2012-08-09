#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#define MAXNODENUM 2000*50
#define MAXENGINENUM 5
#define MAXTASKNUM 15
#define TRUE 1
#define FALSE 0
FILE *engineStream;
FILE *taskStream;
FILE *outputStream;

#define POPULATIONSIZE 100
#define CROSSOVERPRO 100
#define MUTATIONPRO 100
#define MAXTRYINITIAL 2000
#define TEMINALGENERATION 100
#define ELITISM_MODE 0
#define ELITISM_MODE_ALT 1
#define NORMAL_MODE 1
#define OPTIMIZE_ET 0
#define OPTIMIZE_O 1
#define OPTIMIZE_NORMALIZED 2


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
int engineNum = 0,taskNum = 0;

typedef struct chromsome{
	 int gene[MAXTASKNUM];
	 float fitnessET;
	 int fitnessO;
	 float fitnessNormal;
	 int valid;
	 int rank;
}chromsome;
typedef struct population{
	chromsome *choromsomeP;
	int generationNum;
}population;

int InitialChecking(const engine *, const task *);
int RandomDice(int); //int random number from 0~range-1
float EvalFitness(const engine *, const task *, chromsome *);
int CheckValid(const engine *, const task *, chromsome *);
void Rank(population *,int);//0 for ET, 1 for O, 2 for normalized
void Selection(population *,population *,int);//0 for 10%,1 for best, 2 for none
void PrintPopulationInfo(population *);
void CrossOver(population *,int );//0 for 10%,1 for best, 2 for none
void Mutation(population *,int );//0 for 10%,1 for best, 2 for none
int  TerminalTest(population *, int);//0 for ET, 1 for O, 2 for normalized
void OutputResult(const engine*, const task *,chromsome *, int);//0 for ET, 1 for O, 2 for normalized
long GAET(const engine *, const task *);
long GAO(const engine *, const task *);
long GANORMAL(const engine *, const task *);


int generationCount;
int generationChangedCount;
int populationSize = 30;
int crossoverPro = 70;
int mutationPro = 40;
int fitnessParam;
int randSeedCount = 0;

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
int pass;
double runtimeGAET,runtimeGAO,runtimeGANORMAL;
long searchSpaceGAET,searchSpaceGAO,searchSpaceGANORMAL;
clock_t clock1,clock2;

//full path,command.exe,input#1,input#2,input#3
//for UNIX
/*
if (argc != 4){
	printf("\nPlease input valid command: engineFile taskFile outputFile\n");
	printf("For Example: GATaskAssign we1.dat wf1.dat output.dat\n");
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
	printf("For Example: GATaskAssign we1.dat wf1.dat output.dat\n");
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
	return -1;
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

	for (unsigned int i = 0; i< strlen(list1Head);i++){
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
	return -1;
}
else{
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

	for (unsigned int i = 0; i< strlen(list2Head);i++){
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
	printf("engine name is %s\n",myEngine[i].engineName);
	printf("engine No is %d\n",myEngine[i].engineNo);
	printf("engine speed is %d\n",myEngine[i].speed);
	printf("engine memory size is %d\n",myEngine[i].mem_size);
}
for(int i=0;i<taskNum;i++){
	printf("task name is %c\n",(myTask+i)->taskName);
	printf("task No is %d\n",myTask[i].taskNo);
	printf("task required I is %d\n",(myTask+i)->reqInstruction);
	printf("task mem Size is %d\n",myTask[i].reqMem);
	printf("task Output is %d\n",myTask[i].outputSize);
	//printf("dependNum %d = %d \n",i,dependNum[i]);
	printf("prereq task is %s\n",myTask[i].preReqTask);
}
*/

/*
	//check circular depend
	//check memoryReq & engineSpeed
	//check MAX NUM
	//check %50
*/
pass = InitialChecking(&myEngine[0],&myTask[0]);
if(pass != TRUE){
	return -1;
}

free(list2);
free(list1);
/*
printf("please input the population size\n");
scanf("%d",&populationSize);
if(populationSize > POPULATIONSIZE|| populationSize < 0){
	printf("please input num < %d",POPULATIONSIZE);
	return -1;
}
printf("please input the crossover probability\n");
scanf("%d",&crossoverPro);
if(crossoverPro > CROSSOVERPRO|| crossoverPro < 0){
	printf("please input num < %d",CROSSOVERPRO);
	return -1;
}
printf("please input the mutation probability\n");
scanf("%d",&mutationPro);
if(mutationPro > MUTATIONPRO || mutationPro < 0){
	printf("please input num < %d",MUTATIONPRO);
	return -1;
}
//printf("population size is %d\n",populationSize);
//printf("crossover probability is %d\n",crossoverPro);
//printf("mutation probability is %d\n",mutationPro);
*/

for(int i=0;i<taskNum;i++){
	fitnessParam += myTask[i].reqInstruction;
	fitnessParam += 30;
	fitnessParam += myTask[i].outputSize;
}

//--------------------start GA search------------------------------
clock1 = clock();
searchSpaceGAET = GAET(&myEngine[0],&myTask[0]);
clock2 = clock();
runtimeGAET=(double)(clock2 - clock1)/CLOCKS_PER_SEC ;

clock1 = clock();
searchSpaceGAO = GAO(&myEngine[0],&myTask[0]);
clock2 = clock();
runtimeGAO=(double)(clock2 - clock1)/CLOCKS_PER_SEC ;

clock1 = clock();
searchSpaceGANORMAL = GANORMAL(&myEngine[0],&myTask[0]);
clock2 = clock();
runtimeGANORMAL=(double)(clock2 - clock1)/CLOCKS_PER_SEC ;
//---------------------end GA search-------------------------------

printf("\n");
printf("--------------------Result Analysis---------------------\n");
printf("search space for GA based on ET is %ld\n",searchSpaceGAET);
printf("Runtime for GA based on ET is %f\n",runtimeGAET);
printf("--------------------------------------------------------\n");
printf("search space for GA based on O is %ld\n",searchSpaceGAO);
printf("Runtime for GA based on ET is %f\n",runtimeGAO);
printf("--------------------------------------------------------\n");
printf("search space for GANORMAL based on both ET and O is %ld\n",searchSpaceGANORMAL);
printf("Runtime for GA based on ET is %f\n",runtimeGANORMAL);
printf("--------------------------------------------------------\n");

if( (outputStream= fopen(outputFile, "w" )) == NULL )
{
	printf( "trying open'%s'\n",outputFile);
	printf( "open '%s' failure\n",outputFile );
	return -1;
}
else{ 
	printf("%s successfully opened\n",outputFile );
	fprintf(outputStream,"--------------------Result Analysis---------------------\n");
	fprintf(outputStream,"search space for GA based on ET is %ld\n",searchSpaceGAET);
	fprintf(outputStream,"Runtime for GA based on ET is %f\n",runtimeGAET);
	fprintf(outputStream,"--------------------------------------------------------\n");
	fprintf(outputStream,"search space for GA based on O is %ld\n",searchSpaceGAO);
	fprintf(outputStream,"Runtime for GA based on ET is %f\n",runtimeGAO);
	fprintf(outputStream,"--------------------------------------------------------\n");
	fprintf(outputStream,"search space for GANORMAL based on both ET and O is %ld\n",searchSpaceGANORMAL);
	fprintf(outputStream,"Runtime for GA based on ET is %f\n",runtimeGANORMAL);
	fprintf(outputStream,"--------------------------------------------------------\n");
}


/*
if (engineStream!=NULL){
	printf( "engineStream closed\n" );
	fclose(engineStream);
}
else
	printf("engineStream exist\n");
if (taskStream!=NULL){
	printf( "taskStream closed\n" );
	fclose(taskStream);
}
else
	printf("taskStream exist\n");
if (outputStream!=NULL){
	printf( "outputStream closed\n" );
	fclose(outputStream);
}
else
	printf("outputStream not exist\n");
*/	
//_getch();
}


long GAET(const engine *engineInfoP, const task *taskInfoP){

	int terminal = FALSE;
	chromsome * temp;
	float highestFit = 0;
	int	bestChromsome = -1;
	long searchSpace=0;
	int chromsomeEqual = FALSE;
	int bitEqual = TRUE;
	
	//Initialize population
	population parentGeneration;
	population childGerneration;
	chromsome parentChromsomeList[POPULATIONSIZE]; 
	chromsome childChromsomeList[POPULATIONSIZE]; 
	//let the parent and child pointer point to the right initial place
	parentGeneration.choromsomeP = &parentChromsomeList[0];
	childGerneration.choromsomeP = &childChromsomeList[0];
	parentGeneration.generationNum = -1;
	childGerneration.generationNum = 0; //indicate the next parentGeneration Num

	for(int i=0;i<POPULATIONSIZE;i++){
		for(int j=0;j<MAXTASKNUM;j++){
			parentChromsomeList[i].gene[j] = -1;
			childChromsomeList[i].gene[j] = -1;
		}
		parentChromsomeList[i].fitnessET = -1;
		parentChromsomeList[i].fitnessO = -1;
		parentChromsomeList[i].fitnessNormal = -1;
		parentChromsomeList[i].valid = -1;
		parentChromsomeList[i].rank = -1;
		childChromsomeList[i].fitnessET = -1;
		childChromsomeList[i].fitnessO = -1;
		childChromsomeList[i].fitnessNormal = -1;
		childChromsomeList[i].valid = -1;
		childChromsomeList[i].rank = -1;

	}
	//Generate First Adams & Eve
	for(int i=0;i<populationSize;i++){
		for(int j=0;j<taskNum;j++){
			parentChromsomeList[i].gene[j] = RandomDice(engineNum);
		}
	}
	
	for(int i=0;i<populationSize;i++){
		CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
	}
	for(int i=0;i<populationSize;i++){
		EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
	}
	parentGeneration.generationNum = 0;
	
	
	//start filter
	//make sure that we put at least one valid chromsome into the initial generation 
	int gotOne = FALSE;
	int count = 0;
	while(count < 1){
		gotOne = FALSE;
		count = 0;
		for(int i=0;i<populationSize;i++){
			for(int j=0;j<taskNum;j++){
				parentChromsomeList[i].gene[j] = RandomDice(engineNum);
			}
		}
		for(int i=0;i<populationSize;i++){
			CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
		}
		for(int i=0;i<populationSize;i++){
			EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
		}
		for(int i=0;i<populationSize;i++){
			if(parentChromsomeList[i].valid == TRUE){
				gotOne =TRUE;
				count ++;
			}
		}
		//printf("start filter valid count %d\n",count);
	}
	
	/*
	//another start filter
	//try to change every chromsome into valid one by a certain number of try
	//int gotOne = FALSE;
	//int count = 0;
	count = 0;
	int validCount = 0;
	for(int i=0;i<populationSize;i++){
		count = 0;
		while(parentChromsomeList[i].valid == FALSE && count < 1000){
			for(int j=0;j<taskNum;j++){
				parentChromsomeList[i].gene[j] = RandomDice(engineNum);
			}
			CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
			EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
			count++;
			if(parentChromsomeList[i].valid == TRUE){
				validCount++;
			}
		}
		printf("change time %d for chromsome %d\n",count,i);
	}
	printf("start filter valid count %d\n",validCount);
	*/

	//start the loop until satisfy the exit condition
	while(terminal == FALSE){
	
		//Let the offspring be born
		Rank(&parentGeneration,OPTIMIZE_ET);
		Selection(&parentGeneration,&childGerneration,ELITISM_MODE);

		childGerneration.generationNum ++;
	
		//PrintPopulationInfo(&childGerneration);

		//Let the Modification begin
		CrossOver(&childGerneration,ELITISM_MODE_ALT);
		Mutation(&childGerneration,ELITISM_MODE_ALT);
		//PrintPopulationInfo(&childGerneration);
		
		//after modify the child generation, evaluate them and give the fitness
		for(int i=0;i<populationSize;i++){
			CheckValid(engineInfoP,taskInfoP,(childGerneration.choromsomeP + i));
		}
	
		for(int i=0;i<populationSize;i++){
			EvalFitness(engineInfoP,taskInfoP,(childGerneration.choromsomeP + i));
		}
		//PrintPopulationInfo(&childGerneration);
		
		//calculate the new generated chromsome
		searchSpace += populationSize;
		for (int i=0;i<populationSize;i++){
			chromsomeEqual = FALSE;
			for(int j=0;j<populationSize;j++){
				bitEqual = TRUE;
				for(int k=0;k<taskNum;k++){
					if(childGerneration.choromsomeP[i].gene[k] != parentGeneration.choromsomeP[j].gene[k]){
						bitEqual = FALSE;
					}
				}
				if(bitEqual == TRUE){
					chromsomeEqual = TRUE;
				}
			}
			if(chromsomeEqual == TRUE){
				searchSpace --;
			}
		}
	
		//replace the whole parent generation with the child generation by changing the pointer and the generation No.
		temp = parentGeneration.choromsomeP;
		parentGeneration.choromsomeP = childGerneration.choromsomeP;
		parentGeneration.generationNum = childGerneration.generationNum;
		childGerneration.choromsomeP = temp;
		Rank(&parentGeneration,OPTIMIZE_ET);
		
		//pick the best chromsome for certain generation 
		highestFit = 0;
		for(int i=0;i<populationSize;i++){
			if(parentGeneration.choromsomeP[i].fitnessET > highestFit){
				//printf("current fitnessET %.2f current highestFit %.2f\n",parentGeneration.choromsomeP[i].fitnessET,highestFit);
				highestFit = parentGeneration.choromsomeP[i].fitnessET;
				bestChromsome = i;
			}
		}
		//PrintPopulationInfo(&parentGeneration);
		//printf("for generation %d best chromsome %d fitness is %.2f\n",parentGeneration.generationNum,bestChromsome,highestFit);
		
		//terminal test
		terminal = TerminalTest(&parentGeneration,OPTIMIZE_ET);
		if(terminal == TRUE){
			//if satisfy terminal condition, output the result to the correspond file
			OutputResult(engineInfoP,taskInfoP,(parentGeneration.choromsomeP + bestChromsome),OPTIMIZE_ET);
		}
	}
	
	//PrintPopulationInfo(&parentGeneration);
	printf("for generation %d best chromsome %d fitness is %.2f\n",parentGeneration.generationNum,bestChromsome,highestFit);

	/*
	printf("current Parent Generation Num info\n");
	for(int i=0;i<populationSize;i++){
		printf("Chromsome [%d]\n",i);
		printf("Task  : ");
		for(int j=0;j<taskNum;j++){
			printf("%c ",taskInfoP[j].taskName);
		}
		printf("\nEngine: ");
		for(int j=0;j<taskNum;j++){
			printf("%d ",parentChromsomeList[i].gene[j]);
		}
		printf("\n");
		printf("fitnessET : %.2f\n",parentChromsomeList[i].fitnessET);
		printf("fitnessO : %d\n",parentChromsomeList[i].fitnessO);
		printf("fitnessNormal : %.2f\n",parentChromsomeList[i].fitnessNormal);
		printf("rank : %d\n",parentChromsomeList[i].rank);
		printf("Valid : %d \n",parentChromsomeList[i].valid);
	}
	*/
	return searchSpace;
}

long GAO(const engine *engineInfoP, const task *taskInfoP){

	int terminal = FALSE;
	chromsome * temp;
	int highestFit = 0;
	int	bestChromsome = -1;
	long searchSpace=0;
	int chromsomeEqual = FALSE;
	int bitEqual = TRUE;
	
	//Initialize population
	population parentGeneration;
	population childGerneration;
	chromsome parentChromsomeList[POPULATIONSIZE]; 
	chromsome childChromsomeList[POPULATIONSIZE]; 
	//let the parent and child pointer point to the right initial place
	parentGeneration.choromsomeP = &parentChromsomeList[0];
	childGerneration.choromsomeP = &childChromsomeList[0];
	parentGeneration.generationNum = -1;
	childGerneration.generationNum = 0; //indicate the next parentGeneration Num

	for(int i=0;i<POPULATIONSIZE;i++){
		for(int j=0;j<MAXTASKNUM;j++){
			parentChromsomeList[i].gene[j] = -1;
			childChromsomeList[i].gene[j] = -1;
		}
		parentChromsomeList[i].fitnessET = -1;
		parentChromsomeList[i].fitnessO = -1;
		parentChromsomeList[i].fitnessNormal = -1;
		parentChromsomeList[i].valid = -1;
		parentChromsomeList[i].rank = -1;
		childChromsomeList[i].fitnessET = -1;
		childChromsomeList[i].fitnessO = -1;
		childChromsomeList[i].fitnessNormal = -1;
		childChromsomeList[i].valid = -1;
		childChromsomeList[i].rank = -1;

	}
	//Generate First Adams & Eve
	for(int i=0;i<populationSize;i++){
		for(int j=0;j<taskNum;j++){
			parentChromsomeList[i].gene[j] = RandomDice(engineNum);
		}
	}
	
	for(int i=0;i<populationSize;i++){
		CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
	}
	for(int i=0;i<populationSize;i++){
		EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
	}
	parentGeneration.generationNum = 0;
	
	
	//start filter
	//make sure that we put at least one valid chromsome into the initial generation 
	int gotOne = FALSE;
	int count = 0;
	while(count < 1){
		gotOne = FALSE;
		count = 0;
		for(int i=0;i<populationSize;i++){
			for(int j=0;j<taskNum;j++){
				parentChromsomeList[i].gene[j] = RandomDice(engineNum);
			}
		}
		for(int i=0;i<populationSize;i++){
			CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
		}
		for(int i=0;i<populationSize;i++){
			EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
		}
		for(int i=0;i<populationSize;i++){
			if(parentChromsomeList[i].valid == TRUE){
				gotOne =TRUE;
				count ++;
			}
		}
		//printf("start filter valid count %d\n",count);
	}
	
	/*
	//another start filter
	//try to change every chromsome into valid one by a certain number of try
	//int gotOne = FALSE;
	//int count = 0;
	count = 0;
	int validCount = 0;
	for(int i=0;i<populationSize;i++){
		count = 0;
		while(parentChromsomeList[i].valid == FALSE && count < 1000){
			for(int j=0;j<taskNum;j++){
				parentChromsomeList[i].gene[j] = RandomDice(engineNum);
			}
			CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
			EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
			count++;
			if(parentChromsomeList[i].valid == TRUE){
				validCount++;
			}
		}
		printf("change time %d for chromsome %d\n",count,i);
	}
	printf("start filter valid count %d\n",validCount);
	*/

	//start the loop until satisfy the exit condition
	while(terminal == FALSE){
	
		//Let the offspring be born
		Rank(&parentGeneration,OPTIMIZE_O);
		Selection(&parentGeneration,&childGerneration,ELITISM_MODE);

		childGerneration.generationNum ++;
	
		//PrintPopulationInfo(&childGerneration);

		//Let the Modification begin
		CrossOver(&childGerneration,ELITISM_MODE_ALT);
		Mutation(&childGerneration,ELITISM_MODE_ALT);
		//PrintPopulationInfo(&childGerneration);
		
		//after modify the child generation, evaluate them and give the fitness
		for(int i=0;i<populationSize;i++){
			CheckValid(engineInfoP,taskInfoP,(childGerneration.choromsomeP + i));
		}
	
		for(int i=0;i<populationSize;i++){
			EvalFitness(engineInfoP,taskInfoP,(childGerneration.choromsomeP + i));
		}
		//PrintPopulationInfo(&childGerneration);
		
		//calculate the new generated chromsome
		searchSpace += populationSize;
		for (int i=0;i<populationSize;i++){
			chromsomeEqual = FALSE;
			for(int j=0;j<populationSize;j++){
				bitEqual = TRUE;
				for(int k=0;k<taskNum;k++){
					if(childGerneration.choromsomeP[i].gene[k] != parentGeneration.choromsomeP[j].gene[k]){
						bitEqual = FALSE;
					}
				}
				if(bitEqual == TRUE){
					chromsomeEqual = TRUE;
				}
			}
			if(chromsomeEqual == TRUE){
				searchSpace --;
			}
		}

	
		//replace the whole parent generation with the child generation by changing their pointer and the generation No.
		temp = parentGeneration.choromsomeP;
		parentGeneration.choromsomeP = childGerneration.choromsomeP;
		parentGeneration.generationNum = childGerneration.generationNum;
		childGerneration.choromsomeP = temp;
		Rank(&parentGeneration,OPTIMIZE_O);
		
		//pick the best chromsome for certain generation 
		highestFit = 0;
		for(int i=0;i<populationSize;i++){
			if(parentGeneration.choromsomeP[i].fitnessO > highestFit){
				//printf("current fitnessET %.2f current highestFit %.2f\n",parentGeneration.choromsomeP[i].fitnessET,highestFit);
				highestFit = parentGeneration.choromsomeP[i].fitnessO;
				bestChromsome = i;
			}
		}
		//PrintPopulationInfo(&parentGeneration);
		//printf("for generation %d best chromsome %d fitness is %d\n",parentGeneration.generationNum,bestChromsome,highestFit);
		
		//terminal test
		terminal = TerminalTest(&parentGeneration,OPTIMIZE_O);
		if(terminal == TRUE){
			//if satisfy terminal condition, output the result to the correspond file
			OutputResult(engineInfoP,taskInfoP,(parentGeneration.choromsomeP + bestChromsome),OPTIMIZE_O);
		}
	}
	
	//PrintPopulationInfo(&parentGeneration);
	printf("for generation %d best chromsome %d fitness is %d\n",parentGeneration.generationNum,bestChromsome,highestFit);

	/*
	printf("current Parent Generation Num info\n");
	for(int i=0;i<populationSize;i++){
		printf("Chromsome [%d]\n",i);
		printf("Task  : ");
		for(int j=0;j<taskNum;j++){
			printf("%c ",taskInfoP[j].taskName);
		}
		printf("\nEngine: ");
		for(int j=0;j<taskNum;j++){
			printf("%d ",parentChromsomeList[i].gene[j]);
		}
		printf("\n");
		printf("fitnessET : %.2f\n",parentChromsomeList[i].fitnessET);
		printf("fitnessO : %d\n",parentChromsomeList[i].fitnessO);
		printf("fitnessNormal : %.2f\n",parentChromsomeList[i].fitnessNormal);
		printf("rank : %d\n",parentChromsomeList[i].rank);
		printf("Valid : %d \n",parentChromsomeList[i].valid);
	}
	*/
	return searchSpace;
}

long GANORMAL(const engine *engineInfoP, const task *taskInfoP){

	int terminal = FALSE;
	chromsome * temp;
	float highestFit = 0;
	int	bestChromsome = -1;
	long searchSpace=0;
	int chromsomeEqual = FALSE;
	int bitEqual = TRUE;

	//Initialize population
	population parentGeneration;
	population childGerneration;
	chromsome parentChromsomeList[POPULATIONSIZE]; 
	chromsome childChromsomeList[POPULATIONSIZE]; 
	//let the parent and child pointer point to the right initial place
	parentGeneration.choromsomeP = &parentChromsomeList[0];
	childGerneration.choromsomeP = &childChromsomeList[0];
	parentGeneration.generationNum = -1;
	childGerneration.generationNum = 0; //indicate the next parentGeneration Num

	for(int i=0;i<POPULATIONSIZE;i++){
		for(int j=0;j<MAXTASKNUM;j++){
			parentChromsomeList[i].gene[j] = -1;
			childChromsomeList[i].gene[j] = -1;
		}
		parentChromsomeList[i].fitnessET = -1;
		parentChromsomeList[i].fitnessO = -1;
		parentChromsomeList[i].fitnessNormal = -1;
		parentChromsomeList[i].valid = -1;
		parentChromsomeList[i].rank = -1;
		childChromsomeList[i].fitnessET = -1;
		childChromsomeList[i].fitnessO = -1;
		childChromsomeList[i].fitnessNormal = -1;
		childChromsomeList[i].valid = -1;
		childChromsomeList[i].rank = -1;

	}
	//Generate First Adams & Eve
	for(int i=0;i<populationSize;i++){
		for(int j=0;j<taskNum;j++){
			parentChromsomeList[i].gene[j] = RandomDice(engineNum);
		}
	}
	
	for(int i=0;i<populationSize;i++){
		CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
	}
	for(int i=0;i<populationSize;i++){
		EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
	}
	parentGeneration.generationNum = 0;
	
	
	//start filter
	//make sure that we put at least one valid chromsome into the initial generation 
	int gotOne = FALSE;
	int count = 0;
	while(count < 1){
		gotOne = FALSE;
		count = 0;
		for(int i=0;i<populationSize;i++){
			for(int j=0;j<taskNum;j++){
				parentChromsomeList[i].gene[j] = RandomDice(engineNum);
			}
		}
		for(int i=0;i<populationSize;i++){
			CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
		}
		for(int i=0;i<populationSize;i++){
			EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
		}
		for(int i=0;i<populationSize;i++){
			if(parentChromsomeList[i].valid == TRUE){
				gotOne =TRUE;
				count ++;
			}
		}
		//printf("start filter valid count %d\n",count);
	}
	
	/*
	//another start filter
	//try to change every chromsome into valid one by a certain number of try
	//int gotOne = FALSE;
	//int count = 0;
	count = 0;
	int validCount = 0;
	for(int i=0;i<populationSize;i++){
		count = 0;
		while(parentChromsomeList[i].valid == FALSE && count < 1000){
			for(int j=0;j<taskNum;j++){
				parentChromsomeList[i].gene[j] = RandomDice(engineNum);
			}
			CheckValid(engineInfoP,taskInfoP,&parentChromsomeList[i]);
			EvalFitness(engineInfoP,taskInfoP,&parentChromsomeList[i]);
			count++;
			if(parentChromsomeList[i].valid == TRUE){
				validCount++;
			}
		}
		printf("change time %d for chromsome %d\n",count,i);
	}
	printf("start filter valid count %d\n",validCount);
	*/

	//start the loop until satisfy the exit condition
	while(terminal == FALSE){
	
		//Let the offspring be born
		Rank(&parentGeneration,OPTIMIZE_NORMALIZED);
		Selection(&parentGeneration,&childGerneration,ELITISM_MODE);

		childGerneration.generationNum ++;
	
		//PrintPopulationInfo(&childGerneration);

		//Let the Modification begin
		CrossOver(&childGerneration,ELITISM_MODE_ALT);
		Mutation(&childGerneration,ELITISM_MODE_ALT);
		//PrintPopulationInfo(&childGerneration);
		
		//after modify the child generation, evaluate them and give the fitness
		for(int i=0;i<populationSize;i++){
			CheckValid(engineInfoP,taskInfoP,(childGerneration.choromsomeP + i));
		}
	
		for(int i=0;i<populationSize;i++){
			EvalFitness(engineInfoP,taskInfoP,(childGerneration.choromsomeP + i));
		}
		//PrintPopulationInfo(&childGerneration);
		
		//calculate the new generated chromsome
		searchSpace += populationSize;
		for (int i=0;i<populationSize;i++){
			chromsomeEqual = FALSE;
			for(int j=0;j<populationSize;j++){
				bitEqual = TRUE;
				for(int k=0;k<taskNum;k++){
					if(childGerneration.choromsomeP[i].gene[k] != parentGeneration.choromsomeP[j].gene[k]){
						bitEqual = FALSE;
					}
				}
				if(bitEqual == TRUE){
					chromsomeEqual = TRUE;
				}
			}
			if(chromsomeEqual == TRUE){
				searchSpace --;
			}
		}
	

		//replace the whole parent generation with the child generation by changing their pointer and the generation No.
		temp = parentGeneration.choromsomeP;
		parentGeneration.choromsomeP = childGerneration.choromsomeP;
		parentGeneration.generationNum = childGerneration.generationNum;
		childGerneration.choromsomeP = temp;
		Rank(&parentGeneration,OPTIMIZE_NORMALIZED);
		
		//pick the best chromsome for certain generation 
		highestFit = 0;
		for(int i=0;i<populationSize;i++){
			if(parentGeneration.choromsomeP[i].fitnessNormal > highestFit){
				//printf("current fitnessET %.2f current highestFit %.2f\n",parentGeneration.choromsomeP[i].fitnessET,highestFit);
				highestFit = parentGeneration.choromsomeP[i].fitnessNormal;
				bestChromsome = i;
			}
		}
		//PrintPopulationInfo(&parentGeneration);
		//printf("for generation %d best chromsome %d fitness is %.2f\n",parentGeneration.generationNum,bestChromsome,highestFit);
		
		//terminal test
		terminal = TerminalTest(&parentGeneration,OPTIMIZE_NORMALIZED);
		if(terminal == TRUE){
			//if satisfy terminal condition, output the result to the correspond file
			OutputResult(engineInfoP,taskInfoP,(parentGeneration.choromsomeP + bestChromsome),OPTIMIZE_NORMALIZED);
		}
	}
	
	//PrintPopulationInfo(&parentGeneration);
	printf("for generation %d best chromsome %d fitness is %.2f\n",parentGeneration.generationNum,bestChromsome,highestFit);

	/*
	printf("current Parent Generation Num info\n");
	for(int i=0;i<populationSize;i++){
		printf("Chromsome [%d]\n",i);
		printf("Task  : ");
		for(int j=0;j<taskNum;j++){
			printf("%c ",taskInfoP[j].taskName);
		}
		printf("\nEngine: ");
		for(int j=0;j<taskNum;j++){
			printf("%d ",parentChromsomeList[i].gene[j]);
		}
		printf("\n");
		printf("fitnessET : %.2f\n",parentChromsomeList[i].fitnessET);
		printf("fitnessO : %d\n",parentChromsomeList[i].fitnessO);
		printf("fitnessNormal : %.2f\n",parentChromsomeList[i].fitnessNormal);
		printf("rank : %d\n",parentChromsomeList[i].rank);
		printf("Valid : %d \n",parentChromsomeList[i].valid);
	}
	*/
	return searchSpace;
}


/*
	give the rank value to each chromsome of the population
	type 0: rank based on ET fitness
	type 1: rank based on O fitness
	type 2: rank based on both ET and O normalized fitness

*/
void Rank(population *populationInfoP,int type){
	int lessCount[POPULATIONSIZE];
	for(int i=0;i<POPULATIONSIZE;i++){
		lessCount[i] = -1;
		if(i<populationSize){
			lessCount[i] = 0;
		}
	}
	if(type == OPTIMIZE_ET){
		for(int i=0;i<populationSize;i++){
			for(int j=0;j<populationSize;j++){
				if(populationInfoP->choromsomeP[i].fitnessET > populationInfoP->choromsomeP[j].fitnessET){
					lessCount[i] ++;
				}
			}
		}
		for(int i=0;i<populationSize;i++){
			populationInfoP->choromsomeP[i].rank = lessCount[i] + 1;
		}
	}
	else if(type == OPTIMIZE_O){
		for(int i=0;i<populationSize;i++){
			for(int j=0;j<populationSize;j++){
				if(populationInfoP->choromsomeP[i].fitnessO > populationInfoP->choromsomeP[j].fitnessO){
					lessCount[i] ++;
				}
			}
		}
		for(int i=0;i<populationSize;i++){
			populationInfoP->choromsomeP[i].rank = lessCount[i] + 1;
		}
	}
	else if(type == OPTIMIZE_NORMALIZED){
		for(int i=0;i<populationSize;i++){
			for(int j=0;j<populationSize;j++){
				if(populationInfoP->choromsomeP[i].fitnessNormal > populationInfoP->choromsomeP[j].fitnessNormal){
					lessCount[i] ++;
				}
			}
		}
		for(int i=0;i<populationSize;i++){
			populationInfoP->choromsomeP[i].rank = lessCount[i] + 1;
		}
	}
}

/*
	generate the child population based on the given parent population using Rank value
	type 0: using Elitism method that first put 10% best chromsome directly to the child generation, then Roulette Wheel select based on rank
	type 1: using Elitism method that first put one best chromsome directly to the child generation, then Roulette Wheel select based on rank
	type 2: Roulette Wheel select based on rank
	
*/

void Selection(population *parentInfoP,population *childInfoP,int type){
	int rankSum = 0;
	int currentRankSum = 0,rankPointer = -1;
	//used for elitism
	int sortedRankValue[POPULATIONSIZE],rankKey[POPULATIONSIZE];
	int rankPos = 0;
	int highestRank = 0,bestChromsome=-1;
	int haveSame = FALSE;
	int transferCounter = 0;
	int dice = -1;
	for (int i=0;i<POPULATIONSIZE;i++){
		sortedRankValue[i] = 0;
		rankKey[i] = -1;
	}

	//Elitism mode
	if(type == ELITISM_MODE){
		//Elitism create a sorted array to store the rank value and choose from it
		//pick the largest one to initialize
		for(int i=0;i<populationSize;i++){
			if(parentInfoP->choromsomeP[i].rank > highestRank){
				highestRank = parentInfoP->choromsomeP[i].rank;
				bestChromsome = i;
			}
		}
		sortedRankValue[rankPos] = parentInfoP->choromsomeP[bestChromsome].rank;
		rankKey[rankPos] = bestChromsome;

		while(rankPos != (populationSize-1)){
			highestRank = 0;
			bestChromsome = -1;
			//pick the largest one which hasn't been picked
			for(int i=0;i<populationSize;i++){
				if(parentInfoP->choromsomeP[i].rank > highestRank && parentInfoP->choromsomeP[i].rank <= sortedRankValue[rankPos]){
					//if find some less or equal one check whether we already have it first	
					haveSame = FALSE;
					for(int j=0;j<=rankPos;j++){
						if(i == rankKey[j]){
							haveSame =TRUE;
						}
					}
					if(haveSame != TRUE){
						highestRank = parentInfoP->choromsomeP[i].rank;
						bestChromsome = i;
					}
				}
			}
			//put into array
			rankPos ++;
			sortedRankValue[rankPos] = parentInfoP->choromsomeP[bestChromsome].rank;
			rankKey[rankPos] = bestChromsome;
		}
		/*
			printf("rankValue:");
			for(int i=0;i<populationSize;i++){
				printf("%3d ",sortedRankValue[i]);
			}
			printf("\nrankKey  :");
			for(int i=0;i<populationSize;i++){
				printf("%3d ",rankKey[i]);
			}
			printf("\n");
		*/
		//copy the 10% to child 
		for(int i=0;i<populationSize/10;i++){	
			for(int j=0;j<taskNum;j++){
				childInfoP->choromsomeP[i].gene[j] = parentInfoP->choromsomeP[rankKey[transferCounter]].gene[j];
			}		
			transferCounter ++;		//point to where we should put content next
		}

		//Roulette Wheel
		for(int i=0;i<populationSize;i++){
			rankSum += parentInfoP->choromsomeP[i].rank;
		}
		while(transferCounter < populationSize){
			
			currentRankSum = 0;
			rankPointer = -1;
			dice = RandomDice(rankSum+1);
			if(dice == 0){
				rankPointer = 0;
			}
			while(currentRankSum < dice){
				currentRankSum += parentInfoP->choromsomeP[++rankPointer].rank;			
			}
			
			//printf("child[%d] the dice is %d and we stop at %d which get currentRankSum %d\n",transferCounter,dice,rankPointer,currentRankSum);
			//printf("stop at %3d \n",rankPointer);
			//rankPointer = RandomDice(populationSize);
			for(int j=0;j<taskNum;j++){
				childInfoP->choromsomeP[transferCounter].gene[j] = parentInfoP->choromsomeP[rankPointer].gene[j];
			}
			transferCounter ++;
		}
	}
	else if(type == ELITISM_MODE_ALT){
		//Elitism create a sorted array to store the rank value and choose from it
		//pick the largest one to initialize
		for(int i=0;i<populationSize;i++){
			if(parentInfoP->choromsomeP[i].rank > highestRank){
				highestRank = parentInfoP->choromsomeP[i].rank;
				bestChromsome = i;
			}
		}

		//copy the best to child 
		for(int j=0;j<taskNum;j++){
			childInfoP->choromsomeP[0].gene[j] = parentInfoP->choromsomeP[bestChromsome].gene[j];
		}		
		transferCounter ++;		//point to where we should put content next

		//Roulette Wheel
		for(int i=0;i<populationSize;i++){
			rankSum += parentInfoP->choromsomeP[i].rank;
		}
		while(transferCounter < populationSize){
			
			currentRankSum = 0;
			rankPointer = -1;
			dice = RandomDice(rankSum+1);
			if(dice == 0){
				rankPointer = 0;
			}
			while(currentRankSum < dice){
				currentRankSum += parentInfoP->choromsomeP[++rankPointer].rank;			
			}
			
			//printf("child[%d] the dice is %d and we stop at %d which get currentRankSum %d\n",transferCounter,dice,rankPointer,currentRankSum);
			//printf("stop at %3d \n",rankPointer);
			//rankPointer = RandomDice(populationSize);
			for(int j=0;j<taskNum;j++){
				childInfoP->choromsomeP[transferCounter].gene[j] = parentInfoP->choromsomeP[rankPointer].gene[j];
			}
			transferCounter ++;
		}
	}
	else if(type == NORMAL_MODE){

		//Roulette Wheel
		for(int i=0;i<populationSize;i++){
			rankSum += parentInfoP->choromsomeP[i].rank;
		}
		while(transferCounter < populationSize){
			currentRankSum = 0;
			rankPointer = -1;
			dice = RandomDice(rankSum+1);
			
			if(dice == 0){
				rankPointer = 0;
			}
			while(currentRankSum < dice){
				currentRankSum += parentInfoP->choromsomeP[++rankPointer].rank;			
			}
			
			//printf("child[%d] the dice is %d and we stop at %d which get currentRankSum %d\n",transferCounter,dice,rankPointer,currentRankSum);
			//printf("stop at %3d \n",rankPointer);
			//rankPointer = RandomDice(populationSize);
			for(int j=0;j<taskNum;j++){
				childInfoP->choromsomeP[transferCounter].gene[j] = parentInfoP->choromsomeP[rankPointer].gene[j];
			}
			transferCounter ++;
		}
	}

	//PrintPopulationInfo(parentInfoP);
	//PrintPopulationInfo(childInfoP);
}

/*
	make corssover for current generation, 
	type 0: keep first 10% chromsome exempt of corssover operation 
	type 1: keep the first which is the best chromsome exempt of corssover operation
	type 2: corssover operation apply to all chromsome
*/

void CrossOver(population *populationInfoP,int type){
	int cutPoint;
	int crossOverDice;
	int temp;
	
	if(type == ELITISM_MODE){
		for(int i=populationSize/10;i<populationSize-1;i+=2){
		//for(int i=populationSize/10;i<populationSize-1;i+=2){
			crossOverDice = RandomDice(101);
			if(crossOverDice <= crossoverPro){
				//make cross over
				cutPoint = RandomDice(taskNum-1);
				//printf("chromsome %d cross point at %d\n",i,cutPoint);
				for(int j=0;j<taskNum;j++){
					if(j<=cutPoint){
						temp = populationInfoP->choromsomeP[i].gene[j]; 
						populationInfoP->choromsomeP[i].gene[j] = populationInfoP->choromsomeP[i+1].gene[j];
						populationInfoP->choromsomeP[i+1].gene[j] = temp;
					}
				}
			}
		}
	}
	else if(type == ELITISM_MODE_ALT){
		for(int i=1;i<populationSize-1;i+=2){
		//for(int i=populationSize/10;i<populationSize-1;i+=2){
			crossOverDice = RandomDice(101);
			if(crossOverDice <= crossoverPro){
				//make cross over
				cutPoint = RandomDice(taskNum-1);
				//printf("chromsome %d cross point at %d\n",i,cutPoint);
				for(int j=0;j<taskNum;j++){
					if(j<=cutPoint){
						temp = populationInfoP->choromsomeP[i].gene[j]; 
						populationInfoP->choromsomeP[i].gene[j] = populationInfoP->choromsomeP[i+1].gene[j];
						populationInfoP->choromsomeP[i+1].gene[j] = temp;
					}
				}
			}
		}
	}
	else if(type == NORMAL_MODE){
		for(int i=0;i<populationSize-1;i+=2){
		//for(int i=populationSize/10;i<populationSize-1;i+=2){
			crossOverDice = RandomDice(101);
			if(crossOverDice <= crossoverPro){
				//make cross over
				cutPoint = RandomDice(taskNum-1);
				//printf("chromsome %d cross point at %d\n",i,cutPoint);
				for(int j=0;j<taskNum;j++){
					if(j<=cutPoint){
						temp = populationInfoP->choromsomeP[i].gene[j]; 
						populationInfoP->choromsomeP[i].gene[j] = populationInfoP->choromsomeP[i+1].gene[j];
						populationInfoP->choromsomeP[i+1].gene[j] = temp;
					}
				}
			}
		}
	}
}

/*
	make mutation for current generation, 
	type 0: keep first 10% chromsome exempt of mutation operation
	type 1: keep the first which is the best chromsome exempt of mutation operation
	type 2: mutation operation apply to all chromsome
*/
void Mutation(population *populationInfoP,int type){
	int mutationPoint;
	int mutationDice;
	int mutateTo;
	int reRollCount;
	
	if(type == ELITISM_MODE){
		//PrintPopulationInfo(populationInfoP);
		for(int i=populationSize/10;i<populationSize-1;i+=2){
		//for(int i=populationSize/10;i<populationSize;i++){
			mutationDice = RandomDice(101);
			if(mutationDice <= mutationPro){
				//make mutation
				for(int j=0;j<taskNum/2;j++){
					mutationPoint = RandomDice(taskNum);
					mutateTo = RandomDice(engineNum);
					reRollCount = 0;
					while( populationInfoP->choromsomeP[i].gene[mutationPoint] == mutateTo){
						mutateTo = RandomDice(engineNum);
						//printf("reRoll dice to %d\n",mutateTo);
						reRollCount++;
					}
					//printf("chromsome %d gene %d mutate from %d to %d\n",i,mutationPoint,populationInfoP->choromsomeP[i].gene[mutationPoint],mutateTo);
					populationInfoP->choromsomeP[i].gene[mutationPoint] = mutateTo;
				}
			}
		}
		//PrintPopulationInfo(populationInfoP);
	}
	else if(type == ELITISM_MODE_ALT){
		//PrintPopulationInfo(populationInfoP);
		for(int i=1;i<populationSize-1;i+=2){
		//for(int i=populationSize/10;i<populationSize;i++){
			mutationDice = RandomDice(101);
			if(mutationDice <= mutationPro){
				//make mutation
				for(int j=0;j<taskNum/2;j++){
					mutationPoint = RandomDice(taskNum);
					mutateTo = RandomDice(engineNum);
					reRollCount = 0;
					while( populationInfoP->choromsomeP[i].gene[mutationPoint] == mutateTo){
						mutateTo = RandomDice(engineNum);
						//printf("reRoll dice to %d\n",mutateTo);
						reRollCount++;
					}
					//printf("chromsome %d gene %d mutate from %d to %d\n",i,mutationPoint,populationInfoP->choromsomeP[i].gene[mutationPoint],mutateTo);
					populationInfoP->choromsomeP[i].gene[mutationPoint] = mutateTo;
				}
			}
		}
		//PrintPopulationInfo(populationInfoP);
	}
	else if(type == NORMAL_MODE){
		//PrintPopulationInfo(populationInfoP);
		for(int i=0;i<populationSize-1;i+=2){
		//for(int i=populationSize/10;i<populationSize;i++){
			mutationDice = RandomDice(101);
			if(mutationDice <= mutationPro){
				//make mutation
				for(int j=0;j<taskNum/2;j++){
					mutationPoint = RandomDice(taskNum);
					mutateTo = RandomDice(engineNum);
					reRollCount = 0;
					while( populationInfoP->choromsomeP[i].gene[mutationPoint] == mutateTo){
						mutateTo = RandomDice(engineNum);
						//printf("reRoll dice to %d\n",mutateTo);
						reRollCount++;
					}
					//printf("chromsome %d gene %d mutate from %d to %d\n",i,mutationPoint,populationInfoP->choromsomeP[i].gene[mutationPoint],mutateTo);
					populationInfoP->choromsomeP[i].gene[mutationPoint] = mutateTo;
				}
			}
		}
		//PrintPopulationInfo(populationInfoP);
	}
}

/*
	Check wether this chromsome is valid or not, put the value in to its structure
*/	
int CheckValid(const engine *engineInfoP, const task *taskInfoP, chromsome *chromsomeInfoP){
	int assignCount[MAXENGINENUM];
	int valid = TRUE;

	for(int i=0;i<MAXENGINENUM;i++){
		assignCount[i] = -1;
		if(i<engineNum){
			assignCount[i] = 0;
		}
	}

	for(int i=0;i<taskNum;i++){
		assignCount[chromsomeInfoP->gene[i]] ++;
		if(engineInfoP[chromsomeInfoP->gene[i]].mem_size < taskInfoP[i].reqMem){
			valid = FALSE;
			//printf("invalid since engine %d mem is %d < task %c reqMem is %d\n",chromsomeInfoP->gene[i]
			//,engineInfoP[chromsomeInfoP->gene[i]].mem_size,taskInfoP[i].taskName,taskInfoP[i].reqMem);
		}
	}
	for(int i=0;i<engineNum;i++){
		if (assignCount[i] > taskNum/2){
			valid = FALSE;
			//printf("invalid since engine %d has assign %d tasks which exceed the limit %d\n",i,assignCount[i],taskNum/2);
		}
	}
	chromsomeInfoP->valid = valid;
	return valid;
}

/*
	Evaluate the fitness for a single chromsome, put the value in to its structure
*/
float EvalFitness(const engine *engineInfoP, const task *taskInfoP, chromsome *chromsomeInfoP){
	float engineET[MAXENGINENUM];
	float highestET=0;
	float totalTransitionCost=0;
	int totalCostO = 0;
	char *dependTaskP;
	int selectedEngine = -1;
	
	for(int i=0;i<MAXENGINENUM;i++){
		engineET[i] = -1;
		if(i<engineNum){
			engineET[i] = 0;
		}
	}

	//check valid first
	if(chromsomeInfoP->valid == FALSE){
		chromsomeInfoP->fitnessET = 0;
		chromsomeInfoP->fitnessO = 0;
		chromsomeInfoP->fitnessNormal = 0;
		return -1;
	}
	
	//Calculate ET for switching part
	for(int i=0;i<taskNum;i++){ 
		if(chromsomeInfoP->gene[i] != -1){
			engineET[chromsomeInfoP->gene[i]] += (float)(taskInfoP[i].reqInstruction)/(float)(engineInfoP[chromsomeInfoP->gene[i]].speed);
			engineET[chromsomeInfoP->gene[i]] += (float)10/(float)(engineInfoP[chromsomeInfoP->gene[i]].speed);
		}
	}
	for(int i=0;i<engineNum;i++){
			if(engineET[i] > highestET){
				highestET = engineET[i];
				selectedEngine = i;
			}
	}
	chromsomeInfoP->fitnessET = highestET;
	
	//Calculate O
	for(int i=0;i<taskNum;i++){ 
		if(chromsomeInfoP->gene[i] != -1){
			dependTaskP = taskInfoP[i].preReqTask;
			while(*dependTaskP != '\0'){
				//check all assigned tasks check whether this depend is assigned to different one
				for(int j=0;j<taskNum;j++){
					if(taskInfoP[j].taskName == *dependTaskP && chromsomeInfoP->gene[j] != -1){
						if(chromsomeInfoP->gene[i] != chromsomeInfoP->gene[j]){
							totalCostO += taskInfoP[j].outputSize;
						}
					}
				}
				dependTaskP++;
			}
		}
	}
	chromsomeInfoP->fitnessO = fitnessParam - totalCostO;

	//Calculate ET for transition part
	for(int i=0;i<taskNum;i++){
		dependTaskP = taskInfoP[i].preReqTask;
		if(*dependTaskP != '\0'){
			while(*dependTaskP != '\0'){
				for(int j=0;j<taskNum;j++){
					if(taskInfoP[j].taskName == *dependTaskP){
						//outgoing arrow
						if(chromsomeInfoP->gene[j] == selectedEngine && chromsomeInfoP->gene[i] != selectedEngine){
							totalTransitionCost += (float)20/(float)(engineInfoP[chromsomeInfoP->gene[j]].speed);
						}
						//incoming arrow
						if(chromsomeInfoP->gene[j] != selectedEngine && chromsomeInfoP->gene[i] == selectedEngine){
							totalTransitionCost += (float)20/(float)(engineInfoP[chromsomeInfoP->gene[j]].speed);
						}
					}
				}
				dependTaskP++;
			}
		}
	}	
	chromsomeInfoP->fitnessET = fitnessParam - (totalTransitionCost + highestET);
	
	chromsomeInfoP->fitnessNormal = fitnessParam - ((highestET + totalTransitionCost)*(float)0.7 + (float)totalCostO*(float)0.3);

	return totalTransitionCost;
}

/*
	Test whether we should terminate in this generation  using return value
	generally the return condition should be after the generation number
	type 0: optimize for ET
	type 1: optimize for O
	type 2: optimize for both ET and O
*/
int  TerminalTest(population *populationInfoP, int type){
	int terminal = FALSE;

	if(type == OPTIMIZE_ET){
		if(populationInfoP->generationNum > TEMINALGENERATION){
			terminal = TRUE;
		}
	}
	else if(type == OPTIMIZE_O ){
		if(populationInfoP->generationNum > TEMINALGENERATION){
			terminal = TRUE;
		}
	}
	else if(type == OPTIMIZE_NORMALIZED){
		if(populationInfoP->generationNum > TEMINALGENERATION){
			terminal = TRUE;
		}
	}

	if(terminal == TRUE){
		return TRUE;
	}
	else{
		return FALSE;
	}
}


int InitialChecking(const engine *engineInfoP, const task *taskInfoP){
	int hasBreakPoint = FALSE;
	int pass = TRUE;
	int taskAssignable[MAXTASKNUM];
	int engineCapacity[MAXENGINENUM];
	int taskChosen[MAXTASKNUM];
	int emptyEngine = FALSE, assignFail = FALSE;
	int currentHighestTask,currentHighestEngine;
	int pickTaskNo,pickEngineNo;
	int taskCount=0;


	for(int i=0;i<MAXENGINENUM;i++){
		engineCapacity[i] = -1;
		if(i<engineNum){
			engineCapacity[i] = taskNum/2;
		}
	}
	for(int i=0;i<MAXTASKNUM;i++){
		taskAssignable[i] = FALSE;
		taskChosen[i] = -1;
		if(i<taskNum){
			taskChosen[i] = FALSE;
		}
	}
	
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
				//printf("Task[%d] reqMem  %d <<enginemem[%d] Mem %d \n",i,taskInfoP[i].reqMem,j,engineInfoP[j].mem_size);
			}
		}
	}
	for(int i=0;i<taskNum;i++){
		if (taskAssignable[i] == FALSE){
			pass = FALSE;			
			printf("Can not assign tasks since we can not assign task %c to any engine\n",taskInfoP[i].taskName);
		}
	}
	
	for (int i=0;i<engineNum;i++){
		if (engineInfoP[i].speed == 0){
			pass = FALSE;
			printf("Can not assign tasks since engine %s Speed is 0\n",engineInfoP[i].engineName);
		}
	}

	if(((taskNum % 2) == 1 && engineNum == 2) || engineNum == 1){
		pass = FALSE;
		printf("Can not assign tasks since we have to assign more than %% 50 total tasks for some engine\n");
	}

	while((taskCount < taskNum) && (assignFail == FALSE)){
		currentHighestTask = 0;
		currentHighestEngine = 0;
		pickTaskNo = -1;
		pickEngineNo = -1;
		emptyEngine = FALSE;
		for(int i=0;i<taskNum;i++){
			if(taskInfoP[i].reqMem > currentHighestTask && taskChosen[i] == FALSE){
				pickTaskNo = i;
				currentHighestTask = taskInfoP[i].reqMem;
			}
		}
		for(int i=0;i<engineNum;i++){
			if(engineCapacity[i] > 0 && engineInfoP[i].mem_size > currentHighestEngine){
				pickEngineNo = i;
				currentHighestEngine = engineInfoP[i].mem_size;
			}
		}

		if(pickEngineNo == -1 && pickTaskNo != -1){
			assignFail = TRUE;
		}

		if(assignFail == FALSE){
			if(currentHighestTask <= currentHighestEngine){
				taskChosen[pickTaskNo] = TRUE;
				engineCapacity[pickEngineNo] --;
			}
			else{
				assignFail = TRUE;
			}
		}
		taskCount ++;
	}
	if(assignFail == TRUE){
		pass = FALSE;
		printf("Can not assign tasks since we can't satisfy %% 50 tasks requirement and memory requirement at same time\n");
	}

	if(pass == TRUE)
		return TRUE;
	else
		return FALSE;
}

/*
	print the result to the relavant .txt file, 
	type 0: optimize for ET
	type 1: optimize for O
	type 2: optimize for both ET and O
*/
void OutputResult(const engine *engineInfoP,const task *taskInfoP,chromsome *chromsomeInfoP,int type){
	float engineET[MAXENGINENUM];
	int assignOrder[MAXTASKNUM];
	float highestET=0;
	float totalTransitionCost=0;
	float normalizedCost=0;
	int totalCostO = 0;
	char *dependTaskP;
	int selectedEngine = -1;
	int priorityOrder[MAXTASKNUM];//low mean less dependcy,taskNum is the highest value
	int dependNum[MAXTASKNUM];
	int priorityPointer=0;
	for(int i=0;i<MAXTASKNUM;i++){
		priorityOrder[i] = -1;
		dependNum[i] = -1;
	}
	for(int i=0;i<MAXTASKNUM;i++){
		assignOrder[i] = -1;
	}
	for(int i=0;i<MAXENGINENUM;i++){
		engineET[i] = -1;
		if(i<engineNum){
			engineET[i] = 0;
		}
	}
	
	if(chromsomeInfoP->valid == FALSE){
		chromsomeInfoP->fitnessET = 0;
		chromsomeInfoP->fitnessO = 0;
		chromsomeInfoP->fitnessNormal = 0;
		printf("We didn't find a valid solution for this time\n");
		return ;
	}

	//Calculate ET for switching part
	for(int i=0;i<taskNum;i++){ 
		if(chromsomeInfoP->gene[i] != -1){
			engineET[chromsomeInfoP->gene[i]] += (float)(taskInfoP[i].reqInstruction)/(float)(engineInfoP[chromsomeInfoP->gene[i]].speed);
			engineET[chromsomeInfoP->gene[i]] += (float)10/(float)(engineInfoP[chromsomeInfoP->gene[i]].speed);
		}
	}
	for(int i=0;i<engineNum;i++){
		if(engineET[i] > highestET){
			highestET = engineET[i];
			selectedEngine = i;
		}
	}
	//switching et store in highestET
	
	//Calculate O
	for(int i=0;i<taskNum;i++){ 
		if(chromsomeInfoP->gene[i] != -1){
			dependTaskP = taskInfoP[i].preReqTask;
			while(*dependTaskP != '\0'){
				//check all assigned tasks check whether this depend is assigned to different one
				for(int j=0;j<taskNum;j++){
					if(taskInfoP[j].taskName == *dependTaskP && chromsomeInfoP->gene[j] != -1){
						if(chromsomeInfoP->gene[i] != chromsomeInfoP->gene[j]){
							totalCostO += taskInfoP[j].outputSize;
						}
					}
				}
				dependTaskP++;
			}
		}
	}
	//O store in totalCostO
	
	//Calculate ET for transition part
	for(int i=0;i<taskNum;i++){
		dependTaskP = taskInfoP[i].preReqTask;
		if(*dependTaskP != '\0'){
			while(*dependTaskP != '\0'){
				for(int j=0;j<taskNum;j++){
					if(taskInfoP[j].taskName == *dependTaskP){
						//outgoing arrow
						if(chromsomeInfoP->gene[j] == selectedEngine && chromsomeInfoP->gene[i] != selectedEngine){
							totalTransitionCost += (float)20/(float)(engineInfoP[chromsomeInfoP->gene[j]].speed);
						}
						//incoming arrow
						if(chromsomeInfoP->gene[j] != selectedEngine && chromsomeInfoP->gene[i] == selectedEngine){
							totalTransitionCost += (float)20/(float)(engineInfoP[chromsomeInfoP->gene[j]].speed);
						}
					}
				}
				dependTaskP++;
			}
		}
	}
	//transition et store in totalTransitionCost

	normalizedCost = (float)0.7*(totalTransitionCost + highestET) + (float)0.3*(float)totalCostO;
	
	//reorder part
	int alreadyIn;
	int lackDepend;
	int findPriority;
	for(int i=0;i<taskNum;i++){ //i for current dependNum
		for(int j=0;j<taskNum;j++){ //j for current checking task j
			dependTaskP = taskInfoP[j].preReqTask;
			if(*dependTaskP == '\0' && i == 0){
				priorityOrder[priorityPointer] = j;
				priorityPointer ++;
				//printf("**we find task %c require no depend\n",taskInfoP[j].taskName);
			}
			else{
				alreadyIn = FALSE;
				for(int k=0;k<taskNum;k++){
					if(priorityOrder[k] == j){
						alreadyIn = TRUE;
					}
				}
				if(alreadyIn == FALSE){		
					lackDepend = FALSE;
					while(*dependTaskP != '\0'){
						for(int k=0;k<taskNum;k++){
							if(taskInfoP[k].taskName == *dependTaskP){
								//printf("checking for depend task %c for task %c\n",*dependTaskP,taskInfoP[j].taskName);
								findPriority = FALSE;
								for(int x=0;x<taskNum;x++){
									if(priorityOrder[x] == k){
										findPriority = TRUE;
										//printf("find task %c in prioritylist position %d\n",taskInfoP[k].taskName,x);
									}
								}
								if(findPriority == FALSE){
									lackDepend = TRUE;
									//printf("set lackDepend to TRUE for task %c\n",taskInfoP[k].taskName);
								}
							}
						}
						dependTaskP++;
						//printf(" lackDepend value is %d\n",lackDepend);
					}
					
					if(lackDepend == FALSE){
						priorityOrder[priorityPointer] = j;
						priorityPointer ++;
						//printf("**add task %c into priority list\n",taskInfoP[j].taskName);
					}
				}
			}
		}
		/*
		printf("reordered assignment:\n");
		for(int k=0;k<taskNum;k++){
			if(k == taskNum-1){
				printf(" %d\n ",priorityOrder[k]);
			}
			else{
				printf(" %d -> ",priorityOrder[k]);
			}
		}
		*/
	}
	/*
	printf("reordered assignment:\n");
	for(int i=0;i<taskNum;i++){
		if(i == taskNum-1){
			printf(" %c\n ",taskInfoP[priorityOrder[i]].taskName);
		}
		else{
			printf(" %c -> ",taskInfoP[priorityOrder[i]].taskName);
		}
	}
	*/

	for(int i=0;i<taskNum;i++){	//i for position in priority list
		for(int j=0;j<taskNum;j++){
			if(priorityOrder[i] == j){
				assignOrder[i] = chromsomeInfoP->gene[j];
			}
		
		}
	}
	
		
	if(type == OPTIMIZE_ET){
		if( (outputStream= fopen( "1.txt", "w" )) == NULL )
		{
			printf( "trying open'%s'\n","1.txt");
			printf( "open '%s' failure\n","1.txt" );
			return;
		}
		else{
			printf( "'%s' successfully opened\n","1.txt" );		
			fprintf(outputStream,"plan-01:\n");
			fprintf(outputStream,"Optimize for ET\n");
			fprintf(outputStream,"ET Cost is %f (switching cost %f + transition cost %f)\n",highestET+totalTransitionCost,highestET,totalTransitionCost);
			fprintf(outputStream,"task assignment order is\n");
			for(int i=0;i<engineNum;i++){
				fprintf(outputStream,"engine %d : ",i);
				for(int j=0;j<taskNum;j++){
					if(assignOrder[j] == i){
						fprintf(outputStream,"%c ",taskInfoP[priorityOrder[j]].taskName);
					}
				}
				fprintf(outputStream,"\n");
			}
			fclose(outputStream);
		}
	}
	else if(type == OPTIMIZE_O){
		if( (outputStream= fopen( "2.txt", "w" )) == NULL )
		{
			printf( "trying open'%s'\n","2.txt");
			printf( "open '%s' failure\n","2.txt" );
			return;
		}
		else{
			printf( "'%s' successfully opened\n","2.txt" );		
			fprintf(outputStream,"plan-02:\n");
			fprintf(outputStream,"Optimize for O\n");
			fprintf(outputStream,"total O Cost is %d \n",totalCostO);
			fprintf(outputStream,"task assignment order is\n");
			for(int i=0;i<engineNum;i++){
				fprintf(outputStream,"engine %d : ",i);
				for(int j=0;j<taskNum;j++){
					if(assignOrder[j] == i){
						fprintf(outputStream,"%c ",taskInfoP[priorityOrder[j]].taskName);
					}
				}
				fprintf(outputStream,"\n");
			}
			fclose(outputStream);
		}
	}
	else if(type == OPTIMIZE_NORMALIZED){
		if( (outputStream= fopen( "3.txt", "w" )) == NULL )
		{
			printf( "trying open'%s'\n","3.txt");
			printf( "open '%s' failure\n","3.txt" );
			return;
		}
		else{
			printf( "'%s' successfully opened\n","3.txt" );		
			fprintf(outputStream,"plan-03:\n");
			fprintf(outputStream,"Optimize for both ET and O\n");
			fprintf(outputStream,"normalized Cost is %f (I cost %f , O cost %d)\n",normalizedCost,highestET+totalTransitionCost,totalCostO);
			fprintf(outputStream,"task assignment order is\n");
			for(int i=0;i<engineNum;i++){
				fprintf(outputStream,"engine %d : ",i);
				for(int j=0;j<taskNum;j++){
					if(assignOrder[j] == i){
						fprintf(outputStream,"%c ",taskInfoP[priorityOrder[j]].taskName);
					}
				}
				fprintf(outputStream,"\n");
			}
			fclose(outputStream);
		}
	}

}

//print function used for debug
void PrintPopulationInfo(population *populationInfoP){
	printf("current Parent Generation Num %d info\n",populationInfoP->generationNum);
	for(int i=0;i<populationSize;i++){
		printf("Chromsome [%d]\n",i);
		printf("Task  : ");
		for(int j=0;j<taskNum;j++){
			printf("%2c ",j+65);
		}
		printf("\nEngine: ");
		for(int j=0;j<taskNum;j++){
			printf("%2d ",populationInfoP->choromsomeP[i].gene[j]);
		}
		printf("\n");
		printf("fitnessET : %.2f\n",populationInfoP->choromsomeP[i].fitnessET);
		printf("fitnessO : %d\n",populationInfoP->choromsomeP[i].fitnessO);
		printf("fitnessNormal : %.2f\n",populationInfoP->choromsomeP[i].fitnessNormal);
		printf("rank : %d\n",populationInfoP->choromsomeP[i].rank);
		printf("Valid : %d \n",populationInfoP->choromsomeP[i].valid);
	}

}

int RandomDice(int range){
	int temp;
	int srandPattern;
	srandPattern = randSeedCount/20;
	if(randSeedCount > 10000){
		randSeedCount = 0;
	}
	else{
		randSeedCount ++;
	}
	srand (time(NULL) + rand() + srandPattern);
	temp = rand() % range;
	return temp;
}
