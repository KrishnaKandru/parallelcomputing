#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include<string.h>
#include<mpi.h>
#include<time.h>

int **allocArray(int P, int Q)
 {
    int i;
    int *p, **a;
    
    p = (int *)malloc(P*Q*sizeof(int));
    a = (int **)malloc(P*sizeof(int *));
    
    if (p == NULL || a == NULL)
        printf("Error allocating memory\n");
    for (i = 0; i < P; i++)
        a[i] = &p[i*Q];
    
    return a;
}

int **seed(int **world, int rowlength, int collength, int rank, int transcript) 
{
    int i,j;
    
    for (i=0; i<rowlength; i++) {
        int integer = i + transcript;
        srand(integer);
        for (j=0; j<collength; j++) {
            if (i==0 || j==0 || i==rowlength-1 || j==collength-1) {
                world[i][j] = 0; 
            } else {
                world[i][j] = (rand()%2);
            }
        }
    }
    
    return world;
}

void printWorld(int **world, int rowlength, int collength) {
    int i,j;
    
    for (i=0; i<rowlength; i++) {
        for (j=0; j<collength; j++)
            printf("%d ", world[i][j]);
        printf("\n");
    }
}

void step(int **world1, int **world2, int rowlength, int collength) {
    int neighbours = 0;
    int i,j;
    for (i=1;i<rowlength-1;i++) {
        for (j = 1; j < collength-1; j++) {
            neighbours = 0; 
            if (world1[i-1][j] == 1) {
                neighbours++;
            }  if (world1[i-1][j-1] == 1) {
                neighbours++;
            }if (world1[i-1][j+1] == 1) {
                neighbours++;
            }if (world1[i][j-1] == 1) {
                neighbours++;
            }if (world1[i][j+1] == 1) {
                neighbours++;
            }if (world1[i+1][j] == 1) {
                neighbours++;
            }if (world1[i+1][j-1] == 1) {
                neighbours++;
            }if (world1[i+1][j+1] == 1) {
                neighbours++;
            }if (world1[i][j] == 1) {
                if (neighbours <= 1) {
                    world2[i][j] = 0;
                } else if (neighbours >= 4) {
                    world2[i][j] = 0;
                } else if (neighbours == 2 || neighbours == 3) {
                    world2[i][j] = 1;
                }
            } else {
                if (neighbours == 3) {
                    world2[i][j] = 1;
                } else {
                    world2[i][j] = 0;
                }
            }
        }
    }
}



int main(int argc, char **argv)
{
    int size, I,itr, local_size, tag1=1, tag2=2;
    int **world1=NULL, **world2=NULL, **temp= NULL;
    clock_t start_time;
    clock_t end_time;
    int comm_sz, rank, past, present;
    int j,turn=1;

    size = atoi(argv[1]);
    I = atoi(argv[2]);


    MPI_Request reqs[4];
    MPI_Status stats[4];
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    present = rank+1;
    past = rank-1;
    
    if (rank == 0) {
        past = MPI_PROC_NULL;
    }
    
    if (rank == comm_sz-1) {
        present = MPI_PROC_NULL;
    }
    
    int *counter = (int *)malloc(sizeof(int)*comm_sz);
    int *transcript = (int *)malloc(sizeof(int)*comm_sz);
    
    for (j=0; j<comm_sz; j++) {
        counter[j] = size/comm_sz + ((j<size%comm_sz)?1:0);
    }
    for (transcript[0]=0,j=1; j<comm_sz; j++) {
        transcript[j] = transcript[j-1] + counter[j-1];
    }
    
    local_size = size/comm_sz + (rank < size%comm_sz ? 1:0);
    world1 = allocArray(local_size+2, size+2);
    world2 = allocArray(local_size+2, size+2);
    
    seed(world1, local_size+2, size+2, rank, transcript[rank]);
    start_time = clock();
    
    for (itr = 0; itr < I && turn !=0; itr++) {

        MPI_Send(&world1[1][0], size+2, MPI_INT, past, tag2, MPI_COMM_WORLD);
        MPI_Recv(&world1[0][0], size+2, MPI_INT, past, tag1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        MPI_Recv(&world1[local_size+1][0], size+2, MPI_INT, present, tag2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        
        MPI_Send(&world1[local_size][0], size+2, MPI_INT, present, tag1, MPI_COMM_WORLD);

        step(world1, world2, local_size+2, size+2);
        
        temp = world1;
        world1 = world2;
        world2 = temp;
        //using print statements here will print mtrices
    }
    
    
    MPI_Finalize();
    end_time = clock() - start_time;
    double time_taken = ((double)end_time)/CLOCKS_PER_SEC;
    
    printf("The program took %lf seconds to execute", time_taken);
    
    return 0;
}
