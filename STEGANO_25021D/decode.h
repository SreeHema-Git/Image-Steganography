#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include<stdlib.h>
#include "types.h" // Contains user defined types
#include "typesdecode.h"

/*
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname; // To store the stego image filename
    FILE *fptr_stego_image;  // File pointer for stego image


  /* Output File Info */
    char output_fname[50];    // To store the decoded output file name
    FILE *fptr_output;        // File pointer for output file
    char extn_secret_file[10];//to store decoded file extension 
    int extn_size;// To store the extension length
    long secret_file_size; // To store decoded secret file size
   

} DecodeInfo;

/* Decoding function prototype */


/* Read and validate decode args from argv */
// Reads and validates command-line arguments for decoding
Statusdecode read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform decoding */
// Main function to perform the decoding process
Statusdecode do_decoding(DecodeInfo *decInfo);

/* Open input/output files */
// Opens stego image and output file for decoding
Statusdecode open_decode_files(DecodeInfo *decInfo);

/* Skip BMP header */
// Skips the BMP file header before decoding begins
Statusdecode skip_bmp_header(FILE *fptr_stego_image);

/* Decode functions */
// Decodes and verifies the magic string from the image
Statusdecode decode_magic_string(DecodeInfo *decInfo);

// Decodes the size of the secret file extension
Statusdecode decode_secret_file_extn_size(DecodeInfo *decInfo);

// Decodes the actual secret file extension
Statusdecode decode_secret_file_extn( DecodeInfo *decInfo);

// Decodes the size of the secret file data
Statusdecode decode_secret_file_size( DecodeInfo *decInfo);

// Decodes the actual secret file data from the image
Statusdecode decode_secret_file_data(DecodeInfo *decInfo);

/* Helper functions */
// Decodes a single byte from the LSBs of the image buffer
char decode_byte_from_lsb( char *image_buffer);

// Decodes an integer (like size) from LSBs of image buffer
int decode_size_from_lsb(char *imageBuffer);



#endif
