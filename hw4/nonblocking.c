#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <mpi.h>


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
  int *world_b, **size;

  world_b = (int *)malloc(P*Q*sizeof(int));
  size = (int **)malloc(P*sizeof(int*));

  if (world_b == NULL || size == NULL)
    printf("Error allocating memory\n");
  for (i = 0; i < P ; i++)
    size[i] = &world_b[i*Q];
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




int **addghosts(int I, int c, int **size) {

    int i,j;

    
    for (i=1; i<(c - 1) ; i++)
    {
        size[0][i] = size[I-2][i];
    }
    for (i=1; i<(c - 1) ; i++)
    {
        size[I-1][i] = size[1][i];
    }
    for (i=0; i<I ; i++)
    {
        size[i][0] = size[i][c-2];
    }
    for (i=0; i<I ; i++)
    {
        size[i][c-1] = size[i][1];
    }

  return size;
}



void step(int **world_a, int **world_b, int size1, int size2) {
  int i,j,neighbours;
  for (i=1; i<=size1; i++)
  {
      for (j=1; j<=size2; j++)
      {
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
  }
}






int main(int argc, char **argv){

    int I, itr,size, local_size, count = 0,local_a=1, local_b=2;
    int **world1=NULL, **world2 = NULL, **temp = NULL;
    clock_t start_time;
    clock_t end_time;
    int turn = 1;
    int nextprocess, pastprocess, comm_sz, my_rank,size;
    
    size = atoi(argv[1]);
    I = atoi(argv[2]);
  
  
  MPI_Request reqs[4];
  MPI_Status stats[4];

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  
  if (my_rank == comm_sz -1){
    
    int s = size / comm_sz;
    local_size = size - (comm_sz-1)*s;
    
  }else {
    local_size = size / comm_sz;

  }
  
  
    // allocate memory
  world1 = allocArray(local_size+2, size+2);
    world2 = allocArray(local_size+2, size+2);

  srand(time(NULL) + my_rank);
  //srand48(123456);

    world2 = seed(local_size+2, size+2, world2);
    world2 = addghosts(local_size+2, size+2, world2);
  pastprocess = my_rank-1;
  nextprocess = my_rank+1;
  if (my_rank == 0)  {pastprocess = comm_sz - 1;}
  if (my_rank == (comm_sz - 1))  {nextprocess = 0;}

   
  if (my_rank == 0){start_time = clock();}

  
  for (itr = 0; itr < I && turn != 0; itr++) {
    
    
    MPI_Irecv(&world2[0][0], size+2, MPI_INT, pastprocess, local_a, MPI_COMM_WORLD, &reqs[0]);
    MPI_Irecv(&world2[local_size + 1][0], size+2, MPI_INT, nextprocess, local_b, MPI_COMM_WORLD, &reqs[1]);

    MPI_Isend(&world2[1][0], size+2, MPI_INT, pastprocess, local_b, MPI_COMM_WORLD, &reqs[2]);
    MPI_Isend(&world2[local_size][0], size+2, MPI_INT, nextprocess, local_a, MPI_COMM_WORLD, &reqs[3]);

    
    MPI_Waitall(4, reqs, stats);
    step(world2, world1, local_size, size);
    temp = world1;
        world1 = world2;
        world2 = temp;
    world2 = addghosts(local_size+2, size+2, world2);
    printf("\n");
    printf("%d",itr);
    printf("\n");
    printWorld(local_size+2, size+2, world2);
    MPI_Barrier(MPI_COMM_WORLD);
  }
    
  if (my_rank == 0){
    end_time= clock()-start_time;
    double time_taken = ((double)end_time)/CLOCKS_PER_SEC;
    printf("The program took %lf seconds to execute", time_taken);
  }

  MPI_Finalize();

    return 0;
}
