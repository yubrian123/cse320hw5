#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct secondLevel
{
	int occupied2;
	int physicalMem;
}secondLevel;

typedef struct firstLevel
{
	int occupied1;
	secondLevel pointSecond[8];
}firstLevel;

typedef struct processStruct
{
	pthread_t threadID;
	int inUse;
	firstLevel firstLevelPT[8];
}processStruct;

typedef struct cache
{
	pthread_t threadID;
	long virtualAddress;
	int physicalAddress;
}cache;

void* threadFunction()
{
        while(1)
        {
        }
}

void initialize(processStruct processes[])
{
  int i;
	int j;
	int k;
	for(i = 0; i < 4; i++)
	{
    processes[i].inUse = 0;
		for(j = 0; j < 8; j++)
		{
			for(k = 0; k < 8; k++)
			{
				processes[i].firstLevelPT[j].pointSecond[k].occupied2 = 0;
				processes[i].firstLevelPT[j].pointSecond[k].physicalMem = 0;
			}
			processes[i].firstLevelPT[j].occupied1 = 0;
		}
  }
}

int cse320_malloc(processStruct process[], int position)
{
	int i = 0;
	int j = 0;
	while(i < 8)
	{
		if(process[position].firstLevelPT[i].occupied1 == 0)
		{
			while(j < 8)
			{
				if(process[position].firstLevelPT[i].pointSecond[j].occupied2 == 0)
				{
					int k = i;
					k = k << 22;
					int l = j;
					l = l << 12;
					k = k|l;
					process[position].firstLevelPT[i].pointSecond[j].occupied2 = 1;
					if(j == 7)
					{
						process[position].firstLevelPT[i].occupied1 = 1;
					}
					return k;
				}
				j++;
			}
		}
		i++;
	}
	printf("No space to allocate \n");
	exit(0);
}

int writeToPhysical(processStruct process[], int position, int physicalIndex, int allocatePosition)
{
	int i = allocatePosition >> 22;
	int j = allocatePosition << 10;
	j = j >> 22;
	if(process[position].firstLevelPT[i].pointSecond[j].occupied2 == 1)
	{
		process[position].firstLevelPT[i].pointSecond[j].physicalMem = physicalIndex;
	}
}

//Taken from https://www.programmingsimplified.com/c/source-code/c-program-convert-decimal-to-binary
int decToBinary(int n)
{
	int c, k;
	for (c = 31; c >= 0; c--)
	{
		k = n >> c;

		if (k & 1)
			printf("1");
		else
			printf("0");
	}
}

int printMem(processStruct process)
{
	int i;
	int j;
	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 8; j++)
		{
			if(process.firstLevelPT[i].pointSecond[j].occupied2 == 1)
			{
				int k = i;
				k = k << 22;
				int l = j;
				l = l << 12;
				k = k|l;
				decToBinary(k);
				printf("\n");
			}
		}
	}
}

int cse_320_virt_to_phys(processStruct process, long memAdds)
{
	int i = memAdds >> 22;
	int j = memAdds << 10;
	j = j >> 22;
	if(process.firstLevelPT[i].pointSecond[j].occupied2 == 1)
	{
		return process.firstLevelPT[i].pointSecond[j].physicalMem;
	}
	else
	{
		return -1;
	}
}

void initializeCache(cache caches[])
{
	int i;
	for(i = 0; i < 4; i++)
	{
		caches[i].physicalAddress = 0;
		caches[i].virtualAddress = 0;
	}
}

void addToCache(cache caches[], int physicalAddress, long virtualAddress, pthread_t threadID)
{
	int i = 0;
	while(i < 4)
	{
		if(caches[i].threadID == 0)
		{
			caches[i].threadID = threadID;
			caches[i].virtualAddress = virtualAddress;
			caches[i].physicalAddress = physicalAddress;
			printf("cache miss \n");
			break;
		}
		else if(caches[i].threadID == threadID && caches[i].virtualAddress == virtualAddress && caches[i].physicalAddress == physicalAddress)
		{
			printf("cache hit \n");
			break;
		}
		i++;
	}
	if(i == 4)
	{
		caches[0].threadID = threadID;
		caches[0].virtualAddress = virtualAddress;
		caches[0].physicalAddress = physicalAddress;
		printf("eviction \n");
	}
}

int checkCache(cache caches[], long virtualAddress, pthread_t threadID)
{
	int i = 0;
	while(i < 4)
	{
		if(caches[i].threadID == 0)
		{
			caches[i].threadID = threadID;
			caches[i].virtualAddress = virtualAddress;
			printf("cache miss \n");
			return caches[i].physicalAddress;
		}
		else if(caches[i].threadID == threadID && caches[i].virtualAddress == virtualAddress)
		{
			printf("cache hit \n");
			return caches[i].physicalAddress;
		}
		i++;
	}
	return -1;
}

void checkWriteCache(cache caches[], int physicalAddress, long virtualAddress, pthread_t threadID)
{
	int i = 0;
	while(i < 4)
	{
		if(caches[i].threadID == threadID && caches[i].virtualAddress == virtualAddress)
		{
			caches[i].physicalAddress = physicalAddress;
			printf("cache hit \n");
			break;
		}
		i++;
	}
	if(i == 4)
	{
		caches[0].threadID = threadID;
		caches[0].virtualAddress = virtualAddress;
		caches[0].physicalAddress = physicalAddress;
		printf("eviction \n");
	}
}

void killCache(cache caches[], pthread_t threadID)
{
	int i = 0;
	while(i < 4)
	{
		if(caches[i].threadID == threadID)
		{
			caches[i].threadID = 0;
			caches[i].virtualAddress = 0;
			caches[i].physicalAddress = 0;
			printf("cache hit \n");
		}
		i++;
	}
}

int main()
{
  printf("Please Type in your Command \n");
  char command[255];
  int numOfThreads = 0;
  processStruct processes[4];
	cache caches[4];
	initializeCache(caches);
  initialize(processes);
	int fd;
	char * myfifo = "/tmp/myfifo";
  mkfifo(myfifo, 0666);
  while (1)
  {
    printf(" -> ");
    fgets(command, 255 ,stdin);
    int n = strlen(command);
    if (n > 0 && command[n - 1] == '\n')
      command[n-1] = '\0';
    if (strcmp(command, "exit") == 0)
    {
			int i;
			int j;
			int k;
			for(i = 0; i < 4; i++)
			{
				if(processes[i].inUse == 1)
				{
					pthread_cancel(processes[i].threadID);
				}
				for(j = 0; j < 8; j++)
				{
					for(k = 0; k < 8; k++)
					{
						if(processes[i].firstLevelPT[j].pointSecond[k].occupied2 == 1)
						{
							fd = open(myfifo, O_WRONLY);
							int pipe[1] = {processes[i].firstLevelPT[j].pointSecond[k].physicalMem};
							write(fd, "kill", strlen("kill")+1);
							write(fd, pipe, sizeof(int));
							close(fd);
						}
					}
				}
		  }
			initialize(processes);
      exit(0);
    }
    else if (strcmp(command, "create") == 0)
    {
      if(numOfThreads >= 4)
      {
          printf("Too Many Processes");
          exit(0);
      }
      else
      {
        int addingProcessToArray = 0;
        while(processes[addingProcessToArray].inUse != 0)
        {
          addingProcessToArray++;
        }
        pthread_create(&(processes[addingProcessToArray].threadID), NULL, threadFunction, NULL);
        processes[addingProcessToArray].inUse = 1;
        numOfThreads++;
      }
    }
    else if (strcmp(command, "list") == 0)
    {
      int printCount = 0;
      while(printCount < 4)
      {
        if(processes[printCount].inUse == 1)
        {
          printf("%lu \n", processes[printCount].threadID);
        }
        printCount++;
      }
    }
		else
		{
			char *token = strtok(command, " ");
	    if(strcmp(command, "kill") == 0)
	    {
	      char* args[2] = {NULL, NULL};
	      token = strtok(NULL, " ");
				if(token == NULL)
	      {
	        printf("No Flags for Kill \n");
	        exit(0);
	      }
	      args[0] = token;
	      token = strtok(NULL, " ");
	      if(token != NULL)
	      {
	        printf("Invalid Input \n");
	        exit(0);
	      }
	      else
	      {
	        long find = atol(args[0]);
					int findCancelCounter = 0;
					int canceled = 0;
					killCache(caches, find);
					while(findCancelCounter < 4)
					{
						if(processes[findCancelCounter].threadID == find)
		        {
							pthread_cancel(processes[findCancelCounter].threadID);
							processes[findCancelCounter].inUse = 0;
							numOfThreads--;
							int j,k;
							for(j = 0; j < 8; j++)
							{
								for(k = 0; k < 8; k++)
								{
									if(processes[findCancelCounter].firstLevelPT[j].pointSecond[k].occupied2 == 1)
									{
										fd = open(myfifo, O_WRONLY);
										int pipe[1] = {processes[findCancelCounter].firstLevelPT[j].pointSecond[k].physicalMem};
										write(fd, "kill", strlen("kill")+1);
										write(fd, pipe, sizeof(int));
										close(fd);
									}
									processes[findCancelCounter].firstLevelPT[j].pointSecond[k].occupied2 = 0;
									processes[findCancelCounter].firstLevelPT[j].pointSecond[k].physicalMem = 0;
								}
								processes[findCancelCounter].firstLevelPT[j].occupied1 = 0;
							}
							canceled++;
						}
						findCancelCounter++;
					}
					if(canceled == 0)
					{
						printf("Process not found \n");
					}
	      }
	    }
			else if(strcmp(command, "allocate") == 0)
			{
				char* args[2] = {NULL, NULL};
	      token = strtok(NULL, " ");
				if(token == NULL)
	      {
	        printf("No Process for Allocate \n");
	        exit(0);
	      }
	      args[0] = token;
	      token = strtok(NULL, " ");
	      if(token != NULL)
	      {
	        printf("Invalid Input \n");
	        exit(0);
	      }
				long find = atol(args[0]);
				int findAllocateCounter = 0;
				int allocate = 0;
				while(findAllocateCounter < 4)
				{
					if(processes[findAllocateCounter].threadID == find)
					{
						allocate = cse320_malloc(processes, findAllocateCounter);
						fd = open(myfifo, O_WRONLY);
						write(fd, "allocate", strlen("allocate")+1);
						close(fd);
						fd = open(myfifo, O_RDONLY);
						int messageBack[1];
						read(fd, messageBack, sizeof(int));
						writeToPhysical(processes, findAllocateCounter, messageBack[0], allocate);
						close(fd);
					}
					findAllocateCounter++;
				}
			}
			else if(strcmp(command, "mem") == 0)
			{
				char* args[2] = {NULL, NULL};
	      token = strtok(NULL, " ");
				if(token == NULL)
	      {
	        printf("No Process for Mem \n");
	        exit(0);
	      }
	      args[0] = token;
	      token = strtok(NULL, " ");
	      if(token != NULL)
	      {
	        printf("Invalid Input \n");
	        exit(0);
	      }
				long find = atol(args[0]);
				int findMemCounter = 0;
				int mem = 0;
				while(findMemCounter < 4)
				{
					if(processes[findMemCounter].threadID == find)
					{
						printMem(processes[findMemCounter]);
					}
					findMemCounter++;
				}
			}
			else if(strcmp(command, "read") == 0)
			{
				char* args[2] = {NULL, NULL};
	      token = strtok(NULL, " ");
				if(token == NULL)
	      {
	        printf("Illegal read command \n");
	        exit(0);
	      }
				args[0] = token;
	      token = strtok(NULL, " ");
				if(token == NULL)
	      {
	        printf("Illegal read command \n");
	        exit(0);
	      }
				args[1] = token;
				token = strtok(NULL, " ");
				if(token != NULL)
	      {
	        printf("Invalid Input \n");
	        exit(0);
	      }
				char* endprt;
				long findAdd = strtol(args[1], &endprt, 2);
				long findMem = atol(args[0]);
				int readMemCounter = 0;
				int mem = 0;
				int readCounter;
				int check = checkCache(caches, findAdd, findMem);
				if(check > -1)
				{
					printf("%d \n", check);
				}
				else 
				{
					while(readMemCounter < 4)
					{
						if(processes[readMemCounter].threadID == findMem)
						{
							readCounter = cse_320_virt_to_phys(processes[readMemCounter], findAdd);
							if(readCounter == -1)
							{
								printf("Virtual Address not in use \n");
								exit (0);
							}
							fd = open(myfifo, O_WRONLY);
							int pipe[1] = {readCounter};
							write(fd, "read", strlen("read")+1);
							write(fd, pipe, sizeof(int));
							close(fd);
							fd = open(myfifo, O_RDONLY);
							int messageBack[1];
							read(fd, messageBack, sizeof(int));
							close(fd);
							addToCache(caches, messageBack[0], findMem, findAdd);
							if(messageBack[0] == -1)
							{
								printf("error, address out of range");
								exit(0);
							}
							if(messageBack[0] == -2)
							{
								printf("error, address is not aligned");
								exit(0);
							}
							printf("%d \n", messageBack[0]);
							mem++;
						}
						readMemCounter++;
					}
					if(mem == 0)
					{
						printf("Process Address Not found \n");
						exit (0);
					}
				}
			}
			else if(strcmp(command, "write") == 0)
			{
				char* args[3] = {NULL, NULL, NULL};
	      token = strtok(NULL, " ");
				if(token == NULL)
	      {
	        printf("Illegal write command \n");
	        exit(0);
	      }
				args[0] = token;
	      token = strtok(NULL, " ");
				if(token == NULL)
	      {
	        printf("Illegal write command \n");
	        exit(0);
	      }
				args[1] = token;
				token = strtok(NULL, " ");
				if(token == NULL)
	      {
					printf("Illegal write command \n");
	        exit(0);
	      }
				args[2] = token;
				token = strtok(NULL, " ");
				if(token != NULL)
	      {
	        printf("Invalid Input \n");
	        exit(0);
	      }
				char* endprt;
				long writeAdd = strtol(args[1], &endprt, 2);
				long writeMem = atol(args[0]);
				int newPhysical = atoi(args[2]);
				int writeMemCounter = 0;
				int memWrite = 0;
				int writeCounter = 0;
				checkWriteCache(caches, newPhysical, writeAdd, writeMem);
				while(writeMemCounter < 4)
				{
					if(processes[writeMemCounter].threadID == writeMem)
					{
						int readCounter;
						readCounter = cse_320_virt_to_phys(processes[writeMemCounter], writeAdd);
						if(readCounter == -1)
						{
							printf("Virtual Address not in use \n");
							exit (0);
						}
						fd = open(myfifo, O_WRONLY);
						int pipe[1] = {readCounter};
						int pipe2[1] = {newPhysical};
						write(fd, "write", strlen("write")+1);
						write(fd, pipe, sizeof(int));
						write(fd, pipe2, sizeof(int));
						close(fd);
						fd = open(myfifo, O_RDONLY);
						int messageBack[1];
						read(fd, messageBack, sizeof(int));
						close(fd);
						if(messageBack[0] == -1)
						{
							printf("error, address out of range");
							exit(0);
						}
						if(messageBack[0] == -2)
						{
							printf("error, address is not aligned");
							exit(0);
						}
						writeCounter++;
					}
					writeMemCounter++;
				}
				if(writeCounter == 0)
				{
					printf("Process Address Not found \n");
					exit (0);
				}
			}
			else
			{
				printf("Invalid Input \n");
				exit(0);
			}
		}
  }
}

