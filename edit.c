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

#include"edit.h"
#include"view.h"
#include"type.h"

/* Perform Validation of the Arguments passed and store in the Structure */
Status read_and_validate_mp3_file_args (char* argv[], Edit *editInfo)
{
    if (!argv[2] || !argv[3] || !argv[4])
    {
        printf("INFO:  For Editing -> ./mp3_tag_reader -e [modifier] <new_content> <filename>\n\n");
        printf("INFO:  Modifiers:\n");
        printf("\t-t\tTitle\n\t-a\tArtist\n\t-A\tAlbum\n\t-y\tYear\n\t-m\tContent\n\t-c\tComment\n");
        return failure;
    }

    if (strcmp(argv[2], "-t") == 0)
    {
        strncpy(editInfo->frame_id, "TIT2", 5);
    }
    else if (strcmp(argv[2], "-a") == 0)
    {
        strncpy(editInfo->frame_id, "TPE1", 5);
    }
    else if (strcmp(argv[2], "-A") == 0)
    {
        strncpy(editInfo->frame_id, "TALB", 5);
    }
    else if (strcmp(argv[2], "-y") == 0)
    {
        strncpy(editInfo->frame_id, "TYER", 5);
    }
    else if (strcmp(argv[2], "-m") == 0)
    {
        strncpy(editInfo->frame_id, "TCON", 5);
    }
    else if (strcmp(argv[2], "-c") == 0)
    {
        strncpy(editInfo->frame_id, "COMM", 5);
    }
    else
    {
        printf("ERROR:  Unsupported Modifier\n");
        return failure;
    }
    
    size_t value_len = strlen(argv[3]);
    if (value_len + 1 > sizeof(editInfo->frame_id_value))
    {
        printf("ERROR: Frame value too long\n");
        return failure;
    }

    editInfo->frame_id_value[0] = 0x00; // Encoding byte
    strncpy(editInfo->frame_id_value + 1, argv[3], value_len);
    editInfo->frame_id_size = value_len + 1;

    editInfo->fptr_mp3 = fopen(argv[4], "rb");
    if (!editInfo->fptr_mp3)
    {
        printf("ERROR: Cannot open file %s\n", argv[4]);
        return failure;
    }

    char sig[3];
    fread(sig, 1, 3, editInfo->fptr_mp3);
    fclose(editInfo->fptr_mp3);

    if (strncmp(sig, "ID3", 3) != 0)
    {
        printf("ERROR: Not a valid MP3 file\n");
        return failure;
    }

    return success;
}

const char* get_frame_description(const char* frame_id)
{
    if (strcmp(frame_id, "TIT2") == 0) return "Title";
    if (strcmp(frame_id, "TPE1") == 0) return "Artist";
    if (strcmp(frame_id, "TALB") == 0) return "Album";
    if (strcmp(frame_id, "TYER") == 0) return "Year";
    if (strcmp(frame_id, "COMM") == 0) return "Comment";
    if (strcmp(frame_id, "TCON") == 0) return "Content";
    return "Unknown";
}


/* Perform the Editing Tag details Operation */
Status edit_tag (char* argv[], Edit *editInfo)
{
    editInfo->fptr_mp3 = fopen(argv[4], "rb");
    editInfo->fptr_temp = fopen("temp.mp3", "wb");

    if (editInfo->fptr_mp3 == NULL || editInfo->fptr_temp == NULL)
    {
        printf("ERROR: File open failed\n");
        return failure;
    }

    printf("INFO: Editing frame %s (%s) in file %s...\n", editInfo->frame_id, get_frame_description(editInfo->frame_id), argv[4]);

    // Copy ID3 header (first 10 bytes)
    char header[10];
    if (fread(header, 1, 10, editInfo->fptr_mp3) != 10)
    {
        printf("ERROR: Failed to read ID3 header\n");
        fclose(editInfo->fptr_mp3);
        fclose(editInfo->fptr_temp);
        return failure;
    }
    fwrite(header, 1, 10, editInfo->fptr_temp);
    printf("INFO:  ID3 header copied successfully\n");
    
    int id3_version = (unsigned char)header[3]; // Extract version

    // Copy and edit frames
    printf("INFO:  Scanning frames\n");
    if (copy_data(editInfo, id3_version) == failure || copy_remaining_data(editInfo) == failure)
    {
        fclose(editInfo->fptr_mp3);
        fclose(editInfo->fptr_temp);
        return failure;
    }

    fclose(editInfo->fptr_mp3);
    fclose(editInfo->fptr_temp);

    // Replace original file
    remove(argv[4]);
    rename("temp.mp3", argv[4]);

    return success;
}

/* Copy the Frame Contents from the Source to Temporary MP3 file */
Status copy_data (Edit *editInfo, int version)
{
    char frame_header[10];

    while (fread(frame_header, 1, 10, editInfo->fptr_mp3) == 10)
    {
        // Optional: Print raw header bytes for debugging
        /*printf("RAW HEADER BYTES: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
               (unsigned char)frame_header[0], (unsigned char)frame_header[1],
               (unsigned char)frame_header[2], (unsigned char)frame_header[3],
               (unsigned char)frame_header[4], (unsigned char)frame_header[5],
               (unsigned char)frame_header[6], (unsigned char)frame_header[7],
               (unsigned char)frame_header[8], (unsigned char)frame_header[9]);
        */
       
        uint frame_size = get_frame_size(frame_header, version);
        
        char frame_id[5];
        strncpy(frame_id, frame_header, 4);
        frame_id[4] = '\0';

        // Validate frame ID: must be 4 uppercase letters or digits
        if (frame_id[0] == '\0' || strspn(frame_id, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") < 4)
        {
            printf("INFO:  Reached non-frame data or padding. Stopping frame scan.\n");
            break;
        }

        // Sanity check for zero-size frames
        if (frame_size == 0)
        {
            printf("INFO:  Encountered zero-size frame. Skipping.\n");
            continue;
        }

        // Check for absurdly large frame sizes
        if (frame_size > MAX_FRAME_SIZE)
        {
            printf("ERROR:  Frame size too large (%u bytes)\n", frame_size);
            return failure;
        }

        //printf("DEBUG: Frame ID = %s, Size = %u bytes\n", frame_id, frame_size);

        if (strcmp(frame_id, editInfo->frame_id) == 0)
        {
            // Write updated frame
            printf("INFO:  Replacing frame %s\n", frame_id);
            fwrite(editInfo->frame_id, 1, 4, editInfo->fptr_temp);

            write_big_endian_uint(editInfo->fptr_temp, editInfo->frame_id_size);

            fwrite("\0\0", 1, 2, editInfo->fptr_temp); // Flags
            fwrite(editInfo->frame_id_value, 1, editInfo->frame_id_size, editInfo->fptr_temp);

            // Skip old frame data
            fseek(editInfo->fptr_mp3, frame_size, SEEK_CUR);
        }
        else
        {
            // Copy original frame
            fwrite(frame_header, 1, 10, editInfo->fptr_temp);
            char *buffer = malloc(frame_size);
            if (!buffer)
            {
                printf("ERROR: Memory allocation failed\n");
                return failure;
            }
            fread(buffer, 1, frame_size, editInfo->fptr_mp3);
            fwrite(buffer, 1, frame_size, editInfo->fptr_temp);
            free(buffer);
        }
    }

    return success;

}


/* Copy the contents after the Frames have been copied from Source to Temporary MP3 file */

Status copy_remaining_data (Edit *editInfo)
{
    char ch;
    printf("INFO:  Copying remaining frames unchanged\n");
    printf("INFO:  Copying remaining MP3 data\n");

    while (fread(&ch, 1, 1, editInfo->fptr_mp3) == 1)
    {
        fwrite(&ch, 1, 1, editInfo->fptr_temp);
    }
    return success;
}


/* Helper: Convert 4-byte big-endian to uint */
uint get_frame_size(char *header, int version)
{
    if (version == 4) // ID3v2.4 uses synchsafe integers
    {
        return ((header[4] & 0x7F) << 21) |
               ((header[5] & 0x7F) << 14) |
               ((header[6] & 0x7F) << 7)  |
               (header[7] & 0x7F);
    }
    else // ID3v2.3 and earlier
    {
        return ((unsigned char)header[4] << 24) |
               ((unsigned char)header[5] << 16) |
               ((unsigned char)header[6] << 8)  |
               ((unsigned char)header[7]);
    }
}

/* Helper: Write uint as 4-byte big-endian */
void write_big_endian_uint(FILE *fp, uint value)
{
    fputc((value >> 24) & 0xFF, fp);
    fputc((value >> 16) & 0xFF, fp);
    fputc((value >> 8) & 0xFF, fp);
    fputc(value & 0xFF, fp);
}
