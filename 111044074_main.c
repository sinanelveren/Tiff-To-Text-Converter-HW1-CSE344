/**
 * tiffProcessor.c
 * Sinan Elveren.
 * Gebze Technical University - Computer Engineering
 * CSE 344 System Programming
 * HW01 - TIFF proccesor that reading a image from file and print it to screen
 * TIF FILE :: || ByteOrder(2bytes) + id(2bytes) + offset**(4bytes) + ImageData(bytes) + **TagID & Data ||
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define NDEBUG               //make it for debug mode


//for merge bytes & switch bytes
#define endian(hex) (((hex & 0x00ff) << 8) + ((hex & 0xff00) >> 8))
#define endian4(hex) (((hex & 0x000000ff) << 24) + ((hex & 0x0000ff00) << 8) + ((hex & 0xff000000) >> 24) + ((hex & 0x00ff0000) >> 8))
#define merge(hex1, hex2) (((hex1 & 0x00ff) << 8) + (hex2 & 0x00ff)) //?
#define merge4(hex1, hex2) (((hex1 & 0x0000ffff) << 16) + (hex2 & 0x0000ffff)) //?



//convert and print to given byte's bit
void byteToBinary(int byte, int bit, int color) {
    char white = '1';
    char black = '0';
    if(color == 0) {  //if wihte is zero, do it one
        black = '1';
        white = '0';
    }

    if (bit == 0)
        putc(byte & 0x80 ? white : black, stdout);
    if (bit == 1)
        putc(byte & 0x40 ? white : black, stdout);
    if (bit == 2)
        putc(byte & 0x20 ? white : black, stdout);
    if (bit == 3)
        putc(byte & 0x10 ? white : black, stdout);
    if (bit == 4)
        putc(byte & 0x08 ? white : black, stdout);
    if (bit == 5)
        putc(byte & 0x04 ? white : black, stdout);
    if (bit == 6)
        putc(byte & 0x02 ? white : black, stdout);
    if (bit == 7)
        putc(byte & 0x01 ? white : black, stdout);

    putchar(' ');
}


int main(int argc, char *argv[]) {

    int     tagID;
    int     photometric = 0,
            stripOffset = 0 ,
            imageWidth = 0,
            imageLength = 0, rowsPerStrip, stripByteCounts;
    int     index;
    int     byteOrder;
    int     offset;
    int     tagCount =0;       //total tags count
    FILE *  pFile;
    long    fileSize;
    unsigned char*  buffer;


    /* There are no argument that file name */
    if (argc != 2){
        printf ("Usage: ./tiffProcessor test.tif");
        return 1;
    }

    //open tiff file
    pFile = fopen (argv[1]  , "rb" );
    if (pFile==NULL) {
        fprintf (stderr, "ERR : File didn't opened %s [%s]\n", argv[1], strerror(1));
        return STDERR_FILENO;;
    }


    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    fileSize = ftell (pFile);
    rewind (pFile);


    // allocate memory to contain the whole file:
    buffer = (unsigned char*) malloc (sizeof(unsigned char)*fileSize);
    if (buffer == NULL) {
        fputs ("Memory error",stderr);
        return EXIT_FAILURE;
    }


    //read header from file to head structer
    fread(buffer, sizeof(short),1,pFile);
    byteOrder= merge(buffer[0],buffer[1]);
    //pass to id
    fread(buffer, sizeof(short),1,pFile);
    //read offset - ifd
    fread(buffer, sizeof(long),1,pFile);
    offset= merge4( merge(buffer[0], buffer[1]), merge(buffer[2], buffer[3]) );


    //if it s intel tiff file
    if(byteOrder == 0x4949) {
        offset = endian4(offset);
    }
    //exit if it s wrong tif file
    else if (byteOrder != 0x4D4D){
        perror("Error: Wrong File - Exit");
        return EXIT_FAILURE;
    }


#ifndef NDEBUG        //offset info
    printf("BYTE ORDER %.4X: \n", byteOrder);
    printf("OFFSET %.8X: \n", offset);
#endif


    rewind(pFile);


#ifndef NDEBUG        //print all bytes
    for (int i = 0; i < fileSize; i++) {

        if(i%16 == 0) {
            printf("\n");
            printf("%.4x :", i);
        }

        if(i%2 == 0 )
            printf(" ");

        result = fread(buffer, sizeof(char),1,pFile);
        printf("%.2x",  buffer[0]);
    }
    rewind(pFile);
#endif


    //go to location of tags that offset'th bytes
    fseek( pFile, offset, SEEK_SET );

    //Number of Directory Entry (0c) - Total TAGs count
    fread(buffer, 2*sizeof(char),1,pFile);

    if(byteOrder == 0x4D4D)
        tagCount = merge(buffer[0], buffer[1]);
    else
        tagCount = merge(buffer[1], buffer[0]);

#ifndef NDEBUG
    printf("\nTags's count : %d\n", tagCount);
#endif

    index = offset+2;       //read 2 bytes <-
    //find PhotometricInterpretation
    for (int i = 0; i < tagCount; i++) {
        fread(buffer, 12*sizeof(char),1,pFile);

        if(byteOrder == 0x4D4D)
            tagID = merge(buffer[0],buffer[1]);
        else
            tagID =  merge(buffer[1],buffer[0]);

        if (tagID == 256)
            imageWidth = index;
        else if (tagID == 257)
            imageLength = index;
        else if (tagID == 262)
            photometric = index;
        else if (tagID == 278)
            rowsPerStrip = index;
        else if (tagID == 273)
            stripOffset = index;
        else if (tagID == 279)
            stripByteCounts = index;
        index += 12;
    }


    //assign tags's data
    rewind(pFile);
    fseek( pFile, imageWidth, SEEK_SET );
    fread(buffer, 10*sizeof(char),1,pFile);
    if(byteOrder == 0x4D4D)
        imageWidth = merge(buffer[8],buffer[9]);
    else
        imageWidth = merge(buffer[9],buffer[8]);

    fseek( pFile, imageLength, SEEK_SET );
    fread(buffer, 10*sizeof(char),1,pFile);
    if(byteOrder == 0x4D4D)
        imageLength = merge(buffer[8],buffer[9]);
    else
        imageLength = merge(buffer[9],buffer[8]);

    fseek( pFile, photometric, SEEK_SET );
    fread(buffer, 10*sizeof(char),1,pFile);
    if(byteOrder == 0x4D4D)
        photometric = merge(buffer[8],buffer[9]);
    else
        photometric = merge(buffer[9],buffer[8]);

    fseek( pFile, stripOffset, SEEK_SET );
    fread(buffer, 10*sizeof(char),1,pFile);
    if(byteOrder == 0x4D4D)
        stripOffset = merge(buffer[8],buffer[9]);
    else
        stripOffset = merge(buffer[9],buffer[8]);


#ifndef NDEBUG
    printf("imageWidth : %.4x.\nimageLength : %.4x.\nphotometric : %.4x.\nStripOffset : %.4x.\n ",
           imageWidth, imageLength,photometric, stripOffset);
    printf("\n\n\n");
#endif


    //go to location of first pixel
    rewind(pFile);
    fseek( pFile, 8, SEEK_SET );


    //print to screen of image info and pixel of image
    fprintf(stdout, "Width : %d pixels\n", imageWidth);
    fprintf(stdout, "Length : %d pixels\n", imageLength);

    if(byteOrder == 0x4D4D)
        fprintf(stdout,"Byte Order : Motorola\n");
    else
        fprintf(stdout,"Byte Order : İntel\n");


    for (int line = 0; line < imageLength; line++) {
        int i = 0;
        //read - bytes that upperRound(width of a line / 1byte)
        //for 25 pixel, need to (25 / 8bit) + 1
        fread(buffer, (imageWidth/8 + 1) * sizeof(char), 1, pFile);

        //print bit by bit  until image width
        //one byte is printing bit by bit
        for(int j = 0; j < imageWidth; j++) {
            //next byte
            if (j % 8 == 0 && j > 0)
                i++;

            //print one byte. 8 times for print bit by bit
            byteToBinary((int) buffer[i], (j % 8), photometric);
        }
        printf("\n");
    }


    // terminate
    fclose (pFile);
    free (buffer);


    return (EXIT_SUCCESS);    // Zero success, while any Non-Zero value an error
}
