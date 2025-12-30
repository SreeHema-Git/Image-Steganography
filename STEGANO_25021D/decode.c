#include <stdio.h>
#include<string.h>
#include "decode.h"
#include "typesdecode.h"
#include "common.h"
// Function to read and validate command-line arguments for decoding
Statusdecode read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    
    char *src=strstr(argv[2],".bmp");//checking if .bmp is present in present in the src file given
    
    if(src==NULL||strcmp(src,".bmp")!=0)// Check if .bmp extension exists in given image file
    {
        printf(RED"The image file passed is not a .bmp file\n"RESET);
        return d_failure;
    }
    decInfo->stego_image_fname =argv[2];//copying beautiful.bmp into src_image_fname of structure
    if (argv[3] != NULL)
    {
        
        strcpy(decInfo->output_fname, argv[3]); // use given name
    }
    else
    {
        strcpy(decInfo->output_fname, "decoded_output"); // default name
    }
    return d_success;
    
}
// Function to open stego image file
Statusdecode open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image=fopen(decInfo->stego_image_fname,"rb"); // Open image in read-binary mode

    if(decInfo->fptr_stego_image==NULL)// Check if file failed to open
    {
        perror("fopen");//prints error in human readable form
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);//prints error into given output stream heree its stderr 

        return d_failure;
    }
    printf(GREEN"Stego image opened successfully.\n"RESET);
    return d_success;

    
}
// Function to skip BMP header (first 54 bytes)
Statusdecode skip_bmp_header(FILE *fptr_stego_image)
{
    // Move file pointer 54 bytes ahead
    if((fseek(fptr_stego_image,54,SEEK_SET))!=0)//0on sucess and non zero on failure
    {
        printf("Failed in skipping the 54 bytes header\n");
        return d_failure;

    }
    return d_success;
}
// Function to decode and verify the magic string
Statusdecode decode_magic_string(DecodeInfo *decInfo)
{
    
    char image_buffer[8];// Buffer to hold 8 bytes at a time
    int len=strlen(MAGIC_STRING);// Get length of magic string
    char magic_string[len+1];// Create buffer for decoded magic string
    for(int i=0;i<len;i++)// Loop through each character
    {
        if(fread(image_buffer,1,8,decInfo->fptr_stego_image)!=8)// Read 8 bytes
        {
            printf("Failed in reading 8 bytes from stego to buffer at magic string \n");
            return d_failure;
        }
        magic_string[i]=decode_byte_from_lsb(image_buffer);// Decode one byte
    }
    magic_string[len]='\0';// Null-terminate the string
    if(strcmp(magic_string,MAGIC_STRING)!=0)// Compare decoded string
    {
       printf("Magic string mismatch!\n");
       return d_failure; 
    }
    printf(GREEN "Magic string decoded successfully: %s\n" RESET, magic_string);
    return d_success;
}

// Function to decode secret file extension size
Statusdecode decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32];// Buffer for 32 bytes (4 bytes * 8 bits)
   
    if(fread(image_buffer,1,32,decInfo->fptr_stego_image)!=32) // Read 32 bytes
    {
        printf("Failed to read 32 bytes for extension size\n");
        return d_failure;
    }
    decInfo->extn_size = decode_size_from_lsb(image_buffer);// Decode size from LSBs
    printf(GREEN "Decoded extension size: %d\n" RESET, decInfo->extn_size);
    return d_success;


}
// Function to decode the secret file extension
Statusdecode decode_secret_file_extn( DecodeInfo *decInfo)
{
    char image_buffer[8];// Buffer for 8 bytes

     // Make sure buffer is large enough
    if (decInfo->extn_size >= sizeof(decInfo->extn_secret_file))// Validate extension length
    {
        printf(RED "Extension buffer too small!\n" RESET);
        return d_failure;
    }
    
    for(int i=0;i<decInfo->extn_size;i++)// Loop through extension size
    {
        if(fread(image_buffer,1,8,decInfo->fptr_stego_image)!=8)// Read 8 bytes for each char
        {
            printf("Failed to read 8 bytes for extension\n");
            return d_failure;
        }
        //Decode one character using your helper
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(image_buffer);// Decode each byte
    }
    decInfo->extn_secret_file[decInfo->extn_size] = '\0'; // null-terminate string
    printf(GREEN "Decoded file extension: %s\n" RESET, decInfo->extn_secret_file);
    return d_success;

}
// Function to decode secret file size
Statusdecode decode_secret_file_size( DecodeInfo *decInfo)
{
    char image_buffer[32];
    if (fread(image_buffer, 1, 32, decInfo->fptr_stego_image) != 32)// Buffer for 32 bytes (file size stored in 4 bytes)
    {
        printf("Failed to read 32 bytes for secret file size\n");
        return d_failure;
    }
    decInfo->secret_file_size = decode_size_from_lsb(image_buffer);// Decode file size
    printf(GREEN "Decoded secret file size: %ld bytes\n" RESET, decInfo->secret_file_size);
    return d_success;


}
// Helper function to decode 1 byte from 8 LSBs
char decode_byte_from_lsb(char *image_buffer)
{
    char ch = 0;
    for (int i = 0; i < 8; i++)
    {
        ch = (ch << 1) | (image_buffer[i] & 1);// Extract LSB and combine bits
    }
    return ch;
}
// Helper function to decode integer (like size) from 32 LSBs
int decode_size_from_lsb(char *imageBuffer)
{
    int size=0;
    for(int i=0;i<32;i++)
    {
        size = (size<<1) | ((imageBuffer[i] & 1) );// Combine 32 bits into integer
    }
    return size;
}
// Function to decode the actual secret file data and write to output
Statusdecode decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];// Buffer for reading 8 bytes at a time
    
     // Allocate memory for storing the entire decoded data
    char *secret_data = malloc(decInfo->secret_file_size);
    if (secret_data == NULL)// Check memory allocation
    {
        printf("Memory allocation failed!\n");
        return d_failure;
    }

    printf(YELLOW "Decoding secret file data...\n" RESET);
    for(int i=0;i<decInfo->secret_file_size;i++)// Decode each byte
    {
        if(fread(image_buffer,1,8,decInfo->fptr_stego_image)!=8)
        {
            printf(RED "Failed to read 8 bytes for secret file data at byte %d\n" RESET, i);
            free(secret_data);
            return d_failure;
        }
        // Decode one byte (character)
        secret_data[i] = decode_byte_from_lsb(image_buffer);// Decode each character
    }
    // Now write entire buffer to output file
    char final_output_name[100];
    strcpy(final_output_name, decInfo->output_fname);
    char *dot = strrchr(final_output_name, '.');// Find existing extension
    if (dot != NULL)
        *dot = '\0'; // cut off existing extension
    strcat(final_output_name, decInfo->extn_secret_file);// Add new extension

    

     // open output file for writing decoded data
    decInfo->fptr_output = fopen(final_output_name, "wb");
    if (decInfo->fptr_output == NULL)
    {
        printf("Failed to open output file %s\n", final_output_name);
        free(secret_data);// Free allocated memory
        return d_failure;
    }
    fwrite(secret_data, 1, decInfo->secret_file_size, decInfo->fptr_output);// Write decoded data
    printf(GREEN "Secret data decoded and written to %s successfully\n" RESET, final_output_name);

    free(secret_data);// Free allocated memory
    fclose(decInfo->fptr_output);// Close output file
    return d_success;


}
    
//function for decoding
Statusdecode do_decoding(DecodeInfo *decInfo)
{
    printf(BLUE"Decoding started...\n"RESET);

    if(!(open_decode_files(decInfo)))//opening files 
    {
        printf(RED"file  not opened successfully\n"RESET);//if e_failure comes prints error msg
        return d_failure;//failed case
    }
    else{
        
        printf(GREEN"All files are opened successfully\n"RESET);//if files opened successfully
        if (skip_bmp_header(decInfo->fptr_stego_image) == d_failure)// Skip BMP header
        {
            printf(RED "Failed to skip BMP header\n" RESET);
            return d_failure;
        }
        printf(GREEN"bmp header copied successfully\n"RESET);
        if(decode_magic_string(decInfo)==d_failure) // Verify magic string
        {
            printf(RED"Magic string is not correct\n"RESET);
            return d_failure;
        }
        printf(GREEN"Magic string verified successfully\n"RESET);
        if(decode_secret_file_extn_size(decInfo)==d_failure)// Decode extension size
        {
            printf(RED"Secret file extension size is not decoded\n"RESET);
            return d_failure;

        }
        printf(GREEN "Decoded file extension size successfully\n");
        
        if (decode_secret_file_extn(decInfo) == d_failure)// Decode file extension
        {
            printf(RED"Secret file extension  is not decoded\n"RESET);
            return d_failure;

        }
        printf(GREEN "Decoded file extension successfully\n");
        if (decode_secret_file_size(decInfo) == d_failure)// Decode file size
        {
            printf(RED"Secret file  size is not decoded\n"RESET);
            return d_failure;

        }
        printf(GREEN "Decoded file  size successfully\n");
        if (decode_secret_file_data(decInfo) == d_failure)// Decode actual file data
        {
            printf(RED"Secret file data is not decoded\n"RESET);
            return d_failure;
        }
        printf(GREEN "Secret file data encoded successfully\n");
        printf(GREEN "Decoding completed successfully\n" RESET);
        return d_success;

        
    }
}