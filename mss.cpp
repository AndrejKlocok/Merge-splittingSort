/**
 * Autor:   Andrej Klocok, xkloco00
 * Projekt: Merge-splitting sort
 * Subor:   mss.cpp
 */

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
/** 
 * Tag reprezentuje odosielanu hodnotu ako integer, ktory predstavuje velkost
 * nasledne odosielaneho pola hodnot. 
 */
#define SIZE_VEC 0
/**
 * Tag reprezentuje odosielanu hodnotu ako pole integerov, ktore obsahuje
 * ciselne hodnoty o velkosti, ktora bola prijata v predchadzajucej sprave.
 */
#define VECTOR 1
/**
 * Tag reprezentuje odosielanu hodnotu ako integer, ktory predstavuje pociatocne
 * rozhodenie cisel medzi procesory.
 */
#define INIT 2

using namespace std;

/**
 * Funkciu pouziva procesor s rank 0. Sluzi na nacitanie suboru "numbers" s nahodnymi cislami 0-255
 * a rozposlanie cisel procesorom. Na konci sa poslem -1 ako symbol ukoncenia samotneho posielania. 
 * 
 * @param processorsCount pocet procesorov v MPI_COMM_WORLD
 */
void readNumbers( int processorsCount, bool timeCheck){
    int number;

    fstream file("numbers");
    int stop = -1;
    int pCount = 0;
    
    if(!file.is_open()){
        cerr << "Cant open file numbers"<<endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    //precitaj prve
    number = file.get();
    if(file.good()){
        if(!timeCheck)
            printf("%d", number);
        MPI_Send(&number, 1, MPI_INT, pCount, INIT, MPI_COMM_WORLD);
        pCount++;
    }
    /* Spracovavanie vstupneho suboru */
    while(file.good()){
        number=file.get();
        /* Necitaj posledny */
        if(!file.good())
            break;
        if(!timeCheck)
            printf(" %d", number);        
        if(pCount>=processorsCount){
            pCount=0;
        }
        MPI_Send(&number, 1, MPI_INT, pCount, INIT, MPI_COMM_WORLD);
        pCount++;
    }
    file.close();
    
    /* Koniec nacitania*/
    for(int i=0; i< processorsCount; i++)
        MPI_Send(&stop, 1, MPI_INT, i, INIT, MPI_COMM_WORLD);

    printf("\n");
}
/**
 * Funkcia prechadza podpole urcene indexami left, middle, right a na zaklade pozicii indexov
 * a hodnot vo vstupnom poli usporiaduva pole do pomocneho pola temp
 * 
 * @param vec       vstupne pole cisel
 * @param temp      pomocne pole
 * @param left      lavy index
 * @param middle    index stredu podpola
 * @param right     pravy index podpola, hranica
 */
void merge(vector<int> *vec, vector<int> *temp, int left, int middle, int right){
    int l = left;   // lavy index pomocou, ktoreho prechadzame podpole
    int m = middle; // index, ktorym prechadzame pravu stranu pola
    // cez podpole ohranicene left a right
    for (int i = left; i < right; i++){
        // rozdelenie podpola na polovice pomocou middle indexu
        if(l < middle)
            //presiel som za koniec podpola alebo je hodnota na lavom inexe mensia 
            // ako hodnota za stredom podpola na indexe m
            if(m >= right || vec->at(l) <= vec->at(m))
                // zapis hodnotu na indexe l do temp na poziciu i
                // inkrementuj index l
                temp->at(i) = vec->at(l++);
            else
                // zapis hodnotu na indexe m do temp na poziciu i
                // inkrementuj m
                temp->at(i) = vec->at(m++);
        // lavim indexom som presiel polovicu, zapisujem pravu polovicu pola
        else
            temp->at(i) = vec->at(m++);
    }
}
/**
 * Funkcia implementuje nerekurzivny Merge sort, ktory zoradi postupnost vo vstupnom vektore.
 * 
 * @param vec vstupny vektor cisel procesora
 */
void bottomUpMergeSort(vector<int> *vec){
    int n = vec->size(); // cielove pole hodnot
    vector<int> temp; // pomocny vektor
    temp.resize(n);
    
    //kazdy raz zvacsi 2x velkost podpola, log N 
    for(int sz = 1; sz<n; sz*=2){ 
        //zoradime jednotlive podpolia N
        for( int i=0; i< n; i+= 2*sz)
            //vstupne pole, pomocne pole, index podpola: pociatocny, stred, konecny
            merge(vec, &temp, i, min(i+sz,n), min(i+2*sz,n));        
        
        //kopirovanie zotriedenych podpoli, N
        for (int i = 0; i<temp.size(); i++){
            vec->at(i) = temp.at(i);
        }      
    }
}
/**
 * Funkcia vytlaci na stdout zoradene postupnosti kazdeho procesora.
 * 
 * @param processorsCount   postupnost, ktoru vlastni hlavny procesor 0
 * @param processorQ        postupnost, ktoru vlastnia ostatne procesory
 */
void finalPrint(int processorsCount, vector<int> *processorQ){
    int nSize;
    vector<int> nProcessorQ;
    MPI_Status status;
    // co mam ja
    for(int i=0; i < processorQ->size(); i++)
                printf("%d\n", processorQ->at(i));
    // co maju ostatny
    for (int p=1; p< processorsCount; p++){
        MPI_Recv(&nSize, 1, MPI_INT, p, SIZE_VEC, MPI_COMM_WORLD, &status);
        nProcessorQ.resize(nSize);
        MPI_Recv(&nProcessorQ[0], nSize, MPI_INT, p, VECTOR, MPI_COMM_WORLD, &status);
        for(int i=0; i < nProcessorQ.size(); i++)
            printf("%d\n", nProcessorQ.at(i));          
        } 
}
/**
 * Funkcia prijme postupnost procesora P_i, spoji dve zoradene postupnosti S_i a S_i+1 procesorov P_i a P_i+1 do jednej
 * zoradenej postupnosti merged. Procesoru P_i odosle prvu cast merged o velkosti [S_i], seba si neha zbytok. 
 *  
 * @param rank          jedinecny identifikator procesu, ktory vola funkciu mergeSplit
 * @param processorQ    pole hodnot procesora rank 
 * @return              zbytok pola merged po odoslani procesoru P_i
 */
vector<int> mergeSplit(int rank, vector<int> processorQ){
    int nSize;
    vector<int> nProcessorQ, mergedQ;
    MPI_Status status;
    
    MPI_Recv(&nSize, 1, MPI_INT, rank-1, SIZE_VEC, MPI_COMM_WORLD, &status);
    nProcessorQ.resize(nSize);
    MPI_Recv(&nProcessorQ[0], nSize, MPI_INT, rank-1, VECTOR, MPI_COMM_WORLD, &status);  
            
    //spojim zoradenu postupnost
    mergedQ.resize(processorQ.size() + nProcessorQ.size());
    std::merge(processorQ.begin(), processorQ.end(), nProcessorQ.begin(), nProcessorQ.end(), mergedQ.begin());
            
    nProcessorQ.clear();
    //naspat poslem mensie
    nProcessorQ = vector<int>(mergedQ.begin(), mergedQ.begin() + nSize);
    MPI_Send(&nSize, 1, MPI_INT, rank-1, SIZE_VEC, MPI_COMM_WORLD);
    MPI_Send(&nProcessorQ[0], nSize, MPI_INT, rank-1, VECTOR, MPI_COMM_WORLD);
    //zmazem a neham si vacsie
    mergedQ.erase(mergedQ.begin(), mergedQ.begin() + nSize);
    return mergedQ; 
}

int main(int argc, char* argv[]) {

    int processorsCount;        // pocet vsetkych procesorov
    int rank;                   // rank procesora
    int vecSize;                // velkost vektora procesora
    vector<int> processorQ;     // vektor hodnot-> pole hodnot
    int number;                 // hodnota z pola
    MPI_Status status;          // status
    bool timeCheck = false;     // meranie casovej narocnosti
    
    if( (argc>1) && (strcmp(argv[1], "-time") == 0) ){
        timeCheck = true;
    }
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&processorsCount);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    
    
    /* Nacitanie suboru s cislami */
    if(rank == 0){
        readNumbers(processorsCount, timeCheck);      
    }
    // Prijem cisel od hlavneho proc
    while(1){
        MPI_Recv(&number, 1, MPI_INT, 0, INIT, MPI_COMM_WORLD, &status);
        //Konecne volanie
        if(number==-1)
            break;
        processorQ.push_back(number);
    }
    vecSize = processorQ.size();
    // meranie casovej zlozitosti
    double timeStart, timeEnd;
    if(rank == 0 && timeCheck){
        timeStart = MPI_Wtime();
    }
    //procesor si zoradi svoje pole sekvencnym algoritmom
    bottomUpMergeSort(&processorQ);
    
    //limity na indexy
    int evenLimit = 2*((processorsCount-1)/2);
    int oddLimit  = 2*(processorsCount/2)-1;
        
    for(int k=0; k<=processorsCount/2; k++){
        //neparne procesory
        if((rank % 2) && (rank < evenLimit)){
            //neparne poslu svoje
            MPI_Send(&vecSize, 1, MPI_INT, rank+1, SIZE_VEC, MPI_COMM_WORLD);
            MPI_Send(&processorQ[0], vecSize, MPI_INT, rank+1, VECTOR, MPI_COMM_WORLD);
            
            // prijmem pole obsahujuce mensie cisla
            MPI_Recv(&vecSize, 1, MPI_INT, rank+1, SIZE_VEC, MPI_COMM_WORLD, &status);
            processorQ.resize(vecSize);
            MPI_Recv(&processorQ[0], vecSize, MPI_INT, rank+1, VECTOR, MPI_COMM_WORLD, &status);          
            
        }else if( (rank <= evenLimit)&& (rank != 0)){
            //parne prijmu susedove
            processorQ = mergeSplit(rank, processorQ);       
        }    
        //parne procesory
        if( (!(rank % 2) || rank == 0) && rank < oddLimit){
            //posli svoje
            MPI_Send(&vecSize, 1, MPI_INT, rank+1, SIZE_VEC, MPI_COMM_WORLD);
            MPI_Send(&processorQ[0], vecSize, MPI_INT, rank+1, VECTOR, MPI_COMM_WORLD);
            
            // prijmem pole obsahujuce mensie cisla
            MPI_Recv(&vecSize, 1, MPI_INT, rank+1, SIZE_VEC, MPI_COMM_WORLD, &status);
            processorQ.resize(vecSize);
            MPI_Recv(&processorQ[0], vecSize, MPI_INT, rank+1, VECTOR, MPI_COMM_WORLD, &status);          
            
        }else if( (rank <= oddLimit)){
            //neparne prijmu susedove
            processorQ = mergeSplit(rank, processorQ);            
        }
    }
    // vypisem co maju procesory
    if(rank == 0){
        if(timeCheck){
            timeEnd = MPI_Wtime();
            cout << "Time:" << (timeEnd-timeStart) << endl;   
        }
        else {
            finalPrint(processorsCount, &processorQ);
        }
    }else{
        // poslem prvemu co mam
        if(!timeCheck){
            MPI_Send(&vecSize, 1, MPI_INT, 0, SIZE_VEC, MPI_COMM_WORLD);
            MPI_Send(&processorQ[0], vecSize, MPI_INT, 0, VECTOR, MPI_COMM_WORLD);   
        }
    }
   MPI_Finalize(); 
    return 0;
}

