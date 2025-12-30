/* 
Name        : S. SRI HEMAMBICA
Date        : 9-11-2025
Description : This Image Steganography Project in C is a console-based application that allows users to hide and retrieve secret information 
              within a BMP image using the Least Significant Bit (LSB) manipulation.

              It consists of the following functionalities:

              Encoding (-e):
                  → Hides secret  data from a file inside a BMP image.
                  → The data is embedded into the least significant bits of the image pixels.
                  → Supports optional output file naming for the encoded image.
                  → Ensures input file validation and correct file format checking .

              Decoding (-d):
                  → Extracts the hidden secret message from the encoded (stego) BMP image.
                  → Reconstructs the original secret file and saves it as an output  file.
                  → Verifies data integrity by reading magic strings and file sizes during decoding.

              File Handling:
                  → Handles multiple file operations (source image, secret file, and output file).
                  → Performs validation for file accessibility and correct extensions.
                  → Uses standard I/O operations for reading and writing binary data.

              Command-Line Arguments (CLA):
                  → Accepts user inputs in the format:
                        For Encoding : ./a.out -e <source.bmp> <secret.txt> [dest.bmp]
                        For Decoding : ./a.out -d <stego.bmp> [output.txt]
                  → Displays usage instructions for incorrect inputs or missing arguments.

              Core Concepts Used:
                  → Bitwise operations for data embedding and extraction.
                  → File pointer manipulation and header skipping in BMP files.
                  → Input validation, modular programming, and structured design in C.

              The project provides a clear demonstration of how digital data can be securely hidden 
              within images using steganography techniques while maintaining the visual integrity 
              of the original image.
*/
 
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "typesdecode.h"
#include "common.h"

OperationType check_operation_type(char *);//function declaration to check operation type

//  Function to print usage instructions
void print_usage()
{
    printf(YELLOW"\nUsage:\n"RESET);
    printf("  For Encoding : ./a.out -e <source.bmp> <secret.txt> [dest.bmp]\n");
    printf("                 -e  : Perform encoding\n");
    printf("                 <source.bmp> : Input BMP image file\n");
    printf("                 <secret.txt> : Secret text file to hide\n");
    printf("                 [dest.bmp]  : (Optional) Output image file name\n\n");

    printf("  For Decoding : ./a.out -d <stego.bmp> [output.txt]\n");
    printf("                 -d  : Perform decoding\n");
    printf("                 <stego.bmp> : Encoded image file\n");
    printf("                 [output.txt] : (Optional) Output file for decoded data\n\n");

    printf(YELLOW"Example:\n"RESET);
    printf("  ./a.out -e beautiful.bmp secret.txt dest.bmp\n");
    printf("  ./a.out -d dest.bmp out.txt\n\n");
}

int main(int argc, char *argv[])
{
    //  Handle missing arguments (./a.out)
    if (argc < 2)
    {
        printf(RED"Error: No operation provided! Use -e for encoding or -d for decoding.\n"RESET);
        print_usage();
        return e_failure;
    }

    OperationType operation = check_operation_type(argv[1]);//checking operation type

    //  if Encoding Operation
    if (operation == e_encode)
    {
        if (argc < 4 || argc > 5)// Check valid number of arguments
        {
            printf(RED"Error: Invalid number of arguments for encoding.\n"RESET);
            print_usage();
            return e_failure;
        }

        EncodeInfo encInfo;//creating variable for structure for the memory to get allocated for encoding 
        Status validate = read_and_validate_encode_args(argv, &encInfo);//validating input arguments (files and exensions)
        if (validate == e_failure)
        {
            printf(RED"Validation failed! Please check your input files and extensions.\n"RESET);
            return e_failure;
        }

        printf(GREEN"Validation of CLA is successful\n"RESET);
        do_encoding(&encInfo); // Start encoding process
    }

    //  Handle Decoding Operation
    else if (operation == e_decode)
    {
        if (argc < 3 || argc > 4)
        {
            printf(RED"Error: Invalid number of arguments for decoding.\n"RESET);
            print_usage();
            return d_failure;
        }

        DecodeInfo decInfo;//creating variable for structure for the memory to get allocated for decoading
        Statusdecode validate = read_and_validate_decode_args(argv, &decInfo);
        if (validate == d_failure)
        {
            printf(RED"Validation failed! Please check your input files and extensions.\n"RESET);
            return d_failure;
        }

        printf(GREEN"Validation of CLA is successful\n"RESET);
        do_decoding(&decInfo); // Start decoding process
    }

    //  Handle invalid operation 
    else
    {
        printf(RED"Error: Unsupported argument! Use -e for encoding or -d for decoding.\n"RESET);
        print_usage();
        return e_failure;
    }

    return e_success;//successful execution
}

OperationType check_operation_type(char *symbol)//// Function to check whether user selected encoding or decoding
{
    if (symbol == NULL)// Check for null argument
    {
        return e_unsupported;
    }

    if (strcmp(symbol, "-e") == 0)//Compare for encoding
    {
        printf(BLUE"Encoding is opted\n"RESET);
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0)// Compare for decoding
    {
        printf(BLUE"Decoding is opted\n"RESET);
        return e_decode;
    }
    else
    {
       // Invalid operation flag
        return e_unsupported;
    }
}
