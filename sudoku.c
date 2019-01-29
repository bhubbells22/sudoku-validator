#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdbool.h>

// 2D array for storing the puzzle is read-only global
int sudoku[9][9];

// array for threads to report errors, each thread is mapped
// and can only write to a single index in the array,
// no need to lock.
int valid[27] = {0};

// functions to be passed to each thread 
void* validateRows(void* args);
void* validateCols(void* args);
void* validateGrid(void* args);

int main (int argc, char* argv[])
{
  // put read values into buffer
  int buffer[81];
  int next;
  int idx = 0;
  while (scanf("%d", &next) != -1)
    {
      buffer[idx] = next;
      idx++;
    }
  // read buffer into 2D array
  idx = 0;
  for (int i = 0; i < 9; i++)
    {
      for (int j = 0; j < 9; j++)
	{
	  sudoku[i][j] = buffer[idx];
	  idx++;
	}
    }

  // create threads to handle rows, cols, and subgrids
  long thread_id;
  pthread_t* thread_handles;

  // allocate enough memory for threads
  long thread_count = 27;
  thread_handles = (pthread_t*) malloc(thread_count * sizeof(pthread_t));

  // create 9 threads for row checks
  for (thread_id = 0; thread_id < 9; thread_id++)
    {
      pthread_create(&thread_handles[thread_id], NULL, validateRows, (void *) thread_id);
    }

  // create 9 threads for column checks 
  for (thread_id = 9; thread_id < 18; thread_id++)
    {
      int start = thread_id - 9;
      pthread_create(&thread_handles[thread_id], NULL, validateCols, (void *) start);
    }

  // create 9 threads for subgrid checks 
  for (thread_id = 18; thread_id < 27; thread_id++)
    {
      int start = thread_id - 18;
      pthread_create(&thread_handles[thread_id], NULL, validateGrid, (void *) start);
    }

  // make sure all threads are terminated
  for (int i = 0; i < thread_count; i++)
    pthread_join(thread_handles[i], NULL);

  // free allocated memory
  free(thread_handles);
  
  // check if any threads returned any errors
  bool error = false;
  for (int i = 0; i < 27; i++)
    {
      if (valid[i] == 0)
	error = true;
    }

  // report errors to console
  if (error)
    printf("%s", "The input is not a valid Sudoku.\n");
  else
    printf("%s", "The input is a valid Sudoku.\n");

  return 0;
}

void* validateRows(void* args)
{
  // cast parameter back to integer
  int row = (int) args;

  // array to check each number exists only once in the row
  int check[9] = {0};
  for (int i = 0; i < 9; i++)
    {
      int num = sudoku[row][i];
      // check against array to make sure number isn't a duplicate
      if (num > 9 || num <= 0 || check[num-1] == 1)
	{
	  printf("%s" "%d" "%s", "Row ", row+1, " doesn't have the required values.\n");
          fflush(stdout);
          pthread_exit(NULL);
	}
      else
	check[num-1] = 1;
    }

  // mark the row as valid
  valid[row] = 1;
  pthread_exit(NULL);
}

void* validateCols(void* args)
{
  // cast parameter back to integer
  int col = (int) args;
  
  // array to check each number exists only once in the row 
  int check[9] = {0};
  for (int i = 0; i < 9; i++)
    {
      int num = sudoku[i][col];
      // check against array to make sure number isn't a duplicate
      if (num > 9 || num <= 0 || check[num-1] == 1)
	{
	  printf("%s" "%d" "%s", "Column ", col+1, " doesn't have the required values.\n");
          fflush(stdout);
          pthread_exit(NULL);
	}
      else 
	check[num-1] = 1;
    }
  
  // mark the column as valid
  valid[9+col] = 1;
  pthread_exit(NULL);
}

void* validateGrid(void* args)
{
  // cast parameter back to integer
  int grid = (int) args;
  int row;
  int col;
  int check[9] = {0};
  char* location;

  // each 3x3 grid is numbered from left to right, top to bottom as
  // 0-8, based on that number we can choose the row and column number,
  // and the string for the location to report an error 
  switch (grid)
    {
    case 0:
      row = 0;
      col = 0;
      char string0[] = "top left";
      location = string0;
      break;

    case 1:
      row = 0;
      col = 3;
      char string1[] = "top middle";
      location = string1;
      break;

    case 2:
      row = 0;
      col = 6;
      char string2[] = "top right";
      location = string2;
      break;

    case 3:
      row = 3;
      col = 0;
      char string3[] = "middle left";
      location = string3;
      break;

    case 4:
      row = 3;
      col = 3;
      char string4[] = "middle";
      location = string4;
      break;

    case 5:
      row = 3;
      col = 6;
      char string5[] = "middle right";
      location = string5;
      break;

    case 6:
      row = 6;
      col = 0;
      char string6[] = "bottom left";
      location = string6;
      break;

    case 7:
      row = 6;
      col = 3;
      char string7[] = "bottom middle";
      location = string7;
      break;

    case 8:
      row = 6;
      col = 6;
      char string8[] = "bottom right";
      location = string8;
      break;

    default:
      printf("%s" "%s" "%s", "The ", location, " subgrid doesn't have the required values.\n");
      fflush(stdout);
      pthread_exit(NULL);
    }

  // check the subgrid for duplicate numbers
  int endR = row+3;
  int endC = col+3;
  for (int i = row; i < endR; i++)
    {
      for (int j = col; j < endC; j++)
	{
	  int num = sudoku[i][j];
	  if (num > 9 || num <= 0 || check[num-1] == 1)
	    {
	      printf("%s" "%s" "%s", "The ", location, " subgrid doesn't have the required values.\n");
	      fflush(stdout);
	      pthread_exit(NULL);
	    }
	  else
	    check[num-1] = 1;
	}
    }

  // mark the grid as valid
  valid[grid+18] = 1;
  pthread_exit(NULL);
}
