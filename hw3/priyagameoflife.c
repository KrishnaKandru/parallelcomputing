// Name: Krishna Priya Kandru
// Blazer Id: b0159342
// Course Section:CS 632
// Homework #: 3
// Instructions to compile the program: icc -qopenmp priyagameoflife.c -o hw3
// Instructions to run the program: ./output generations size threads
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <sys/time.h>
#define SLEEP Sleep(300)
#define CLS system("cls")
#else
#include <unistd.h>
#define SLEEP usleep(30000)
#define CLS printf("\033[H\033[J")
#endif

int **seed(int rowlength, int collength, int **world)
{ 
  int i,j;
  for (j=0;j<rowlength;j++)
  {
  for(i=0;i<collength;i++)
  {
    world[i][j]=(rand()%2);
  }
  }
  return world;
}


int **allocArray(int P, int Q) {
  int i;
  int *p, **size;

  p = (int *)malloc(P*Q*sizeof(int));
  size = (int **)malloc(P*sizeof(int*));

  if (p == NULL || size == NULL)
    printf("Error allocating memory\n");
  for (i = 0; i < P ; i++)
    size[i] = &p[i*Q];
  return size;
}

void printWorld(int rowlength, int collength, int **world)
{ 
  int i,j;
  for (j = 0; j < rowlength; j++)
  {
    for(i = 0 ;i < collength; i++)
    {
      printf("%d", world[i][j]);
    }
  printf("\n");
  }
}

void evaluate_neighbours(int i,int j, int **world_a, int **world_b)
{
  int neighbours;
    
  neighbours = world_a[i-1][j-1]+world_a[i-1][j]+world_a[i-1][j+1]+world_a[i][j-1]+world_a[i][j+1]+world_a[i+1][j-1]+world_a[i+1][j]+world_a[i+1][j+1];

      if (world_a[i][j] == 1)
      {    
        if (neighbours == 2 || neighbours == 3)
        {
          world_b[i][j]=1;
        }
        else
        {
          world_b[i][j]=0;
        }
      }
      else
      {
        if (neighbours == 3)
          {
            world_b[i][j]=1;
          }
        else
        {
          world_b[i][j]=0;
        }
      }

}

void step(int board,int **world_a, int **world_b)
{
  int i,j,neighbours;

  int number = omp_get_thread_num();   
  int count = omp_get_num_threads();
  int len = board / count;         
  int row = 1 +number*len;    
  int rows = (number+1)*len; 
for (i=row; i <= rows; i++)
{
  for (j=1; j <= board; j++)
  {
    
    evaluate_neighbours(i,j,world_a,world_b);
    
    }
  }
}
  

int **addghosts(int b, int c, int **size) {

    int i,j;

    
    for (i=1; i<(c - 1) ; i++)
    {
        size[0][i] = size[b-2][i];
    }
    for (i=1; i<(c - 1) ; i++)
    {
        size[b-1][i] = size[1][i];
    }
    for (i=0; i<b ; i++)
    {
        size[i][0] = size[i][c-2];
    }
    for (i=0; i<b ; i++)
    {
        size[i][c-1] = size[i][1];
    }

  return size;
}

int main(int argc, char **argv)
{
  clock_t start_time;
  clock_t end_time;
  int size;
  // int T;
  int I;
  int  nextgenera, previous, processNum, processlevel,chunk, count = 0,sign=1, sign1=2;
  int i,k;
  int turn = 1;
  int iteration;
  int **world1=NULL, **world2=NULL, **temp = NULL;
  double start, end;
  size = atoi(argv[1]); //size of board
  I = atoi(argv[2]);   //no:of iterations
  // T = atoi(argv[3]);  // no:of threads      
  MPI_Request reqs[4];
  MPI_Status stats[4];

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &processNum);
  MPI_Comm_rank(MPI_COMM_WORLD, &processlevel);

 if (processlevel == processNum -1){
    
    int s = size / processNum;
    chunk = size - (processNum-1)*s;
    
  }else {
    chunk = size / processNum;

  }
  world1 = allocArray(chunk+2,size+2);
  world2 = allocArray(chunk+2,size+2);
  
  srand(123456);
  printf("Welcome to Conway's Game of Life");
  printf("\n\n");
  world2 = seed(chunk+2, size+2, world2);
  world2 = addghosts(chunk+2, size+2, world2);
  previous = processlevel-1;
  nextgenera = processlevel+1;
  if (processlevel == 0)  {previous = processNum - 1;}
  if (processlevel == (processNum - 1))  {nextgenera = 0;}

  // printf("\n");
  // printWorld(chunk+2, size+2, world2);

  // CLS;
  if (processlevel == 0)
  {
  start_time = clock();
}

  printf("\nIterations are about to start\n\n");
  
  for(iteration = 0; iteration < I && turn !=0; iteration++)
  {
    // #pragma omp parallel num_threads(T)
    
     // SLEEP;
    MPI_Irecv(&world2[0][0], size+2, MPI_INT, previous, sign, MPI_COMM_WORLD, &reqs[0]);
    MPI_Irecv(&world2[chunk + 1][0], size+2, MPI_INT, nextgenera, sign1, MPI_COMM_WORLD, &reqs[1]);

    MPI_Isend(&world2[1][0], size+2, MPI_INT, previous, sign1, MPI_COMM_WORLD, &reqs[2]);
    MPI_Isend(&world2[chunk][0], size+2, MPI_INT, nextgenera, sign, MPI_COMM_WORLD, &reqs[3]);

    MPI_Waitall(4, reqs, stats);
    step(world2, world1, chunk, size);
    temp = world1;
        world1 = world2;
        world2 = temp;
    world2 = addghosts(world2, chunk+2, size+2);
    count += 1;
    MPI_Barrier(MPI_COMM_WORLD);
  }
    
      //   step(size,world2, world1);
      //   temp = world1;
      //   world1 = world2;
      //   world2 = temp;
      //   world2= addghosts(size+2,size+2,world2);
      //   printf("\n");
      //   printf("%d",iteration);
      //   printf("\n");
      //   printWorld(size+2,size+2, world2);
      // }
      // else if (turn == 1)
      // {
      //   step(size,world2, world1);
      //   temp = world1;
      //   world1 = world2;
      //   world2 = temp;
      //   // world2= addghosts(size+2,size+2,world2);
      //   printWorld(size,size, world2);
      //   printf("\n");
      //   turn = 0;
      // }
    // SLEEP;
  // }  

if (processlevel == 0){
  
  end = clock()- start_time;
  double time_taken = ((double)end_time)/CLOCKS_PER_SEC;
  printf("The program took %lf seconds to execute", time_taken);
  printf("\n");
}

  MPI_Finalize();
  return 0;
}






