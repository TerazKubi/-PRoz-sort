#include <mpi.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#define SORT 1
#define END 2
// Procesów musi być o jeden więcej niż TABSIZE
#define TABSIZE 5


// Kompilujemy mpicc, uruchamiamy mpirun -np N ./a.out liczby_do_sortowania
// gdzie N to liczba procesów
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int size,rank;
    int min=-1;
    int tmp=-1;
    int i;
    int end=0;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // int TABSIZE = 5;
    int tablica[TABSIZE]={0};
    int sorted[TABSIZE]={-1};

    int first_num = 0;

    int next = rank+1;
    if(next > size-1) next = size-1;

    if (argc<2) {
	    MPI_Finalize();
	    printf("Za malo argumentow\n");
	    exit(0);
    }
    /*
        Szkielet dzieli procesy na dwie kategorie: wierzchołek, liście
    */

    if (rank == 0) {
        /* Wczytywanie liczb do posortowania z pliku podanego jako pierwszy argument */
        printf("Otwieram plik\n");
        FILE *f;int i;
        f = fopen(argv[1],"r");
        for (i=0;i<TABSIZE;i++) fscanf(f, "%d", &(tablica[i]));
        for (i=0;i<TABSIZE;i++) printf("%d ", (tablica[i]));

        printf("\n------------\n");
        /**/
        
        // Popraw TABSIZE na mniejszą liczbę, odpalaj TABSIZE+1 procesów
        /* Tutaj wstaw wysyłanie liczb do bezpośrednich następników wierzchołka */
	    printf("%d: Wczytuje %d, wysylam\n", rank, min);

        for (i=0;i<TABSIZE;i++) {
            MPI_Send(&(tablica[i]), 1, MPI_INT, 1, SORT, MPI_COMM_WORLD);
        }

        /**/
        /* Zawiadamiamy, że więcej liczb do wysyłania nie będzie*/
        int dummy=-1;
	    MPI_Send( &dummy, 1, MPI_INT, 1, END, MPI_COMM_WORLD); //koniec liczb

        /* Tutaj wstaw odbieranie posortowanych liczb */
        for (i=1;i<size;i++) {
	    //    MPI_Recv( gdzie, ile , jakiego typu, od kogo, z jakim tagiem, MPI_COMM_WORLD, &status);
            MPI_Recv(&(sorted[i-1]), 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }

        /* Wyświetlanie posortowanej tablicy */
        printf("%d: SORTED:\n", rank);
        for (i=0;i<TABSIZE;i++) {
            printf("%d \n",sorted[i]);
        }
        printf("\n");

    } else { //Ani wierzchołek, ani liść
        while (!end) {
            MPI_Recv(&tmp, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG==END) {
                end=1;
                int dummy=-1;
                printf("%d: got END from: %d sending END to %d\n", rank, rank-1, next);
	            MPI_Send( &dummy, 1, MPI_INT, next, END, MPI_COMM_WORLD);
                printf("%d: sending %d to ROOT\n", rank, min);
                MPI_Send( &min, 1, MPI_INT, 0, END, MPI_COMM_WORLD);
            } else {
            //sortowanie babelkowe
                if(!first_num){
                    min = tmp;
                    first_num = 1;
                    printf("%d: first num=%d\n", rank, min);
                    continue;
                } 

                if(tmp < min) {
                    //send min
                    MPI_Send( &min, 1, MPI_INT, next, SORT, MPI_COMM_WORLD);
                    printf("%d: got num %d new min=%d, sending=%d\n", rank, tmp, tmp, min);
                    min = tmp;
                } else {
                    // send tmp
                    MPI_Send( &tmp, 1, MPI_INT, next, SORT, MPI_COMM_WORLD);
                    printf("%d: got num %d sending %d \n", rank, tmp, tmp);
                }
                

            }
        }
    }
   
    MPI_Finalize();
}
