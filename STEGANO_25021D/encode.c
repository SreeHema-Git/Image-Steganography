#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

//Validate and read encoding arguments //
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    
    char *src=strstr(argv[2],".bmp");//checking if .bmp is present in present in the src file given
    
    if(src==NULL||strcmp(src,".bmp")!=0)//if not present or if it is not at end
    {
        printf(RED"The image file passed is not a .bmp file\n"RESET);
        return e_failure;
    }
    encInfo->src_image_fname=argv[2];//copying beautiful into src_image_fname of structure
    
    char *secret_file = argv[3];// Secret file name
    const char *allowed_extns[] = {".txt", ".c", ".sh", ".pdf", ".docx"};// Allowed file types
    int valid_extn = 0;
    char *extn_found = NULL;
    // Check if secret file has valid extension
    for (int i = 0; i < 5; i++)
    {
        extn_found = strstr(secret_file, allowed_extns[i]);
        if (extn_found && strcmp(extn_found, allowed_extns[i]) == 0)
        {
            valid_extn = 1;
            break;
        }
    }
    if (!valid_extn)
    {
        printf(RED"The secret file must be one of: .txt, .c, .sh, .pdf, or .docx\n"RESET);
        return e_failure;
    }
    strcpy(encInfo->extn_secret_file, extn_found); // Copy file extension
    encInfo->secret_fname = argv[3];// Store secret file name
    // Handle optional destination file
    if (argv[4] == NULL)
    {
        printf(YELLOW"Stegano file not provided, creating default dest.bmp\n"RESET);
        encInfo->stego_image_fname = "dest.bmp";// Default output file
        printf(GREEN"%s file will be created.\n"RESET, encInfo->stego_image_fname);
    }
    else{
        src = strstr(argv[4], ".bmp");// Validate destination file
        if (src == NULL || strcmp(src, ".bmp") != 0)
        {
            printf(RED"The stego file must be a .bmp file\n"RESET);
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4];// Assign output file name
    }

    return e_success;
}
// Open source, secret, and destination files //

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");//opening src file in read mode
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");//prints error in human readable form
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);//prints error into given output stream heree its stderr 

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");//opening secret text file in read mode
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");//prints error in human readable form
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);//prints error into given output stream heree its stderr 

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");//opening destination file in write mode
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");//prints error in human readable form
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);//prints error into given output stream heree its stderr 

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

// Get BMP image size using width and height fields//
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);//taking width from 54 bytes of header

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);//taking 4 bytes from 18th position
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);//after width in header we have height taking 4 bytes
    printf("height = %u\n", height);

    // Return image capacity
    
    return width * height * 3; //since(1pixcel =3 bytes for RGB );
}
// Get file size of secret file //
uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr,0,SEEK_END);//moving pointer to end
    uint size=ftell(fptr);//the offset of pointer is the size in bytes
    
    return size;
}
// Check if image has enough capacity for encoding //
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity=get_image_size_for_bmp(encInfo->fptr_src_image);//checking the src file image capacity in bytes
    encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);//getting the size of the secret file.txt
    printf("Image Capacity for src file = %u bytes\n",encInfo->image_capacity);//printing .bmp capacity
    printf("Secret file size =%lu bytes\n",encInfo->size_secret_file);//printing sizeof secret file
    // Check if image can hold all secret data
    if((encInfo->image_capacity) > (54+16+32+32+32+(encInfo->size_secret_file)*8))//54-header,16-len of magic string,32-size of .txt ext,32-size of actual .txt,32-size of secrect file data,secretfilelen*8 how much bytes sck file data occupies
    {
        printf(GREEN"Capacity is checked successfully\n"RESET);
        printf(GREEN"Success for size\n"RESET);
        return e_success;
    }
    return e_failure;


}
// Copy 54-byte BMP header //
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);//since we moved filepointer offset to height and width in  get_image_size_for_bmp function we are rewinding it to 0 position
    char buffer[54];
    if(fread(buffer,1,54,fptr_src_image)!=54)//src_img->buffer
    {
        printf("Error:Cannot read .bmp header\n");//if failed to copy
        return e_failure;
    }
    if(fwrite(buffer,54,1,fptr_dest_image)!=1)///buffer->dest.bmp
    { 
        printf("Error:Cannot write .bmp header into dest_img\n");//if failed to copy
        return e_failure;

    } 
    long srcptr=ftell(fptr_src_image);//pointer offset
    long destptr=ftell(fptr_dest_image);//dest pointer offset
    if(srcptr!=destptr)//checking if they are equal
    {
        printf(RED"Pointers are not pointing to same offset\n"RESET);
        return e_failure;
    } 
    printf(RED"Pointers are pointing to same offset\n"RESET); 
    printf(GREEN"Header is copied successfully\n"RESET);
    return e_success;
  
}
// Encode magic string into image //
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    
    int len=strlen(magic_string);
    char imgbuffer[8];
    for(int i=0;i<len;i++)
    {
        if(fread(imgbuffer,1,8,encInfo->fptr_src_image)!=8)//copying 8 bytes from beautil.bmp to imgbuffer
        {
            printf("Error:Cant read 8 bytes from source file\n");
            return e_failure;
        }
        if(encode_byte_to_lsb(magic_string[i],imgbuffer)!=e_success)//calling encode_byte_to_lsb to encode every bit of magic string character in a byte of beautiful.bmp
        {
            printf("Failed to encode byte to lsb in magic string\n");
            return e_failure;

        }
    
        if(fwrite(imgbuffer,8,1,encInfo->fptr_stego_image)!=1)//writing from imgbuffer->dest_image
        {
            printf("Error: Cannot write 8 bytes to destination image.\n");
            printf("The magic string encoded copy in destion is not done\n");
            return e_failure;
        }
     }
    printf(GREEN"Magic string is encoded successfully\n"RESET);    
    return e_success;
    //checking if offsets are pointed to same position
    long srcptr=ftell(encInfo->fptr_src_image);
    long destptr=ftell(encInfo->fptr_stego_image);
    if(srcptr!=destptr)
    {
        printf(RED"Pointers are not pointing to same offset\n"RESET);
        return e_failure;
    } 
    printf(RED"Pointers are pointing to same offset\n"RESET); 
    return e_success;
}

// Encode secret file extension size //
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char imgbuffer[32];//Read 32 bytes from beautiful.bmp
    if(fread(imgbuffer,1,32,encInfo->fptr_src_image)!=32)
    {
        printf("Error: Cannot read 32 bytes from source image for secret file extn size\n");
        return e_failure;
    }
    if(encode_size_to_lsb(size,imgbuffer)!=e_success)// Encode extension size
    {
        printf("Error: Encoding secret file extension size into beautiful.bmp failed\n");
        return e_failure;

    }
    if(fwrite(imgbuffer,32,1,encInfo->fptr_stego_image)!=1)// Write encoded bytes
    {
        printf("Error: Cannot write encoded extn size to stego image\n");
        return e_failure;

    }
    printf(GREEN"Secret file extension size is encoded successfully\n"RESET);
   
   return e_success;
   //checking if pointer offset are at same position
   long srcptr=ftell(encInfo->fptr_src_image);
    long destptr=ftell(encInfo->fptr_stego_image);
    if(srcptr!=destptr)
    {
        printf(RED"Pointers are not pointing to same offset\n"RESET);
        return e_failure;
    } 
    printf(RED"Pointers are pointing to same offset\n"RESET); 
    return e_success;
    
  
}

// Encode secret file extension //
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    int len=strlen(file_extn);
    char imgbuffer[8];
    for(int i=0;i<len;i++)
    {
        if(fread(imgbuffer,1,8,encInfo->fptr_src_image)!=8) // Read 8 bytes
        {
            printf("Error: Cannot read bytes for file extn encoding\n");
            return e_failure;
        }
        if(encode_byte_to_lsb(file_extn[i],imgbuffer)!=e_success)// Encode one char
        {
            printf("Error: Encoding file extn char failed\n");
            return e_failure;

        }
        if (fwrite(imgbuffer, 8, 1, encInfo->fptr_stego_image) != 1)// Write encoded bytes
        {
            printf("Error: Cannot write encoded extn char\n");
            return e_failure;
        }
        

    }
    printf(GREEN"Secret file extension is encoded successfully\n"RESET);
    //checking if offset is pointed to same pointer
    long srcptr=ftell(encInfo->fptr_src_image);
    long destptr=ftell(encInfo->fptr_stego_image);
    if(srcptr!=destptr)
    {
        printf(RED"Pointers are not pointing to same offset\n"RESET);
        return e_failure;
    } 
    printf(RED"Pointers are pointing to same offset\n"RESET); 
    return e_success;
}

// Encode size of the secret file //
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char imgbuffer[32];
    if(fread(imgbuffer,1,32,encInfo->fptr_src_image)!=32)// Read 32 bytes
    {
        printf("Error: cannot read bytes for secret file size encoading\n");
        return e_failure;
    }
    if(encode_size_to_lsb((int)file_size,imgbuffer)!=e_success)// Encode file size
    {
        printf("Error: Encoding secret file size failed\n");
        return e_failure;
        
    }
    if(fwrite(imgbuffer,1,32,encInfo->fptr_stego_image)!=32)// Write to output
    {
        printf("Error: Cannot write encoded secret file size\n");
        return e_failure;
    }
    printf(GREEN"Secret file size is encoded successfully\n"RESET);
     
    long srcptr=ftell(encInfo->fptr_src_image);
    long destptr=ftell(encInfo->fptr_stego_image);
    if(srcptr!=destptr)
    {
        printf(RED"Pointers are not pointing to same offset\n"RESET);
        return e_failure;
    } 
    printf(RED"Pointers are pointing to same offset\n"RESET);  
    return e_success;

}
//  Encode actual secret file data //
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    
    char secret_data[encInfo->size_secret_file];
    rewind(encInfo->fptr_secret);//just ensuring to point it to first
    if(fread(secret_data,encInfo->size_secret_file,1,encInfo->fptr_secret)!=1)
    {
        printf("Error: Cannot read secret bytes from source image for data in secret_file_data encoading\n");
        return e_failure;
    }
    char imgbuffer[8];
    for(int i=0;i<(encInfo->size_secret_file);i++)
    {
        if(fread(imgbuffer,1,8,encInfo->fptr_src_image)!=8)// Read 8 bytes from image
        {
            printf("Error: Cannot read 8 bytes from source image\n");
            return e_failure;
        }
        if (encode_byte_to_lsb(secret_data[i], imgbuffer) != e_success)// Encode secret byte
        {
            printf("Error: Encoding secret file data failed\n");
            return e_failure;
        }
        if (fwrite(imgbuffer, 1, 8, encInfo->fptr_stego_image) != 8)// Write encoded bytes
        {
            printf("Error: Cannot write encoded bytes to stego image\n");
            return e_failure;
        }

        

    }
    printf(GREEN"Secret file data encoded successfully!\n"RESET);
    long srcptr=ftell(encInfo->fptr_src_image);
    long destptr=ftell(encInfo->fptr_stego_image);
    if(srcptr!=destptr)
    {
        printf(RED"Pointers are not pointing to same offset\n"RESET);
        return e_failure;
    } 
    printf(RED"Pointers are pointing to same offset\n"RESET); 
        
    return e_success;

}
// Copy remaining bytes from source image to destination //
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,1,1,fptr_src)==1) // Read byte by byte
    {
        fwrite(&ch,1,1,fptr_dest);// Copy to output image
    }
    printf(GREEN"Remaining data is encoded successfully\n"RESET);  
    //checking if offset is pointing to same position
    long srcptr=ftell(fptr_src);
    long destptr=ftell(fptr_dest);
    if(srcptr!=destptr)
    {
        printf(RED"Pointers are not pointing to same offset\n"RESET);
        return e_failure;
    } 
    printf(RED"Pointers are pointing to same offset\n"RESET); 
    return e_success;
}


// Encode a single byte into 8 LSBs of image buffer //
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    
    for(int i=0;i<8;i++)
    {
        int get_bit=((data>>(7-i))&1);//geting bit from data 
        image_buffer[i]=((image_buffer[i]&(~1))|(get_bit));//clearing the lsb bit in image buffer and placing get_bit
        
        
    }
    return e_success;


}

// Encode integer size into 32 LSBs of image buffer //
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for(int i=0;i<32;i++)
    {
        int get_bit=((size>>(31-i))&1);//geting bit from data
        imageBuffer[i]=((imageBuffer[i]&(~1))|(get_bit));//clearing the lsb bit in image buffer and placing get_bit
        
        
    }
    return e_success;
    

}



// Perform all encoding steps in sequence //
Status do_encoding(EncodeInfo *encInfo)
{
    if(!(open_files(encInfo)))//opening files 
    {
        printf(RED"files are not opened successfully\n"RESET);//if e_failure comes prints error msg
        return e_failure;//failed case
    }
    else{
        printf(GREEN"All files are opened successfully\n"RESET);//if files opened successfully
        if(check_capacity(encInfo)==e_failure)//checking for capacity
        {
            printf(RED"Image size is less\n"RESET);
            return e_failure;
        }
        
        if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure)//copying beautiful.bmp header into dest.bmp
        {
            printf(RED"Header encoading failed\n"RESET);
            return e_failure;
        }
        if(encode_magic_string(MAGIC_STRING,encInfo)==e_failure)
        {
            printf(RED"Failed in encoading magic string\n"RESET);
            return e_failure;

        }
        if((encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo))==e_failure)
        {
            printf(RED"Failed in encoading secret file extension size\n"RESET);
            return e_failure;

        }
        if((encode_secret_file_extn(encInfo->extn_secret_file,encInfo))==e_failure)
        {
            printf(RED"Failed in encoading secret file extension \n"RESET);
            return e_failure;

        }
        if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_failure)
        {
            printf(RED"Failed in encoading secret file size\n"RESET);
            return e_failure;
        }
        if(encode_secret_file_data(encInfo)==e_failure)
        {
            printf(RED"Failed in encoading secret file data\n"RESET);
            return e_failure;
        }
        if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_failure)
        {
            printf(RED"Failed in copying remaining data from beautiful.bmp to dest.bmp\n"RESET);
            return e_failure;

        }
        printf(GREEN"Encoding completed successfully!\n"RESET);
        return e_success;
        
        
    }
    
    
   

}
