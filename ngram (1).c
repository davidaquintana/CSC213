/*
  David Quintana
  CSC213 - Assignment 1
  Sept. 4, 2023

  Credit:
    *GeeksforGeeks and StackOverflow was used for refrencing parameters and types within C
      - forgot how to use malloc
      - forgot how to use free
      - used to see if EOF can be used for stdin, not just files
    *Previous code was submitted that returns ngram, but did not use fgetc or memory allocation
    *CSC213 assignment page
      - learned that I needed to use fgetc, malloc and free
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** argv) {
  // Make sure the user provided an N parameter on the command line
  if (argc != 2) {
    fprintf(stderr, "Usage: %s N (N must be >= 1)\n", argv[0]);
    exit(1);
  }

  // Convert the N parameter to an integer
  int N = atoi(argv[1]);

  // Make sure N is >= 1
  if (N < 1){
    fprintf(stderr, "Invalid N value %d\n", N);
    exit(1);
  }

  //charcater pointer that will hold the characters read in by fgetc
  //the cast type has been set to a character pointer and the byte-size set to N+1
  char* str = (char *)malloc((N+1));

  //sets the block of memory of str to null with the bytes being set to N+1
  memset(str, '\0', N+1);
  
  // integer used to read in the characters of fgetc
  int c = 0;
  
  // helper used to reitirate our string by n
  int helper = 0; 

  //while loop runs through every charater of the standard input
  while((c = fgetc(stdin)) != EOF)
  {
    //forloop that will initialize the value of str[i] for as many times given by N-1
    for(int i = 0; i < N - 1; i++)
    {
      //printf("i: %c\n", str[i]); -> used to make sure that the forloop and array was working and all variables were incremented
      //printf("i+1: %c\n", str[i+1]);-> used to make sure that the forloop and array was working and all variables were incremented
      
      str[i] = str[i+1];

      //printf("iB: %c\n", str[i]); -> used to make sure that the forloop and array was working and all variables were incremented
      //printf("i+1B: %c\n", str[i+1]); -> used to make sure that the forloop and array was working and all variables were incremented
    }

    //sets the value of str[N-1] to the character being read in by c
    str[N-1] = c;

    if(helper >= N -1)
    //printf("Helper: %d\n", helper); -> used to make sure that the forloop and array was working and all variables were incremented
    {
      //prints the string
      printf("%s", str);

      //prints the newline
      printf("\n");
    }

    //increases our helper so when requirements are meet it will print, or when requirements are not meet the while loop will be restarted
    helper++;
  }

  //frees the memory block that was previously malloced to str
  free(str);

  return 0;
}
