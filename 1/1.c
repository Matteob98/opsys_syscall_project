#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>

void ricerca(char *path, int depth, int maxdepth); // Funzione ricorsiva per la ricerca dei file/dr
void substr(char* str, char* sub, int start, int len); //Ritorna una sottostringa di str
char *lsperms(int mode); //Per ottenere i permessi in formato stringa
int filetypeletter(int mode); //Usata da lsperms
char* MyReadLink(char* path); //Legge il path del file puntato da un link simbolico

int dirNotFound=0; //Numero di dir non trovate
char *maxdepth=NULL, *type=NULL, *size=NULL, *perma=NULL;
int isPrint=1; //Se 1 faccio il print, se 0 faccio ls
char linkPath[PATH_MAX]; //path usato per i link simbolici

int main(int argc, char *argv[])
{
	int nrFile=0; //Incrementata ogni volta che trovo un file passato in input
	int isFile=0; //0 se cerco file, 1 se sono argomenti
	
	//Per tutti gli argomenti passati in input
	for (int i=1; i<argc; i++) {
		
		//Finchè non inizia con - cerco i file
		if (isFile==0 && argv[i][0]!='-') {
			nrFile++;
		}
		else {
			isFile=1;
			if (strcmp(argv[i], "-maxdepth") == 0) {
				i++;
				maxdepth=argv[i];
			}
			if (strcmp(argv[i], "-type") == 0) {
				i++;
				type=argv[i];
			}
			if (strcmp(argv[i], "-size") == 0) {
				i++;
				size=argv[i];
			}
			if (strcmp(argv[i], "-perma") == 0) {
				i++;
				perma=argv[i];
			}
			if (strcmp(argv[i], "-ls") == 0)
				isPrint=0;
			if (strcmp(argv[i], "-print") == 0)
				isPrint=1;
			
		}
	}
	
	/*
	Se il numero di file è uguale a 0 lancio un errore
	*/
	if(nrFile==0) {
		fprintf( stderr, "Usage %s dirs [-maxdepth N] [tests] [actions]\n", argv[0]);
		return 20;
	}
	
	//Per ogni file lancio la ricerca
	for(int i=1; i<=nrFile; i++)
		ricerca(argv[i], 1, (maxdepth!=NULL ? atoi(maxdepth) : -1)); //Se maxdepth è stato specificato lo converto in intero, altrimenti passo -1
	
	return dirNotFound;
}

void ricerca(char * path, int depth, int maxdepth) 
{
	DIR * d; //Variabile che mi servierà per lavorare sulle directory
	
	
	/*
	Caso base 1 -> Non esiste
	*/
	struct stat fileStat;
	//Con lstat prendo il file alla posizione path, se trovo un link simbolico prendo il link e non il file a cui è riferito (come stat)
	if(lstat(path,&fileStat) < 0) {
		dirNotFound++;
		fprintf( stderr, "Unable to open %s because of %s\n", path, strerror(errno));
		return; //esco dalla ricersione
	}
	
	/*
	Passo ricorsivo -> E' una directory
	*/
	d = opendir(path);
	struct dirent *dir;
	
	//Se è una directory faccio la ricorsione	
	if (d) {
		if (maxdepth==-1 || depth<=maxdepth )
			while ((dir = readdir(d)) != NULL) { //finchè posso leggere nella directory
				if (strcmp (dir->d_name, "..") != 0 && strcmp (dir->d_name, ".") != 0) { //se non sono file nascosti o che portano indietro "..", "."
					char newPath[PATH_MAX];
					snprintf(newPath, sizeof(newPath), "%s/%s", path, dir->d_name); //prendo il nuovo path (figlio)
					ricerca(newPath, depth+1, maxdepth); // vecchio path + / + nuovoPath
				}
			}
		
		closedir(d);
		
	}
	
	/*
	Caso base 2-> Faccio il print o ls
	*/
	
	//Se 1 va stampato, se 0 no
	int isPrintable=0;
	
	if (type!=NULL) {
		if (strcmp(type,"d")==0 && (S_ISDIR(fileStat.st_mode))) //se cerco le directory ed è una directory
			isPrintable=1;
		if (strcmp(type,"f")==0 && (S_ISREG(fileStat.st_mode)) && ( ! (S_ISLNK(fileStat.st_mode)) ) ) //Se type=f allora devo cercare solo i file regolari (ed escludere i link)
			isPrintable=1;
		
		if (strcmp(type,"l")==0 && (S_ISLNK(fileStat.st_mode))) //Se type=l allora devo cercare solo i soft link
			isPrintable=1;

	}

	if(size!=NULL) {
		
		char N[sizeof(size)-1]; //Prendo il prefisso +/-
		substr(size, N, 1, sizeof(size)-1); //Prendo il suffisso (il numero)
		int dim = fileStat.st_size;
		
		/*
		Se è stato selezionato + e la dimensione è maggiore o uguale a N
		Altrimenti se è stato selezionato - e la dimensione è minore o uguale a N
		*/
		if (&size[0] == "+") {
			if (dim>=atoi(N))
				isPrintable=1;
		}
		else if (dim<=atoi(N))
			isPrintable=1;

	}
	if(perma!=NULL) {
		
		char permessi[5];
		//Prendo i permessi in formato 4 bit (or logico dei permessi)
		sprintf(permessi, "%o",(fileStat.st_mode & ( S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO)));
		
		if ( strcmp(permessi,perma)==0)
			isPrintable=1;
		
		
	}
	
	//Se non ho specificato test, stampa sempre
	if (type==NULL && size==NULL && perma==NULL)
		isPrintable=1;
		
	
	/*
		Chiamo print o ls
	*/
	if (isPrintable==1 && isPrint==1) {

		printf("%s\n",path); //print
	}
	else if (isPrintable==1 ) {
		/*
		Implemento ls
		*/
		
		printf("%s", lsperms(fileStat.st_mode)); //Chiama la funzione che scrive i permessi
		printf( "\t%d", fileStat.st_nlink); //Numero di hard link
		printf( "\t%d", fileStat.st_size); //Dimensione del file
		printf("\t%s",path);
		
		if (S_ISLNK(fileStat.st_mode))  //Se è un soft link 
			printf(" -> %s", MyReadLink(path)); //Chiamo la funzione che mi ritorna il file a cui punta
		
		printf("\n");
		
	}
	

}

void substr(char* str, char* sub, int start, int len) {
	memcpy(sub, &str[start], len);
	sub[len] = '\0';
}


char *lsperms(int mode)
{
       static const char *rwx[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
       static char bits[11];

       bits[0] = filetypeletter(mode);
       strcpy(&bits[1], rwx[(mode >> 6)& 7]);
       strcpy(&bits[4], rwx[(mode >> 3)& 7]);
       strcpy(&bits[7], rwx[(mode & 7)]);
       if (mode & S_ISUID)
           bits[3] = (mode & S_IXUSR) ? 's' : 'S';
       if (mode & S_ISGID)
           bits[6] = (mode & S_IXGRP) ? 's' : 'S';
       if (mode & S_ISVTX)
           bits[9] = (mode & S_IXOTH) ? 't' : 'T';
       bits[10] = '\0';
       return(bits);
}

/*
Si occupa di scrivere il primo bit dei permessi
Relativo al tipo di file
*/
int filetypeletter(int mode)
{
       char    c;

       if (S_ISREG(mode))
           c = '-';
       else if (S_ISDIR(mode))
           c = 'd';
       else if (S_ISBLK(mode))
           c = 'b';
       else if (S_ISCHR(mode))
           c = 'c';

       else if (S_ISFIFO(mode))
           c = 'p';

       else if (S_ISLNK(mode))
           c = 'l';

       else if (S_ISSOCK(mode))
           c = 's';

       else
       {
           c = '?';
       }
       return(c);
}

/*
Legge il file a cui punta il soft link
*/
char* MyReadLink(char* path) {
    ssize_t len = readlink(path , linkPath, sizeof(linkPath)-1);
    if (len != -1) {
      linkPath[len] = '\0';
    }
	else   {
		fprintf( stderr, "System call %s failed because of %s\n", "readlink", strerror(errno));
		exit(100);
	}
    return linkPath;
}
