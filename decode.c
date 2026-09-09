#include <stdio.h>
#include <string.h>
#include "decode.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2],".BMP") || strstr(argv[2],".bmp"))
    {
        decInfo->stego_image_fname=argv[2];
    }
    else
    {
        return e_failure;
    }
    if(argv[3]!=NULL)
    {
        if(strstr(argv[3],".txt"))
        {
            decInfo->secret_fname=argv[3];
        }
        else
        {
            return e_failure;
        }
    }
    else
    {
        decInfo->secret_fname="output.txt";
    }
    return e_success;
}
Status open_decode_file(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image=fopen(decInfo->stego_image_fname,"r");
    if(decInfo->fptr_stego_image==NULL)
    {
        return e_failure;
    }
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    if(decInfo->fptr_secret == NULL)
    {
        return e_failure;
    }
    return e_success;
}
Status skip_bmp_header(DecodeInfo *decInfo)
{
    if(fseek(decInfo->fptr_stego_image,54,SEEK_SET)!=0)
    {
        return e_failure;
    }
    return e_success;
}
int decode_int_from_image(char *ms_size)
{
    int size = 0;

    for (int i = 0; i < 32; i++)
    {
        int bit = ms_size[i] & 1;
        size= size | (bit << i);
    }

    return size;
}
Status decode_magic_string_size(DecodeInfo *decInfo)
{
    char arr[32];
    if(fread(arr,32,1,decInfo->fptr_stego_image)!=1)
    {
        return e_failure;
    }
    int size=decode_int_from_image(arr);
    decInfo->magic_string_size=size;
    //printf("Magic string size = %d\n", size);
    return e_success;
}
char decode_char_from_image(char *arr)
{
    char data = 0;

    for(int i = 0; i < 8; i++)
    {
        int bit = arr[i] & 1;
        data = data | (bit << i);
    }

    return data;
}
Status decode_magic_string(DecodeInfo *decInfo)
{
    for(int i=0;i<decInfo->magic_string_size;i++)
    {
        char arr[8];
        if(fread(arr,8,1,decInfo->fptr_stego_image)!=1)
        {
            return e_failure;
        }
        decInfo->magic[i]=decode_char_from_image(arr);
    
    }
    decInfo->magic[decInfo->magic_string_size] = '\0';

    //printf("Decoded magic string: %s\n", decInfo->magic); 
    char m_s[10];
    printf("Enter magic string : ");
    scanf("%s",m_s);
    if(strcmp(m_s,decInfo->magic)==0)
    {
        return e_success;
    }
    else
    {
        printf("Enterd magic string not match.\n");
        return e_failure;
    }
    return e_success;
}
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char arr[32];
    if(fread(arr,32,1,decInfo->fptr_stego_image)!=1)
    {
        return e_failure;
    }
    decInfo->extn_size=decode_int_from_image(arr);
    //printf("Extension size = %d\n", decInfo->extn_size);
    return e_success;
}
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    for(int i=0;i<decInfo->extn_size;i++)
    {
        char arr[8];
        if(fread(arr,8,1,decInfo->fptr_stego_image)!=1)
        {
            return e_failure;
        }
        decInfo->extn_secret_file[i]=decode_char_from_image(arr);
    }
    decInfo->extn_secret_file[decInfo->extn_size]='\0';
    //printf("Secret file extension : %s\n",decInfo->extn_secret_file);
    return e_success;
}
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char arr[32];
    if(fread(arr,32,1,decInfo->fptr_stego_image)!=1)
    {
        return e_failure;
    }
    decInfo->size_secret_file=decode_int_from_image(arr);
    //printf("Secret file size = %ld\n", decInfo->size_secret_file);
    return e_success;
}
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    for(int i=0;i<decInfo->size_secret_file;i++)
    {
        char arr[8];
        if(fread(arr,8,1,decInfo->fptr_stego_image)!=1)
        {
            return e_failure;
        }
        char data=decode_char_from_image(arr);
        if(fwrite(&data,1,1,decInfo->fptr_secret)!=1)
        {
            return e_failure;
        }
    }   
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_file(decInfo)==e_failure)
    {
        printf("ERROR: Failed to opend file %s \n",decInfo->stego_image_fname);
        return e_failure;
    }
    if(skip_bmp_header(decInfo)==e_failure)
    {
        printf("ERROR: Failed to skip BMP header\n");
        return e_failure;
    }
    if(decode_magic_string_size(decInfo)==e_failure)
    {
        printf("ERROR: Failed to decode magic string size\n");
        return e_failure;
    }
    if(decode_magic_string(decInfo)==e_failure)
    {
        printf("ERROR: Failed to decode magic string\n");
        return e_failure;
    }
    if(decode_secret_file_extn_size(decInfo)==e_failure)
    {
        printf("ERROR: Failed to decode secret file Extension size.\n");
        return e_failure;
    }
    if(decode_secret_file_extn(decInfo)==e_failure)
    {
        printf("ERROR: Failed to decode secret file Extension.\n");
        return e_failure;
    }
    if(decode_secret_file_size(decInfo)==e_failure)
    {
        printf("ERROR: Failed to decode secret file Size.\n");
        return e_failure;
    }
    if(decode_secret_file_data(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file data.\n");
        return e_failure;
    }

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_secret);
    printf("Decoding completed successfully.\n");
    return e_success;

}