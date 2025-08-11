/*
Name : Nestin Gregorios Sunny

Date : 07.08.2025

Description :
Project 3 - MP3 Tag Reader and Editor using C programing

Functions Demonstrated:
1. View details of first 6 tags of a MP3 File
    (Title, Artist, Album, Year, Content, Comments)
2. Edit any of the first 6 tags of a MP3 File 

Sample input and output:
i/p : ./mp3_tag_reader
o/p : 
ERROR:  Use "./mp3_tag_reader --h" for Help Menu

i/p : ./mp3_tag_reader --h
o/p : 
***Help Menu***
INFO:  Viewer: Use "./mp3_tag_reader -v <filename>
INFO:  Editor: Use "./mp3_tag_reader -e [modifier] <new_content> <filename>

INFO:  Modifiers:
        -t      Title
        -a      Artist
        -A      Album
        -y      Year
        -m      Content
        -c      Comment
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"view.h"
#include"type.h"

/* Check Operation Type */
Operation check_operation_type(char *argv[])
{
    if (!argv[1]) 
    {
        return unsupported;
    }

    if(strcmp(argv[1], "-v") == 0)     
    {   
        return view;
    }
    else if (strcmp(argv[1], "-e") == 0)   
    {
        return edit;
    }
    else if (strcmp(argv[1], "--h") == 0)   
    {
        return help;
    }
    else
    {
        return unsupported;
    }
}

/* Perform Validation */
Status read_and_validate_mp3_file(char **argv, View *viewInfo)
{
    if (argv[2] == NULL)
    {
        printf("INFO:  For viewing the tags -> ./mp3_tag_reader -v <filename>\n");
        return failure;
    }

    viewInfo->fptr_mp3 = fopen(argv[2], "rb");
    if (viewInfo->fptr_mp3 == NULL)
    {
        printf("ERROR: The file does not appear to be a valid MP3 (missing 'ID3' signature)\n");
        return failure;
    }
            
    char header[4] = {0}; // extra byte for '\0'
    fread(header, 3, 1, viewInfo->fptr_mp3);
    if (strcmp(header, "ID3") != 0)
    {
        fprintf(stderr, "ERROR: Missing 'ID3' signature\n");
        fclose(viewInfo->fptr_mp3);
        return failure;
    }

    printf("    Version ID: v2.3\n");
    printf("**************************\n\n");

    return success;
}

/* View Tag Details Operation */
Status view_tag(char **argv, View *viewInfo)
{
    viewInfo->fptr_mp3 = fopen (argv [2], "rb");		//Open the file in Read Only Mode.
    if (!viewInfo->fptr_mp3) 
    {
        fprintf(stderr, "ERROR: Unable to open MP3 file\n");
        return failure;
    }

    //Skip ID3 header
    fseek (viewInfo->fptr_mp3, 10L, SEEK_SET);		//The first 10 bytes of the '.mp3' file is ID3 Header and need to be skipped.
	Status ret;

	//Function call with the necessary parameters to display the Information about Title tag.
    ret = get_and_display("Title: ", "TIT2", &viewInfo->title_tag_size, &viewInfo->title_tag, viewInfo->fptr_mp3);
    if (ret == failure)		//If the function doesn't display the data successfully, the process of Displaying will terminate.
    {
        printf ("ERROR:  Title Frame ID Contents cannot be displayed.\n");
        return failure;
    }

	//Function call with the necessary parameters to display the Information about Artist tag.
    ret = get_and_display("Artist: ", "TPE1", &viewInfo->artist_tag_size, &viewInfo->artist_tag, viewInfo->fptr_mp3);
    if (ret == failure)		//If the function doesn't display the data successfully, the process of Displaying will terminate.
    {
        printf ("ERROR:  Artist Frame ID Contents cannot be displayed.\n");
        return failure;
    }

	//Function call with the necessary parameters to display the Information about Album tag.
    ret = get_and_display("Album: ", "TALB", &viewInfo->album_tag_size, &viewInfo->album_tag, viewInfo->fptr_mp3);
    if (ret == failure)		//If the function doesn't display the data successfully, the process of Displaying will terminate.
    {
        printf ("ERROR:  Album Frame ID Contents cannot be displayed.\n");
        return failure;
    }

	//Function call with the necessary parameters to display the Information about Year tag.
    ret = get_and_display("Year: ", "TYER", &viewInfo->year_size, &viewInfo->year_tag, viewInfo->fptr_mp3);
    if (ret == failure)		//If the function doesn't display the data successfully, the process of Displaying will terminate.
    {
        printf ("ERROR:  Year Frame ID Contents cannot be displayed.\n");
        return failure;
    }

	//Function call with the necessary parameters to display the Information about Content tag.
    ret = get_and_display("Content: ", "TCON",  &viewInfo->content_tag_size, &viewInfo->content_tag, viewInfo->fptr_mp3);
    if (ret == failure)		//If the function doesn't display the data successfully, the process of Displaying will terminate.
    {
        printf ("ERROR:  Content Type Frame ID Contents cannot be displayed.\n");
        return failure;
    }

	//Function call with the necessary parameters to display the Information about Comments tag.
    ret = get_and_display("Comments: ", "COMM", &viewInfo->comments_tag_size, &viewInfo->comments_tag, viewInfo->fptr_mp3);
    if (ret == failure)		//If the function doesn't display the data successfully, the process of Displaying will terminate.
    {
        printf ("ERROR:  Content Type Frame ID Contents cannot be displayed.\n");
        return failure;
    }

    
    fclose (viewInfo->fptr_mp3);				//To close the Source MP3 file.

    //No p-failure returned above, then return p_success.
    return success;
}

/* Display the various frame contents of MP3 file */
Status get_and_display(const char *str_frame, const char *str_id, uint *tag_size, char **tag, FILE *fp)
{
    char frame_id[FRAME_ID_BUF_SIZE] = {0};
    uint size;
    long start_pos = ftell(fp);

    while (fread(frame_id, 1, FRAME_ID_SIZE, fp) == FRAME_ID_SIZE)
    {
        frame_id[4] = '\0';

        if (strcmp(frame_id, "\0\0\0\0") == 0)
        {
            break;
        }

        if (fread(&size, sizeof(uint), 1, fp) != 1)
        {
            fprintf(stderr, "ERROR: Unable to read tag size\n");
            return failure;
        }

        size = swap_endian(size);

        // Skip flags
        fseek(fp, FRAME_HEADER_SKIP, SEEK_CUR); 

        if (strcmp(frame_id, str_id) == 0)
        {
            *tag = (char *)malloc(size);
            if (!*tag)
            {
                fprintf(stderr, "ERROR: Memory allocation failed\n");
                return failure;
            }

            if (fread(*tag, 1, size - 1, fp) < size - 1)
            {
                fprintf(stderr, "ERROR: Unable to read tag data\n");
                free(*tag);
                return failure;
            }

            (*tag)[size - 1] = '\0';
            *tag_size = size;
            printf("%-10s%s\n", str_frame, *tag);
            free(*tag);
            return success;
        }
        else
        {
            fseek(fp, size, SEEK_CUR); // Skip to next frame
        }
    }

    fprintf(stderr, "ERROR: %s Frame ID not found\n", str_id);
    return failure;

}

/* Convert Big Endian to Little Endian */
uint swap_endian(uint val)
{
    return ((val >> 24) & 0x000000FF) | ((val >> 8)  & 0x0000FF00) |
           ((val << 8)  & 0x00FF0000) | ((val << 24) & 0xFF000000);
}