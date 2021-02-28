#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	int totalBytes = 0;     //Contador de bytes copiados
	int readByte = 0;  //Byte de lectura

	if (origin == NULL || destination == NULL) { return -1; }     
	else {
		while ((totalBytes < nBytes) && (readByte = getc(origin)) != EOF) {

			putc(readByte, destination);
			totalBytes++;
		}
	}
    return totalBytes;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file) 
{
	int filenameLength = 0, index = 0;
	char *name;
	char bit;

	if(file == NULL){return NULL;}

	while ((bit = getc(file) != '\0')) {		
		filenameLength++;
		if (bit == 0) {}
	}
	
	name = malloc(sizeof(char) * (filenameLength + 1)); // +1 por el \0 
	fseek(file, -(filenameLength + 1), SEEK_CUR);

	for (index = 0; index < filenameLength + 1; index++) {
		name[index] = getc(file);
	}

	return name;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	int nr_files = 0, index = 0; 
	stHeaderEntry *stHeader = NULL; 

	if(tarFile == NULL){return NULL;}

	//Leer el numero de ficheros
	if (fread(&nr_files, sizeof(int), 1, tarFile) == 0) { 
		return NULL; 
	}

	stHeader = malloc(sizeof(stHeaderEntry)*nr_files); 

	for (index = 0; index < nr_files; index++) {

		if ((stHeader[index].name = loadstr(tarFile)) == NULL) {
			return NULL;
		}
		if(fread(&stHeader[index].size, sizeof(unsigned int), 1, tarFile) == 0){
			return NULL;
		}
		
	}

	(*nFiles) = nr_files; 
	return stHeader;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	FILE * inputFile; // para los archivos
	FILE * outputFile; //archivo tar 

	int copiedBytes = 0, stHeaderBytes = 0, index = 0;
	stHeaderEntry *stHeader; 

	stHeader = malloc(sizeof(stHeaderEntry) * nFiles); 

	//Numero de ficheros + ficheros con nombre y tamaño
	//Guardamos el tamaño del header, que es un entero indicando cuantos
	//ficheros hay mas el tamaño del nombre del archivo + 1 por el 0 mas 
	//el tamaño de ese archivo
	stHeaderBytes += sizeof(int) + nFiles * sizeof(unsigned int);

	for (index = 0; index < nFiles; index++) {
		stHeaderBytes += strlen(fileNames[index]) + 1; // +1 por el \0
	}

	if((outputFile = fopen(tarName, "w")) == NULL){return EXIT_FAILURE;}
	
	//Colocamos el puntero en la zona de data
	if(fseek(outputFile, stHeaderBytes, SEEK_SET)< 0){return EXIT_FAILURE;}

	//Copiamos el contenido de cada fichero
	for (index = 0; index < nFiles; index++) {

		if ((inputFile = fopen(fileNames[index], "r")) == NULL) {
			return (EXIT_FAILURE);
		} 
		copiedBytes = copynFile(inputFile, outputFile, INT_MAX); 

		if (copiedBytes == -1) {
			return EXIT_FAILURE;
		}
		else {
			stHeader[index].size = copiedBytes; 
			stHeader[index].name = malloc(sizeof(fileNames[index]) + 1); 
			strcpy(stHeader[index].name, fileNames[index]); 
		}
		fclose(inputFile); 
	}

	//Colocamos de nuevo el puntero al inicio
	rewind(outputFile);
	//Escribimos el numero de Ficheros
	fwrite(&nFiles, sizeof(int), 1, outputFile); 
	//Rellenamos la cabecera con los datos de cada fichero				
	for (index = 0; index < nFiles; index++) {
		fwrite(stHeader[index].name, strlen(stHeader[index].name) + 1, 1, outputFile); 
		fwrite(&stHeader[index].size, sizeof(unsigned int), 1, outputFile); 															
	}

	free(stHeader);
	fclose(outputFile);

	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 *
 * First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	FILE *tarFile = NULL;
	FILE *destinationFile = NULL;
	stHeaderEntry *stHeader; 
	int nr_files = 0, index = 0, copiedBytes = 0;

	//Abrimos tarfile y leemos cabeceras
	if (((tarFile = fopen(tarName, "r")) == NULL) || ((stHeader = readHeader(tarFile, &nr_files)) == NULL)) {
		return (EXIT_FAILURE);
	}

	for (index = 0; index < nr_files; index++) {

		if ((destinationFile = fopen(stHeader[index].name, "w")) == NULL) { return EXIT_FAILURE; } 
		else {
			copiedBytes = copynFile(tarFile, destinationFile, stHeader[index].size);       
			if (copiedBytes == -1) { return EXIT_FAILURE; }
		} 
		fclose(destinationFile);
	}

	free(stHeader);
	fclose(tarFile); 

	return (EXIT_SUCCESS);
}
