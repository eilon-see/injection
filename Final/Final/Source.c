/*This is a skeleton code for Group home assignment written by Shai&Avihai*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define BMP "flake.bmp"
#define BMPCPY "flake-copy.bmp"
#define TXT "defects.txt"
#define BESTFILE "best-route.txt"
#define DBG 'd'
#define DX 0.001

typedef struct { //pixel's coordinates
	int x;
	int y;
}co_t;

typedef struct pixel { //list of pixels
	co_t p;
	struct pixel* next;
}pix_t;

typedef struct flake { //flake's detailed list extructed of bmp
	int size; //the number of pixels that combine the flake
	co_t p1;
	co_t p2;
	pix_t* pArr; //list of flake pixels
	struct flake* next;
}flakeList_t;

typedef struct tflist {//flake's list read from txt file
	co_t p1, p2;
	int size;
	struct tflist* next;
}textfile_t;

typedef struct map_s { // flake in a map context
	textfile_t* flake;
	struct map_s* n1;
	double n1Dist;
	struct map_s* n2;
	double n2Dist;
	int visit;
	struct map_s* route;
	int oil;
	struct map_s* next;
}map_t;

typedef struct route_s { //route
	textfile_t* flake;
	int oil;
	int count;
	struct route_s* next;
}route_t;



//prototype
void option1();
void option2();
void dimensions(FILE * file, unsigned int* width, unsigned int* height); //  FINISHED!      sets the width and hegiht of the bmp file.
unsigned char** readBMP(FILE * file, unsigned int* width, unsigned int* height);			//read bmp file
flakeList_t* buildFlakesList(unsigned char** mat, unsigned int width, unsigned int height);//scan picture for flake pixels to constract flake list from them
int pixelInFlake(int x, int y, pix_t * list);	//	FINISHED! 0 if found, 1 if not      	check if a pixels is in the flake list
pix_t* buildPixelsArr(unsigned char** mat, unsigned int j, unsigned int i, unsigned int width, unsigned int height, pix_t * pixel); //scan bmp matrix for a group of black pixel e.g. flake
pix_t* Pixelinsert(pix_t * list, int x, int y); // insert a new pixel to a current list.
void freeMat(unsigned char** blue, unsigned int width, unsigned height);
void writeFileFlakeProp(flakeList_t * flakes, char* filename);				//write flakes prop to file
int getPixelCount(pix_t * list);	//return the number of pixels in a pixel list
int getFlakesCount(flakeList_t * flake);											//return the number of flake structures in the flake list
flakeList_t* revList(flakeList_t * flake);
void printProperties(flakeList_t * flakeList); //printing to screen first flake.
void freeFlakeList(flakeList_t * flakeList);									//free flakelist
void freelist(pix_t * list); //free a list from a flake.
co_t maxcor(pix_t * list);
flakeList_t* addToList(pix_t * list, flakeList_t * flake);//add a list of pixels that qualify for a flake
int getnumber(char* string); //turn a string to an int
char** StringPointerCreator(char** string, int size);//create a string pointer for option 2
void PointerFreer(char** string, int count, int* size);//freeing string pointer for oprion 2
void sortedListPrinter(char** string, int* sizes, int count);



int main() {
	int choice = 0;
	do {
		printf("--------------------------\nME LAB services\n--------------------------\nMenu:\n");
		printf("1. Scan flakes\n2. Print sorted flake list\n3. Select route\n4. Numeric report.\n5. Students addition\n9. Exit.\nEnter choice: ");
		choice = fgetc(stdin) - '0';
		if (choice + '0' != '\n' && fgetc(stdin) != '\n')
			while ((choice = fgetc(stdin)) != '\n');
		printf("\n");
		switch (choice) {
		case 1:
			option1(); break;
		case 2:
			option2(); break;
		case 9:
			printf("Good bye!\n"); break;
		default:
			printf("Bad input, try again\n\n");
		}
	} while (choice != 9);
	return 0;
}

//wrote Shai - a skeleton example
void option1() {
	unsigned char** mat = NULL;
	unsigned int width = 0, height = 0;
	flakeList_t* flakeList = NULL;
	FILE* file;
	fopen_s(&file, BMP, "rb");
	if (!file) {
		printf_s("Error open the flake.bmp\n");
		return;
	}
	dimensions(file, &width, &height);
	mat = readBMP(file, &width, &height);
	if (!mat) return;
	flakeList = buildFlakesList(mat, width, height);
	//free matrix memory
	freeMat(mat, width, height);
	flakeList = revList(flakeList);
	writeFileFlakeProp(flakeList, TXT);
	printProperties(flakeList);
	freeFlakeList(flakeList);	//free flake list and pixel array in it
	fclose(file);
}

//finds the width and height of the bmp file and sets the file position to the beginning of the array.
void dimensions(FILE* file, unsigned int* width, unsigned int* height) {
	int i, offset = 0;
	fseek(file, 18, SEEK_CUR);
	//width
	for (i = 0; i < 4; i++) {
		*width |= fgetc(file) << (i * 8);
	}
	//height
	for (i = 0; i < 4; i++) {
		*height |= fgetc(file) << (i * 8);
	}
	fseek(file, 2, SEEK_CUR);
	//sets the position.
	for (i = 0; i < 2; i++) {
		offset |= fgetc(file) << (i * 8);
	}
	fseek(file, offset, SEEK_CUR);
}

/*read bmp image to a matrix
@file - bmp file
@width,height - bmp dimention
@return the blue matrix of the image
*/
unsigned char** readBMP(FILE* file, unsigned int* width, unsigned int* height) {
	unsigned char** blue = NULL; //blue matrix of bmp image
	unsigned int pad = 0, i, j, temp[3] = { 0 };
	//caclculating the amount of padding required
	pad = 4 - ((*width * 3) % 4);
	if ((*height > 0) && (*width > 0)) {
		blue = (unsigned char**)malloc(sizeof(unsigned char*) * (*width));
		if (!blue) exit(1);
		for (i = 0;i < (*width);i++) {
			blue[i] = (unsigned char*)malloc(sizeof(unsigned char) * (*height));
			if (!blue[i]) exit(0);
		}
	}
	//reading the bmp map into the "blue" matrix.
	//1 for a black pixel and 2 for any other pixel.
	if ((*height > 0) && (*width > 0)) {
		for (i = 0;i < *height;i++) {
			for (j = 0;j < *width;j++) {
				temp[0] = fgetc(file);
				temp[1] = fgetc(file);
				temp[2] = fgetc(file);
				if ((temp[0] == 0) && (temp[1] == 0) && (temp[2] == 0)) {
					blue[j][i] = 1;
				}
				else blue[j][i] = 0;
			}
			//skipping the padding at the end of every row.
			fseek(file, pad, SEEK_CUR);
		}
	}
	return blue;
}

//scanning matrix for new flakes.
flakeList_t* buildFlakesList(unsigned char** mat, unsigned int width, unsigned int height) {
	unsigned int i, j;
	co_t maxcord;
	flakeList_t* flake = NULL, * tempflake = NULL;
	pix_t* list = NULL;
	for (i = 0;i < height;i++) {
		for (j = 0;j < width;j++) {
			//if mat[j][i] is 1 its black and it is the first pixel in a flake
			if (mat[j][i] == 1) {				
				//making a new flake
				tempflake = (flakeList_t*)malloc(sizeof(flakeList_t));
				if (!tempflake) exit(1);
				// j, i are coordinates 1 of the flake
				tempflake->p1.x = j + 1;
				tempflake->p1.y = i + 1;
				tempflake->pArr = NULL;
				//putting the new flake first in the flake list
				tempflake->next = flake;
				//the new flake is now flake
				flake = tempflake;
				list = NULL;
				//getting a list of all the pixels in the flakes
				list = buildPixelsArr(mat, j, i, width, height, list);
				//adding the list achieved to the flake
				flake = addToList(list, flake);
				//extracting the second coordinate from the list of pixels in the flake
				maxcord = maxcor(flake->pArr);
				flake->p2.x = maxcord.x;
				flake->p2.y = maxcord.y;
				//finding the size of the flake
				flake->size = getPixelCount(flake->pArr);				
			}
		}
	}
	return flake;
}

//checking neighbour pixels and sending to Pixelinsert.
pix_t* buildPixelsArr(unsigned char** mat, unsigned int j, unsigned int i, unsigned int width, unsigned int height, pix_t* list) {
	//turning the pixel that was already checked to 0
	mat[j][i] = 0;
	//checking every neighbour pixel, if its in the matrix, if its black and if its already on the list.
	//if so, it is sent to the buildPixelArr to be added to the pixel list and to have its neighbours checked.
	//the first "if" is to confirm that the pixel has a neighbour and it is not on the edge.
	if ((j + 1 < width) && (mat[j + 1][i] == 1)) {
		if (pixelInFlake(j + 1, i, list)) list = buildPixelsArr(mat, j + 1, i, width, height, list);
	}
	if ((j + 1 < width && i + 1 < height) &&(mat[j+1][i+1]==1)) {		
			if (pixelInFlake(j + 1, i + 1, list)) list = buildPixelsArr(mat, j + 1, i + 1, width, height, list);
	}
	if ((i + 1 < height) && (mat[j][i+1]==1)) {		
			if (pixelInFlake(j, i + 1, list)) list = buildPixelsArr(mat, j, i + 1, width, height, list);
	}
	if (((int)j - 1 >= 0 && i + 1 < height) && (mat[j-1][i+1]==1)) {		
			if (pixelInFlake(j - 1, i + 1, list)) list = buildPixelsArr(mat, j - 1, i + 1, width, height, list);
	}
	if (((int)j - 1 >= 0) && (mat[j-1][i]==1)) {		
			if (pixelInFlake(j - 1, i, list)) list = buildPixelsArr(mat, j - 1, i, width, height, list);
	}
	if (((int)j - 1 >= 0 && (int)i - 1 >= 0) && (mat[j-1][i-1]==1)) {		
			if (pixelInFlake(j - 1, i - 1, list)) list = buildPixelsArr(mat, j - 1, i - 1, width, height, list);
	}
	if (((int)i - 1 >= 0) && (mat[j][i-1]==1)) {		
			if (pixelInFlake(j, i - 1, list)) list = buildPixelsArr(mat, j, i - 1, width, height, list);
	}
	if ((j + 1 < width && (int)i - 1 >= 0) && (mat[j+1][i-1]==1)) {
			if (pixelInFlake(j + 1, i - 1, list)) list = buildPixelsArr(mat, j + 1, i - 1, width, height, list);
	}
	//inserting the pixel to the current flakes list.
	list = Pixelinsert(list, j + 1, i + 1);
	return list;
}

//inserting a pixel to the list of the flake
pix_t* Pixelinsert(pix_t* list, int x, int y) {
	pix_t* pixel = NULL;
	pixel = (pix_t*)malloc(sizeof(pix_t));
	if (!pixel) exit(1);
	pixel->p.x = x;
	pixel->p.y = y;
	pixel->next = NULL;
	if (list == NULL) {
		return pixel;
	}
	pixel->next = list;
	return pixel;
}

//checking if the pixel is part of the flake.
int pixelInFlake(int x, int y, pix_t* list) {
	while (list != NULL) {		
		if (list->p.x == (x + 1) && list->p.y == (y + 1)) return 0;
		list = list->next;		
	}
	return 1;
}

//adding a list of pixels to a flake
flakeList_t* addToList(pix_t* list, flakeList_t* flake) {
	flake->pArr = list;
	return flake;
}

// finding the second coordinate of a flake
co_t maxcor(pix_t* list) {
	co_t cord = { 0 };
	if (list->next != NULL) cord = maxcor(list->next);
	if (list->p.y > cord.y) {
		cord.x = list->p.x;
		cord.y = list->p.y;
		return cord;
	}
	if (list->p.y < cord.y) {
		return cord;
	}
	if (list->p.x > cord.x) {
		cord.x = list->p.x;
		cord.y = list->p.y;
		return cord;
	}
	return cord;
}

//freeing matrix
void freeMat(unsigned char** blue, unsigned int width, unsigned height) {
	unsigned int i;
	if (!blue) return;
	for (i = 0;i < width;i++) {
		free(blue[i]);
	}
	free(blue);
}

//printing first flake to screen.
void writeFileFlakeProp(flakeList_t* flakes, char* filename) {
	FILE* file = NULL;
	flakeList_t* flake = NULL;
	flake = flakes;
	fopen_s(&file, filename, "wt");
	if (!file) return;
	fprintf_s(file, "Coordinate1	Coordinate2	Size\n");
	fprintf_s(file, "===========	===========	====\n");
	while (flake != NULL) {
		fprintf_s(file, "(%d,%d)   	(%d,%d)   	%d\n", flake->p1.x, flake->p1.y, flake->p2.x, flake->p2.y, flake->size);
		flake = flake->next;
	}
	fclose(file);
}

//return the number of pixels in a pixel list
int getPixelCount(pix_t* list) {
	int count = 0;
	while (list != NULL) {
		count++;
		list = list->next;
	}
	return count;
}

//counts the number of flakes
int getFlakesCount(flakeList_t* flake) {
	int count = 0;
	while (flake != NULL) {
		count++;
		flake = flake->next;
	}
	return count;
}

//reverse the order of the flakes list
flakeList_t* revList(flakeList_t* flake) {
	flakeList_t* next , * current = flake, * prev = NULL;	
	while (current != NULL) {
		next = current->next;
		current->next = prev;
		prev = current;
		current = next;
	}
	flake = prev;
	return flake;
}

//print the flake properties to the screen
void printProperties(flakeList_t* flakeList) {
	printf_s("Coordinate x1,y1 of the first discoverd flake (%d,%d)\n", flakeList->p1.x, flakeList->p1.y);
	printf_s("Size %d\n", flakeList->size);
	printf_s("Total of %d flakes.\n", getFlakesCount(flakeList));
}

//free a list of pixels from a flake.
void freelist(pix_t* list) {
	if (list->next != NULL) {
		freelist(list->next);
	}
	free(list);
}

//free flake list
void freeFlakeList(flakeList_t* flakeList) {
	if (flakeList->next != NULL) {
		freeFlakeList(flakeList->next);
	}
	if (flakeList->pArr != NULL) {
		freelist(flakeList->pArr);
	}
	free(flakeList);
}

//sorted flakes list
void option2() {
	FILE* file;
	char c[101], ** string = NULL;
	int count=0, i, * sizes;
	fopen_s(&file, TXT, "rt");
	if (!file) {
		printf_s("Problem reading defects.txt file\nList is empty\n");
		return;
	}
	while (fgets(c, 100, file)) {
		count++;
	}
	count -= 2;
	string = StringPointerCreator(string, count);
	fseek(file, 0, SEEK_SET);
	printf_s("Sorted flakes by size :\n");
	for (i = 0;i < 2;i++) {
		fgets(c, 100, file);
		printf_s("%s", c);
	}
	for (i = 0; i < count;i++) {
		fgets(string[i], 100, file);
	}
	if (count==0) exit(1);
	sizes = (int*)malloc(sizeof(int) * count);
	if (!sizes) exit(1);
	//extracting size of flakes and printing sorted list
	sortedListPrinter(string, sizes, count);
	PointerFreer(string, count, sizes);
}

//extracting size of flakes and printing sorted list
void sortedListPrinter(char** string, int* sizes, int count) {
	int i = 0, j = 0, order = 0, max = 0;
	for (i = 0; i < count; i++) {
		j = strlen(string[i]) - 1;
		while (string[i][j] != 9) j--;
		j++;
		sizes[i] = getnumber(&string[i][j]);
	}
	for (j = 0;j < count;j++) {
		for (i = 0; i < count;i++) {
			if (max > sizes[i]);
			else {
				max = sizes[i];
				order = i;
			}
		}
		printf_s("%s", string[order]);
		sizes[order] = 0;
		max = 0;
	}
}

//converts a string to an integer
int getnumber(char* string) {
	int i, j, count = 0, pow = 1, num=0;
	for (i = strlen(string)-2; i >= 0;i--) {
		if (count == 0) pow = 1;
		for (j = 0;j < count;j++) {
			pow *= 10;
		}
		num += ((string[i] - 48) * pow);
		count++;
	}
	return num;
}

//create a string pointer for option 2
char** StringPointerCreator(char** string, int size) {
	int i = 0;
	if (size == 0) exit(1);
	string = (char**)malloc(sizeof(char*) * size);
	if (!string) exit(1);
	for (i = 0;i < size; i++) {
		string[i] = (char*)malloc(sizeof(char) * 30);
		if (!string[i]) exit(1);
	}
	return string;
}

//freeing pointers of oprion 2
void PointerFreer(char** string, int count, int* size) {
	int i = 0;
	for (i = 0;i < count;i++) {
		free(string[i]);
	}
	free(string);
	free(size);
}