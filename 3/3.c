#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

int fileError(char *op, char *path);
void writeFunction();
void readFunction();
void sisError(char * sis);
void fileFormatError();

char *file;
char *op;
int indPar=0;
struct stat fileStat;

int main(int argc, char *argv[])
{
	
	/*
		Se vengono passati meno di 2 o più di 4 argomenti
	*/
	if (argc<3 || argc>4) {
		fprintf( stderr, "Usage %s file mod [digits]\n", argv[0]);
		return 10;
	}		
	
	file =argv[1];
	op =argv[2]; //Operazione
		
	if(argc==4) //Se ho il terzo elemento lo leggo
		indPar= atoi(argv[3]);
	
	/*
	* Se viene dato r
	*/
	if ( strcmp(op, "r") == 0) {
		/*
		* Se vengono passati pochi argomenti dai errore
		*/
		if(argc==3) {
			fprintf( stderr, "If mod r, then also the number of digits for float and double must be given");
			return 30;
		}
		//Con stat prendo il file alla posizione path(file), se non esiste do erroe
		if(stat(file,&fileStat) != 0)
			return fileError(op, file);
		
		readFunction();
	}
	/*
	* Se viene dato un valore diverso da r
	*/
	else {
		/*
		* Se vengono passati troppi argomenti do errore
		*/
		if(argc==4) {
			fprintf( stderr, "If mod is not r, then only the input file must be given");
			return 20;
		}
		writeFunction();
	}
	
	return 0;	
}

void readFunction()
{
	FILE * fd = fopen (file,"r");
	if (fd==NULL)
		exit(fileError(op, file));
	
	int res=1;
	int sel=0;
	int r=0;
	int in=0;
	double dl=0.0;
	float fl=0.0;
	char c=' ';
	
	while(res!=0)
	{
		/*
		 * Leggo il selettore 
		 */
		res=fread(&sel, 1, 1, fd);
		if(res!=1)
		{
			/*
			* Quando finisce il file
			*/
			if (fclose(fd) < 0)  
    		{ 
				sisError("close");

    		}
			return;	
		}
		if (sel<1 || sel>3) //Controllo se il selettore è un numero valido
			fileFormatError();
		/*
		 * Leggo il numero di ripetizioni
		 */
		res=fread(&r, 1, 1, fd);
		
		if(res!=1)
		{
		 	fileFormatError();	
		}
		printf("%d %d\n", sel, r); //selettore ripetizioni --> Stdout
		for(int i=0; i<r; i++){ //Per r volte (numero di righe della struttura)
			switch(sel) { //In base al selettore
				case 1:
				/*
				 * Caso 1:
				 * 1 double 1 float 7 char
				 */
				 //Leggo il double
					res=fread(&dl, sizeof(double), 1, fd);
					if(res!=1)
						fileFormatError();
						
					// Leggo il float
					res=fread(&fl, sizeof(float), 1, fd);
					if(res!=1)
						fileFormatError();
						
					/*float double su stdOut
					 * .* al posto del é si sostituisce il numero di valori dopo la virgola
					 */
					printf("%.*f %.*e", indPar,dl,indPar,fl);
					
					/*
					 * Leggo 7 char
					 */
					for(int j=0; j<7; j++) {
						res=fread(&c, sizeof(char), 1, fd);
						if(res!=1)
							fileFormatError();
						
						printf(" %c", c);
					}
					printf("\n");
					break;
				case 2:
					/*
					 * Caso 2:
					 * 2 interi senza segno, 2 interi con segno, 1 float
					 */
					
					//Leggo i due interi senza segno
					for (int j=0; j<2; j++) {
						res=fread(&in, sizeof(int), 1, fd);
						if(res!=1)
							fileFormatError();
						
						printf("%d ", in);
					}
					
					//Leggo due interi con segno
					for (int j=0; j<2; j++)
					{
						res=fread(&in, sizeof(int), 1, fd);
						if(res!=1)
							fileFormatError();
					
						if(in >= 0)
							printf("+%d ", in);
						else
							printf("%d ", in);
					}

					//Leggo un float
					res=fread(&fl, sizeof(int), 1, fd);
					if(res!=1)
						fileFormatError();
					
					/*
					 * .* -> * si sostituisce con il numero di valori dopo la virgola
					 * e -> formato scientifico
					 */
					printf("%.*e\n",indPar,fl); 
					break;
				case 3:
					/*
					 * Caso 3:
					 * 50 interi con segno
					 */
					
					//Scrivo il primo numero senza spazio davanti, poi gli altri 49
					res=fread(&in, sizeof(int), 1, fd);
					if(res!=1)
						fileFormatError();
					
					if(in >= 0)
						printf("+%d", in);
					else
						printf("%d", in);

					for(int j=0; j<49; j++) {
						
						res=fread(&in, sizeof(int), 1, fd);
						if(res!=1)
							fileFormatError();
						
						if(in >= 0)
							printf(" +%d", in);
						else
							printf(" %d", in);
					}
					printf("\n");
					break;	
			}
		}
		printf("\n");
	}
}

void writeFunction()
{
	FILE *outFd = fopen(file, "w");
	if ( outFd==NULL)
		exit(fileError(op, file));
	int stdinFd=0;

		
	int res=1;
	int sel=0;
	int r=0;
	int in=0;
	double dl=0.0;
	float fl=0.0;
	char c=' ';
	char str[1024];
	
	/*
	 * NOTA
	 * Leggo da stdIn sempre caratteri/stringhe invece di interi/double ecc.
	 * in modo da non dover gestire l'utilizzo di scanf ma fare solo le conversioni
	 */
	
	while(res>0)
	{
		// Leggo il selettore
		res = scanf("%c ", &c);
		
		if(res<1)
		{
			/*
			* Quando finisce il file
			*/
			if(fclose(outFd) != 0)
    		{ 
				sisError("close");

    		}
			return;	
		}
		sel = c -'0'; //selettore -> char to int

		fwrite(&sel, sizeof(unsigned char), 1, outFd); //unsigned char = 1 byte
		
		//Leggo il numero di ripetizioni
		res = scanf("%s\n", str);
		if(res<1)
		{
		 	fileFormatError();	
		}

		r = atoi(str); //ripetizioni -> string to int
		fwrite(&r, sizeof(unsigned char), 1, outFd); //unsigned char = 1 byte
		
		/*
		 * r mi dice quante righe ha la struttura
		 */
		for(int i=0; i<r; i++){
			switch(sel) {
				case 1:
					/*
					 * Caso 1:
					 * 1 double, 1 float, 7 char
					 */
					
					//Leggo il double
	
					res = scanf("%s\n", str);
					if(res<1)
						fileFormatError();
					dl = atof(str); // string to double

					fwrite(&dl, sizeof(double), 1, outFd);
					
					// Leggo il float
					
					res = scanf("%s\n", str);
					if(res<1)
						fileFormatError();
					fl = atof(str); // string to float

					fwrite(&fl, sizeof(float), 1, outFd);
					
					//Leggo 7 char
					for(int j=0; j<7; j++) {

						res = scanf(" %c", &c);
						if(res<1)
							fileFormatError();

						fwrite(&c, sizeof(char), 1, outFd);
					}
					scanf("\n");
					break;
				case 2:
					/*
					 * Caso 2:
					 * 2 interi senza segno, 2 interi con segno, 1 float
					 */
					
					//Leggo tutti e 4 gli interi (non faccio differenza tra con e senza segno
					
					for(int j=0; j<4; j++) {
						res = scanf("%s ", str);
						if(res<1)
							fileFormatError();			
						
						in = atoi(str);

						fwrite(&in, sizeof(int), 1, outFd);
					}
					
					// Leggo il float
					
					res = scanf("%s\n", str);
					if(res<1)
						fileFormatError();
					fl = atof(str); //string to float

					fwrite(&fl, sizeof(float), 1, outFd);
					
					break;
				case 3:
					/*
					*	Caso 3:
					* 	50 interi con il segno
					*/

					for (int j=0; j<50; j++) {
						res = scanf("%s ", str);
						if(res<1)
							fileFormatError();
							
						in = atoi(str); //String to int
						fwrite(&in, sizeof(int), 1, outFd);
					}
					scanf("\n");
					break;
			}
		}
		scanf("\n");
	}
	
	
}

int fileError(char *op, char *path) 
{
	if(strcmp(op, "r")==0)
		fprintf( stderr, "Unable to read from file %s because of %s\n", path, strerror(errno));
	else
		fprintf( stderr, "Unable to write to file %s because of %s\n", path, strerror(errno));
	return 40;
}


void sisError(char * sis)
{
	fprintf( stderr, "System call %s failed because of %s\n", sis, strerror(errno));
    exit(100);
}

void fileFormatError()
{
    exit(50);
}
