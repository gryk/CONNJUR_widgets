/*
 * Copyright (C) 2018 Michael R. Gryk
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

/* 
 * File:   nmrview.c
 * Author: mgryk
 * 
 * Created on August 23, 2018, 2:20 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <gtk/gtk.h>
#include "../lib/cjrutil.h"
#include "../lib/nmrview.h"

/*
 * readNMRViewHeader
 * 
 * This function parses individual fields of a binary NMRView (.nv) file.  
 * 
 * How it works:  This function just uses fread to read the binary header directly into the NMRViewHeader struct.
 * NMRView files can be either endian.  This function does NOT correct endianess.  That should be done elsewhere.
 * 
 * Inputs: NMRView filename
 *         GError for error callbacks.
 * Output: nmrViewHeader (success or failure).  If failure, GError should not be NULL.
 */

nmrviewHeaderStruct readNMRViewHeader(char* fileName, GError **error){
    
    struct stat fileStat;
    nmrviewHeaderStruct nmrviewBuffer;
    FILE *fp;

    if (stat(fileName, &fileStat) != 0)  {
        (*error) = g_error_new(G_FILE_ERROR, G_FILE_ERROR_NOENT, "Stat failure on %s", fileName);
        return nmrviewBuffer;
    }
    if (fileStat.st_size < (NMRVIEW_HEADER_SIZE))  {
        (*error) = g_error_new(G_FILE_ERROR, G_FILE_ERROR_NOENT, "%s too small to be ucsf file", fileName);
        return nmrviewBuffer;
    }
    fp=fopen(fileName,"r+");
    if (fp == NULL) {
        (*error) = g_error_new(G_FILE_ERROR, G_FILE_ERROR_NOENT, "Could not open %s", fileName);
        return nmrviewBuffer;
    }
    fread(&nmrviewBuffer, 1, NMRVIEW_HEADER_SIZE, fp);
    fclose(fp);
    if (nmrviewBuffer.file.magic != NMRVIEW_MAGIC) {
        if (nmrviewBuffer.file.magic == NMRVIEW_CIGAM) (*error) = g_error_new(G_FILE_ERROR, G_FILE_ERROR_NOENT, "Wrong Endian: %s", fileName);
        else (*error) = g_error_new(G_FILE_ERROR, G_FILE_ERROR_NOENT, " Not nmrview file: %s", fileName);
    }
    
    return nmrviewBuffer;
}

void byteSwapNMRViewBuffer(nmrviewHeaderStruct *header){
    
    int i;          
    
    if (header->file.magic == NMRVIEW_CIGAM) {
        printf("here\n");
    }
}

/*
 * nmrviewHeaderAsString
 * 
 * This function converts an NMRView header struct into a string.  
 * This function is created for debugging purposes only.  
 */

char *nmrviewHeaderAsString(nmrviewHeaderStruct *header){
    
    int i;
    static char nmrviewString[1000];
    
    sprintf(nmrviewString,"magic: %d\tversion %d\tfile header size: %d\n", 
            header->file.magic, header->file.version, header->file.fileHeaderSize);
    sprintf(&nmrviewString[strlen(nmrviewString)],
            "block header size: %d\tblock elements: %d\tnumber of dimensions: %d\n", 
            header->file.blockHeaderSize, header->file.blockElements, header->file.nDim);
    if ((header->file.nDim < 1) || (header->file.nDim > 8)) return nmrviewString;
    for (i=0; i<header->file.nDim; i++) {
        sprintf(&nmrviewString[strlen(nmrviewString)],
                "size: %d\tblockSize: %d\tnBlocks: %d\t",
                header->axis[i].size, header->axis[i].blockSize, header->axis[i].nBlocks);
        sprintf(&nmrviewString[strlen(nmrviewString)],
                "sf: %.2f\tsw: %.2f\tnrefpt: %.2f\n",
                header->axis[i].sf, header->axis[i].sw, header->axis[i].refpt);
        sprintf(&nmrviewString[strlen(nmrviewString)],
                "\trefval: %.2f\trefunits: %d\t",
                header->axis[i].refval, header->axis[i].refunits);
        sprintf(&nmrviewString[strlen(nmrviewString)],
                "label: %s\t",
                cjr_terminate_string(header->axis[i].label, 16));      // Extra care here.  If label is 16 bytes, no \0
        sprintf(&nmrviewString[strlen(nmrviewString)],
                "complex: %d\tfreq_dom: %d\tph0: %.2f\tph1: %.2f\tvsize: %d\n",
                header->axis[i].complex, header->axis[i].freqdomain, 
                header->axis[i].ph0, header->axis[i].ph1, header->axis[i].vsize);
    }
    
    return nmrviewString;
}