#ifndef FS_H_INCLUDED
#define FS_H_INCLUDED

/*
* fsFileTable (must fit one sector)
*      -------------------------------------
*128 *| 2B FLAGS|4B FILESIZE|2B FIRST_SECTOR|
*      -------------------------------------
*
* Structure records on HDD:
*                             --------------------------------------------
*FS_PHYSICAL_SECTOR_ELEMENT  |2B ELEMENT_ID| 1020B CONTENT |2B NEXT_SECTOR| 
*                             --------------------------------------------
*
* Maximum disc size = 2^16*1024= 64 MB
*/

#include "HDD.h"

// Maximum number of files that can be saved
#define FS_FILES_MAX        128//65534 //UINT16 FS_SaveFile	//chose yourself

// Return value if FS is FULL
#define FS_ERR_FULL         FS_FILES_MAX

// Return value if FS detects some bad inputs, file not found etc...
#define FS_ERR_ERROR        FS_FILES_MAX+1

#define LogicalSectorSize HDD_SectorSize-2

struct FS_PHYSICAL_SECTOR_ELEMENT 
{
	//UINT16 PhysicalSectorID;//as returned by HDD_Alloc() or required by HDD_Write(..) or HDD_Read(..) //2B
	
	UINT8 LogicalSector[LogicalSectorSize];
	
	UINT16 NextElement;//if NULL then it is the last sector of a file 
};

//Definition of file header
//8B
struct FS_FILE_HEADER
{
	
	UINT16 Flags; 

	UINT32 FileSize;
	
	UINT16 NextElement;//if NULL then file is empty 
};
//Id of File should be the index of array of FS_FILE_HEADER
struct FS_FILE_HEADER fsFileTable[FS_FILES_MAX];

// Initialization of FS(called once on start)
void FS_Init(void); //ok

// Deletes file on HDD
//
// lintHandler	- file ID
// returns	- if file could be deleted
BOOL FS_DeleteFile(UINT16 lintHandler); //ok

// Get file size
//
// lintHandler	- file ID
// returns	- file len
UINT32 FS_GetFileSize( UINT16 lintHandler ); //ok

// Copy file from HDD to alocated memory
//
// lintHandler	- file ID
// lintFilePtr	- ptr on buffer in which u can insert data from HDD
// returns	- if file was correctly copied
BOOL FS_ReadFile( UINT16 lintHandler, UINT8* lintFilePtr); //ok

// Save file from alocated memory to HDD
//
// lintFilePtr	- ptr on data that can be saved on HDD
// lintFileSize	- data len
// returns	- file ID(0-FS_FILES_MAX-1), FS_ERR_FULL when FS is full, FS_ERR_ERROR when some other problem occures
UINT16 FS_SaveFile( UINT8 * lintFilePtr, UINT32 lintFileSize); //ok

#endif // FS_H_INCLUDED
