#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main (int argc, char *argv[])
{
   //const char src[50] = "http://www.tutorialspoint.com";
   //char *dest= (char *)malloc(sizeof(char)*4);
   // int filenumber = 0000;
   // sprintf(dest, "%d", filenumber);
   // char filename[] = "get files/file";
   // strcat(filename, dest);
   // strcat(filename, ".txt");
   // printf("%s\n", filename);

   // printf("Before memcpy dest = %s\n", dest);
   // memcpy(dest, src, strlen(src)+1);
   // printf("After memcpy dest = %s\n", dest);
   // int t = open("file0.txt", "r");
   // printf("%d\n", t);


   // char *dest;
   // int filenumber = 120;
   // sprintf(dest, "%d", filenumber);
   // printf("%s\n", dest);

   char *dest = (char *) malloc(sizeof(char)*strlen(argv[1]));
   strcpy(dest, argv[1]);
   printf("%s\n", dest);


   return(0);
}