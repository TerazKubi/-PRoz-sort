#include <mpi.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#define SORT 1
#define END 2
// #define TABSIZE 20

// Kompilujemy mpic++ mysort.c -0 sort.out
// Uruchamiamy mpirun -oversubscribe -n N sort.out liczby.txt
// gdzie N to liczba procesów

void swap(int* xp, int* yp){
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}
 
void bubbleSort(int arr[], int n){
    for (int i = 0; i < n - 1; i++){
        for (int j = 0; j < n - i - 1; j++){
            if (arr[j] > arr[j + 1])
                swap(&arr[j], &arr[j + 1]);
        }
    }
}
void printArray(int arr[], int size)
{
    for (int i = 0; i < size; i++)
        printf("%d ", arr[i]);
}
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int size,rank;
    int i;
    int end=0;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int tsize = 10; //ile liczb przechowuje kazdy proces
    int TABSIZE = tsize * (size-1); // ilosc sortowanych liczb = ilosc procesow * tsize

    int tablica[TABSIZE]={0};
    int sorted[TABSIZE]={-1};

    if (argc<2) {
	    MPI_Finalize();
	    printf("Za malo argumentow\n");
	    exit(0);
    }
    /*
        Szkielet dzieli procesy na dwie kategorie: wierzchołek, liście
    */

    int next = rank+1;
    if(next > size-1) next = size-1;

    
    int tab[tsize] = {0};

    if (rank == 0) {
        /* Wczytywanie liczb do posortowania z pliku podanego jako pierwszy argument */
        // printf("tsize: %d \n", tsize);
        printf("Otwieram plik\n");
	    FILE *f;int i;
	    f = fopen(argv[1],"r");
	    for (i=0;i<TABSIZE;i++) fscanf(f, "%d", &(tablica[i]));
	    printArray(tablica, TABSIZE);
        fclose(f);
        printf("\n------------\n");
        /**/
        
	    //wyslanie podzielonej tablica do procesow
        for (i=0;i<TABSIZE;i++) {
            tab[i%tsize] = tablica[i];

            if (i%tsize == tsize-1)
                MPI_Send(tab, tsize, MPI_INT, 1, SORT, MPI_COMM_WORLD);
            
        }
        
        /* Zawiadamiamy, że więcej liczb do wysyłania nie będzie*/
        int dummy=-1;
	    MPI_Send( &dummy, 1, MPI_INT, 1, END, MPI_COMM_WORLD); //koniec liczb

        /* Tutaj wstaw odbieranie posortowanych liczb */
        FILE *results;
        results = fopen("wyniki_sortowania.txt", "w");
        for (i=1;i<size;i++) {
            MPI_Recv(&tab, tsize, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            for(int j=0;j<tsize;j++) fprintf(results, "%d\n", tab[j]);           
        }
        printf("\n");
        fclose(results);

        /* Wyświetlanie posortowanej tablicy */
        //for (i=0;i<TABSIZE;i++) {
        //    printf("%d ",sorted[i]);
       // }
        

    } else { //Ani wierzchołek, ani liść
        int buff[tsize] = {0};
        int first = 1;
        while (!end) {
            MPI_Recv(&buff, tsize, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG==END) {
                // printf("%d: ending...\n",rank);
                end=1;
                int dummy=-1;
                MPI_Send(&dummy, 1, MPI_INT, next, END, MPI_COMM_WORLD);
                MPI_Send(tab, tsize, MPI_INT, 0, END, MPI_COMM_WORLD);
                break;
		    
            }

            bubbleSort(buff, tsize);
            
            if(first){
                first=0;
                for(int i=0; i<tsize;i++) tab[i] = buff[i];
                // printf("%d: Got first array= ", rank);
                // printArray(tab,tsize);
                // printf("\n");
                continue;
            }


            if(tab[tsize-1] < buff[0]){
                MPI_Send( &buff, tsize, MPI_INT, next, SORT, MPI_COMM_WORLD);

            } 
            else {
                int tmp[tsize*2]={0};
                for(int i=0;i<tsize;i++){
                    tmp[i] = tab[i];
                    tmp[tsize+i] = buff[i];
                }
                bubbleSort(tmp, tsize*2);

                for(int i=0;i<tsize;i++){
                    tab[i] = tmp[i];
                    buff[i] = tmp[tsize+i];
                }
                MPI_Send( &buff, tsize, MPI_INT, next, SORT, MPI_COMM_WORLD);

            }   
	    }
    }
    MPI_Finalize();
}
