/*
Name:Krishna Priya Kandru
BlazerId: b0159342
Homework #:5
*/


#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>



void gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
    int rank, sz, i, off;
    MPI_Status  *s1;
    MPI_Request *r1;
    MPI_Aint lbound, sizeofsendtype, sizeofrecvtype;
    char *buffer;
    MPI_Comm_rank (comm, &rank);
    MPI_Comm_size (comm, &sz);

    s1 = malloc(sizeof(MPI_Status)*(sz+1));
    r1 = malloc(sizeof(MPI_Request)*(sz+1));
if(rank!=0){
    MPI_Type_get_extent(sendtype, &lbound, &sizeofsendtype);
    MPI_Send(sendbuf, sizeofsendtype*sendcount, MPI_CHAR, root, 0, comm);}
    

    if (rank == 0) { 
        MPI_Type_get_extent(sendtype, &lbound, &sizeofsendtype);
        MPI_Isend(sendbuf, sizeofsendtype*sendcount, MPI_CHAR, root, 0, comm,&r1[sz]);
        
        
    for (i = 0; i < sz; i++) {
        MPI_Type_get_extent(recvtype, &lbound, &sizeofrecvtype);
        off = sizeofrecvtype*recvcount*i;
        buffer = recvbuf + off;
            MPI_Irecv(buffer, sizeofrecvtype*recvcount, MPI_CHAR, i, 0, comm, &r1[i]);
        }
        MPI_Waitall(sz+1, r1, s1);
        
        
    }  
   
    free(r1);
    free(s1);
}

void broadcast(void *sendbuf, int sendcount, MPI_Datatype sendtype,void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
    int rank, sz;
    int i, off;
    MPI_Status  *s1;
    MPI_Request *r1;
    MPI_Aint lbound, sizeofsendtype, sizeofrecvtype;

    MPI_Comm_rank (comm, &rank);
    MPI_Comm_size (comm, &sz);

    s1 = malloc(sizeof(MPI_Status)*(sz+1));
    r1 = malloc(sizeof(MPI_Request)*(sz+1));

    if (rank == 0) { 
    for (i = 0; i < sz; i++) {
        MPI_Type_get_extent(sendtype, &lbound, &sizeofsendtype);
        off = sizeofsendtype*sendcount*i;
        char *buffer = sendbuf + off;
            MPI_Isend(buffer, sizeofsendtype*sendcount, MPI_CHAR, i, 0, comm, &r1[i]);
        }
        MPI_Type_get_extent(recvtype, &lbound, &sizeofrecvtype);
        MPI_Irecv(recvbuf, sizeofrecvtype*recvcount, MPI_CHAR, 0, 0, comm, &r1[i]);
        MPI_Waitall(sz+1, r1, s1);
    } else { 
        MPI_Status s1;
        MPI_Type_get_extent(recvtype, &lbound, &sizeofrecvtype);
        MPI_Recv(recvbuf, sizeofrecvtype*recvcount, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    free(r1);
    free(s1);
}

void pairwise(){

    int sz,rank;
    int sendbuf, recvbuf;
    int tag1,tag2;
    int local_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank %2 == 0){
        local_rank = rank+1;
    }
    else if(rank %2 !=0){
        local_rank = rank -1;

    }

        MPI_Sendrecv(&sendbuf, 1, MPI_INT, local_rank, 0,&recvbuf, 1, MPI_INT, local_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

}





int main(int argc, char **argv)
{
    int i, rank, sz, root=0, off;
    int *sendbuf, *recvbuf=NULL;
    double starttime, endtime;
    int N,j;
    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &sz);

    
   N=32;
   for(j=0;j<16;j++){
    if (rank == 0) {
    recvbuf = (int *)malloc(sizeof(int)*N*sz);
    for (i=0; i<N*sz; i++)
        recvbuf[i] = N*i;
    }
    sendbuf = (int *)malloc(sizeof(int)*N);
    for(i = 0; i<=N;i++){
        sendbuf[i] = 10*N*i;
    }
   
    MPI_Barrier(MPI_COMM_WORLD);
    starttime = MPI_Wtime();
for (i=0; i < 100; i++)
   
    gather(sendbuf, N, MPI_INT, recvbuf, N, MPI_INT, 0, MPI_COMM_WORLD);
    broadcast(recvbuf, N, MPI_INT, sendbuf, N, MPI_INT, 0, MPI_COMM_WORLD);
    if(sz % 2 == 0)
    pairwise();
   
    endtime = MPI_Wtime() - starttime;

    MPI_Reduce(&endtime, &starttime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

   

    if (rank == 0) {
       printf("time taken for %d bytes is %g\n", N,starttime);
    }
    N= N*2;
}

    MPI_Finalize ();

    return 0;
}
