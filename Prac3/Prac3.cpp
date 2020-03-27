//==============================================================================
// Copyright (C) John-Philip Taylor
// tyljoh010@myuct.ac.za
//
// This file is part of the EEE4084F Course
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//
// This is an adaptition of The "Hello World" example avaiable from
// https://en.wikipedia.org/wiki/Message_Passing_Interface#Example_program
//==============================================================================


/** \mainpage Prac3 Main Page
 *
 * \section intro_sec Introduction
 *
 * The purpose of Prac3 is to learn some basics of MPI coding.
 *
 * Look under the Files tab above to see documentation for particular files
 * in this project that have Doxygen comments.
 */



//---------- STUDENT NUMBERS --------------------------------------------------
//
// NYCMIC002; JANJOH021
//
//-----------------------------------------------------------------------------

/* Note that Doxygen comments are used in this file. */
/** \file Prac3
 *  Prac3 - MPI Main Module
 *  The purpose of this prac is to get a basic introduction to using
 *  the MPI libraries for prallel or cluster-based programming.
 */

// Includes needed for the program
#include "Prac3.h"

/** This is the master node function, describing the operations
    that the master will be doing */
void Master () {
 //! <h3>Local vars</h3>
 // The above outputs a heading to doxygen function entry
 int  j;             //! j: Loop counter
 char buff[BUFSIZE]; //! buff: Buffer for transferring message data
 MPI_Status stat;    //! stat: Status of the MPI application

 // Read the input image
 if(!Input.Read("Data/greatwall.jpg")){
  printf("Cannot read image\n");
  return;
 }

 // Allocated RAM for the output image
 if(!Output.Allocate(Input.Width, Input.Height, Input.Components)) return;


/*
 // This is example code of how to copy image files ----------------------------
 printf("Start of example code...\n");
 for(j = 0; j < 10; j++){
  tic();
  int x, y;
  for(y = 0; y < Input.Height; y++){
   for(x = 0; x < Input.Width*Input.Components; x++){
    Output.Rows[y][x] = Input.Rows[y][x];
   }
  }
  printf("Time = %lg ms\n", (double)toc()/1e-3);
 }
 

 printf("End of example code...\n\n");
 */
 // End of example -------------------------------------------------------------

//Initial code to send out data size
//JPEG stores images in a NxM array where the rows or 'scanlines' store RGB data 
//eg RGBRGBRGB, therefore row size or width = 3*image width
//Data type of scanlines is unsigned char 
//Partition by spliting data into 4 sets of rows

//Step 1: Determine the size of data sample 
int bufferSize = Input.Width*Input.Components;
int RowNum = Input.Height;
int rowPerSlave = floor(RowNum/(numprocs-1));

printf("DEBUG1a: Rowsize is: %d \n", (int)RowNum); //Debug statements referenced for easy debuggging
printf("DEBUG1b: Rows per slave is: %d \n", (int)rowPerSlave); //Debug statements referenced for easy debuggging
printf("DEBUG2: Buffer Size is: %d \n", (int)bufferSize);//''

//Step 2: Send Rowsize to each slave


//For loop to send data size to each slave 
for (size_t j = 1; j < numprocs ; j++) //note j<numprocs as master counts as a process
{
    MPI_Send(&bufferSize, 1, MPI_INT, j, TAG, MPI_COMM_WORLD); //Send bufferSize to each slave  
    MPI_Send(&RowNum, 1, MPI_INT, j, TAG+1, MPI_COMM_WORLD); //Convention to increment tag by 1 in order to differentiate each process
    MPI_Send(&rowPerSlave, 1, MPI_INT, j, TAG+2, MPI_COMM_WORLD);
}

/* Important information
JSAMPLE** Rows; // Points to an array of pointers to the
                  // beginning of each row in the image buffer.
                  //
                  // Use to access the image buffer in a row-wise
                  // fashion, with:
                  // Rows[row][num_components*column + component]
*/

//Step 3: Send data to each slave 
 
for (size_t j = 0; j <numprocs; j++)
{
    for (size_t i = 0;  i<=rowPerSlave; i++) //break messages sent to slaves in blocks of rowPerSlave rows and iterate over 
    {
        if((i+j)<RowNum)
        {
        MPI_Send(&Input.Rows[j*rowPerSlave+i][0], bufferSize, MPI_CHAR, j, TAG+4, MPI_COMM_WORLD); 
        //send pointer to each row to slaves
        }
    }
}


//Step 4: Receive data from each slave
//Step 5: Collect and order data 

 // Write the output image
 if(!Output.Write("Data/Output.jpg")){
  printf("Cannot write image\n");
  return;
 }
 //! <h3>Output</h3> The file Output.jpg will be created on success to save
 //! the processed output.
}

//------------------------------------------------------------------------------

/** This is the Slave function, the workers of this MPI application. */
void Slave(int ID){
 // Start of "Hello World" example..............................................
 char idstr[32];
 char buff [BUFSIZE];
 int bufferSize;
 int RowNum;
 int rowPerSlave;
 MPI_Status stat;
 int j; //loop counter and rank


 //Receive size information 
 MPI_Recv(&bufferSize, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
 MPI_Recv(&rowPerSlave, 1, MPI_INT, 0, TAG+1, MPI_COMM_WORLD, &stat);
 MPI_Recv(&RowNum, 1, MPI_INT, 0, TAG+2, MPI_COMM_WORLD, &stat);

 unsigned char rowTypeData[RowNum][bufferSize]; //default data type for row in JPEG is unsigned char
  //use recieved size information to create appropriately sized array


 printf("DEBUG3a: Rowsize sent to slave is: %d \n", (int)RowNum);
 printf("DEBUG3b: Rows per slave sent to slave is: %d \n", (int)rowPerSlave);
 printf("DEBUG4: Buffer Size sent to slave is: %d \n", (int)bufferSize);

for (size_t j = 0; j <numprocs ; j++) //iterate over each slave
{
    for (size_t i = 0;  i <= rowPerSlave ; i++) 
    {
        if((i+j)<RowNum) //iterate until total size reached 
        {
        MPI_Recv(&rowTypeData[rowPerSlave*j+i][0], bufferSize, MPI_CHAR, j, TAG+4, MPI_COMM_WORLD, &stat);  //rowTypeData[x][0] is equivalent to a pointer to a specific row 
        }
    }
    printf("DEBUG5: Slave %d has received data \n", (int)(j+1));
}
 



}
//------------------------------------------------------------------------------

/** This is the entry point to the program. */
int main(int argc, char** argv){
 int myid;

 // MPI programs start with MPI_Init
 MPI_Init(&argc, &argv);

 // find out how big the world is
 MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

 // and this processes' rank is
 MPI_Comm_rank(MPI_COMM_WORLD, &myid);

 // At this point, all programs are running equivalently, the rank
 // distinguishes the roles of the programs, with
 // rank 0 often used as the "master".
 if(myid == 0) Master();
 else          Slave (myid);

 // MPI programs end with MPI_Finalize
 MPI_Finalize();
 return 0;
}
//------------------------------------------------------------------------------
