#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int fd1;

    char * myfifo = "/tmp/myfifo";

    mkfifo(myfifo, 0666);

    void* memory = (int *)calloc(1024, sizeof(int));
    int inUse[256] = {0};
    char str1[80], str2[80];
    while (1)
    {
      sleep(5);
      fd1 = open(myfifo,O_RDONLY);
      read(fd1, str1, 80);
      close(fd1);
      if(strcmp(str1, "kill") == 0)
      {
        fd1 = open(myfifo,O_RDONLY);
        int index[1];
        read(fd1, index, sizeof(int));
        close(fd1);
        void* copy = memory;
        copy = copy + index[0];
        (*(int*)copy) = 0;
        inUse[index[0]/4] = 0;
      }
      else if(strcmp(str1, "allocate") == 0)
      {
        int physicalFree = 0;
        int returnAdd = 0;
        while(physicalFree < 256)
        {
          if(inUse[physicalFree] == 0)
          {
            inUse[physicalFree] = 1;
            break;
          }
          returnAdd = returnAdd + 4;
          physicalFree++;
        }
        int writeBack[1] = {returnAdd};
        fd1 = open(myfifo,O_WRONLY);
        write(fd1, writeBack, sizeof(int));
        close(fd1);
      }
      else if(strcmp(str1, "read") == 0)
      {
        fd1 = open(myfifo,O_RDONLY);
        int index[1];
        read(fd1, index, sizeof(int));
        close(fd1);
        void* copy = memory;
        if(index[0] % 4 != 0)
        {
          int x[1] = {-2};
          fd1 = open(myfifo,O_WRONLY);
          write(fd1, x, sizeof(int));
          close(fd1);
        }
        if(index[0] >= 1024)
        {
          int x[1] = {-1};
          fd1 = open(myfifo,O_WRONLY);
          write(fd1, x, sizeof(int));
          close(fd1);
        }
        else
        {
          copy = copy + index[0];
          int x = (*(int*)copy);
          int returnAdd[1] = {x};
          fd1 = open(myfifo,O_WRONLY);
          write(fd1, returnAdd, sizeof(int));
          close(fd1);
        }
      }
      else if(strcmp(str1, "write") == 0)
      {
        fd1 = open(myfifo,O_RDONLY);
        int index[1];
        int newPhy[1];
        read(fd1, index, sizeof(int));
        read(fd1, newPhy, sizeof(int));
        close(fd1);
        if(index[0] % 4 != 0 )
        {
          int x[1] = {-2};
          fd1 = open(myfifo,O_WRONLY);
          write(fd1, x, sizeof(int));
          close(fd1);
        }
        if(index[0] >= 1024)
        {
          int x[1] = {-1};
          fd1 = open(myfifo,O_WRONLY);
          write(fd1, x, sizeof(int));
          close(fd1);
        }
        else
        {
          void* copy = memory;
          int physicalIndex = index[0];
          copy = copy + physicalIndex;
          (*(int*)copy) = newPhy[0];
          int x[1] = {0};
          fd1 = open(myfifo,O_WRONLY);
          write(fd1, x, sizeof(int));
          close(fd1);
        }
      }
    }
}

