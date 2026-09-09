#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp") || strstr(argv[2], ".BMP"))
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    if (strstr(argv[3], ".txt"))
    {
        encInfo->secret_fname = argv[3];
        strcpy(encInfo->extn_secret_file,strstr(argv[3], "."));
    }
    else
    {
        return e_failure;
    }

    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    else
    {
        if (strstr(argv[4], ".bmp") || strstr(argv[4], ".BMP"))
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            return e_failure;
        }
    }

    return e_success;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity

    rewind(fptr_image);
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{

    printf("Enter magic string: ");
    scanf("%s", encInfo->magic);
    // MOVE the secret_fptr to the last
    fseek(encInfo->fptr_secret,0,SEEK_END);

    int size_of_info =sizeof(int) * 8 + strlen(encInfo->magic) * 8 + sizeof(int) * 8 +
        strlen(encInfo->extn_secret_file) * 8 +
        sizeof(int) * 8 +
        ftell(encInfo->fptr_secret) * 8;

    rewind(encInfo->fptr_secret);

    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    if(size_of_info < encInfo->image_capacity)
    {
        return e_success;
    }

    return e_failure;
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];

    rewind(fptr_src_image);

    if (fread(header, 54, 1, fptr_src_image) != 1)
    {
        return e_failure;
    }

    if (fwrite(header, 54, 1, fptr_dest_image) != 1)
    {
        return e_failure;
    }

    return e_success;
}
void encode_int_image(char *arr,int data)
{
    for(int i=0;i<32;i++)
    {
        arr[i]=arr[i]&(~1);
        int get_bit=(data & (1<<i))>>i;
        arr[i]=arr[i]|get_bit;
    }

}
Status encode_size_of_ms(EncodeInfo *encInfo)
{
    int size = strlen(encInfo->magic);
    char arr[32];

    if (fread(arr, 32, 1, encInfo->fptr_src_image) != 1)
    {
        return e_failure;
    }

    encode_int_image(arr, size);

    if (fwrite(arr, 32, 1, encInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }

    return e_success;
}
void encode_char_to_image(char *arr, char data)
{
    for (int i = 0; i < 8; i++)
    {
        arr[i] = arr[i] & (~1);
        int bit = (data >> i) & 1;
        arr[i] = arr[i] | bit;
    }
}
void encode_string_to_image(const char *str,int size,FILE * fptr_src_image,FILE * fptr_stego_image)
{
    for(int i=0;i<size;i++)
    {
        char arr[8];
        fread(arr,8,1,fptr_src_image);
        encode_char_to_image(arr,str[i]);
        fwrite(arr,8,1,fptr_stego_image);
    }
}

Status encode_magic_string(const char *magic, EncodeInfo *encInfo)
{
    encode_string_to_image(encInfo->magic,strlen(encInfo->magic),encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}
Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    int size = strlen(encInfo->extn_secret_file);
    char arr[32];

    fread(arr, 32, 1, encInfo->fptr_src_image);

    encode_int_image(arr, size);

    fwrite(arr, 32, 1, encInfo->fptr_stego_image);

    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_string_to_image(
        file_extn,
        strlen(file_extn),
        encInfo->fptr_src_image,
        encInfo->fptr_stego_image);

    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char arr[32];
    fread(arr, 32, 1, encInfo->fptr_src_image);
    encode_int_image(arr, file_size);
    fwrite(arr, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_data_to_image(char *data, int size,FILE *fptr_src_image,FILE *fptr_stego_image)
{
    for (int i = 0; i < size; i++)
    {
        char arr[8];
        fread(arr, 8, 1, fptr_src_image);
        encode_char_to_image(arr, data[i]);
        fwrite(arr, 8, 1, fptr_stego_image);
    }

    return e_success;
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char *data;
    data = malloc(encInfo->size_secret_file);
    if (data == NULL)
    {
        return e_failure;
    }
    rewind(encInfo->fptr_secret);
    if (fread(data, 1, encInfo->size_secret_file,encInfo->fptr_secret) != encInfo->size_secret_file)
    {
        free(data);
        return e_failure;
    }

    encode_data_to_image(data,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    free(data);

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        if (fwrite(&ch, 1, 1, fptr_dest) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure)
    {
        return e_failure;
    }

    if (check_capacity(encInfo) == e_failure)
    {   
        printf("Error: Not enough space to encode the secret file\n");
        return e_failure;
    }

    printf("Capacity check successful\n");

    if (copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("Error: Failed to copy BMP header\n");
        return e_failure;
    }
    if(encode_size_of_ms(encInfo)==e_failure){
        printf("Error: Failed to encode magic string size.\n");
        return e_failure;

    }
    if(encode_magic_string(encInfo->magic,encInfo)==e_failure)
    {
        printf("Error: Failed to encode magic strig.\n");
        return e_failure;

    }
    if(encode_secret_file_extn_size(encInfo)==e_failure)
    {
        printf("Error: Failed to encode secret file extention size.\n");
        return e_failure;

    }
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_failure)
    {
        printf("Error: Failed to encode secret file extention.\n");
        return e_failure;
    }

    fseek(encInfo->fptr_secret, 0, SEEK_END);
    encInfo->size_secret_file = ftell(encInfo->fptr_secret);
    rewind(encInfo->fptr_secret);
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo)==e_failure)
    {
        printf("Error: Failed to encode secret file size.\n");
        return e_failure;
    }
    if(encode_secret_file_data(encInfo)==e_failure)
    {
        printf("Error: Failed to encode secret file data.\n");
        return e_failure;
    }
    if (copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("Error: Failed to encode remaining data.\n");
        return e_failure;
    }

    printf("Encoding completed successfully.\n");


    return e_success;
}

