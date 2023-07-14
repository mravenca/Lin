#include "FS.h"

// Initialization of FS(called once on start)
void FS_Init(void)
{
	HDD_Init();
	HDD_WriteHeader(0, NULL, 0); //?
	
}

// Save file from alocated memory to HDD by parts
//
// lintFilePtr	- ptr on data that can be saved on HDD
// lintFileSize	- data len
// returns	- file ID(0-FS_FILES_MAX-1), FS_ERR_FULL when FS is full, FS_ERR_ERROR when some other problem occures
UINT16 FS_SaveFile( UINT8 * lintFilePtr, UINT32 lintFileSize)
{
	
	UINT16 firstFreeFileId;
	//check endianess
	
	HDD_ReadHeader(0,(UINT8 *)&fsFileTable,HDD_SectorSize);
	//..
	
	
	
	//find first free fileId in FS_HEADER; if we don't find it we return FS_ERR_FULL
	for(UINT16 i=0;i<FS_FILES_MAX;i++) 
	{
		if(fsFileTable[i].Flags == 0)
		{
			firstFreeFileId = i;
			break;
		}
		if((i==(FS_FILES_MAX-1))&&(fsFileTable[i].Flags != 0))
		{
			return FS_ERR_FULL; 
		}
		
	}
	//..
	UINT16 AllocatedSector = 0;
	//UINT16 PreviousPhysicalSector = 0;
	//UINT16 SECTOR_IDs[numSectorsForFile];
	
	//FS_FILE_HEADER fsFileHeader;
	
	UINT16 numOfPhysicalSectorsWritten = 0;
	
	UINT16 numOfPhysicalSectorsForFile = lintFileSize / LogicalSectorSize; 
		
	if(lintFileSize % LogicalSectorSize > 0)
	{
		numOfPhysicalSectorsForFile++;
	}
	
	if(numOfPhysicalSectorsForFile > FS_FILES_MAX)
	{
		return FS_ERR_ERROR;
	}
	
		
	//first fill in the header - fill in address of the first sector element
	fsFileTable[firstFreeFileId].Flags = 1;
	fsFileTable[firstFreeFileId].FileSize = lintFileSize;
	AllocatedSector = HDD_Alloc();
	if(AllocatedSector != HDD_FULL)
	{
		fsFileTable[firstFreeFileId].NextElement = AllocatedSector;
	}
	else
	{
		return FS_ERR_FULL;
	}
		
	
	
	struct FS_PHYSICAL_SECTOR_ELEMENT *psel;
	psel = (struct FS_PHYSICAL_SECTOR_ELEMENT *) malloc(sizeof(struct FS_PHYSICAL_SECTOR_ELEMENT));
	
	while(numOfPhysicalSectorsForFile > 0)
	{
		//AllocatedSector = HDD_Alloc();
		
		if(AllocatedSector != HDD_FULL)
		{
			//copy one sector of file to one sector of HDD
			//sector needs to contain overhead data: see FS_PHYSICAL_SECTOR_ELEMENT in FS.h
							
			//PreviousPhysicalSector = AllocatedSector;
			AllocatedSector = HDD_Alloc();
			
			//memcpy Logical sector to physical sector
			//void * memcpy ( void * destination, const void * source, size_t num );
			memcpy(psel->LogicalSector, lintFilePtr, LogicalSectorSize);
						
			if(numOfPhysicalSectorsForFile>1)
			{
				psel->NextElement = AllocatedSector;
				//
				HDD_Write(AllocatedSector, 0/*!!*/, psel->LogicalSector, HDD_SectorSize-2);
				HDD_Write(AllocatedSector, HDD_SectorSize-2, psel->NextElement, 2);

				*lintFilePtr = *lintFilePtr + LogicalSectorSize;
				
				//update filesystem table
				//FS_SECTOR_ELEMENT *nextEl = 
			}
			else
			{
				psel->NextElement = 0;
				//
				//HDD_Write(AllocatedSector, 0/*!!*/, psel, lintFileSize % LogicalSectorSize /*+ HDD_SectorSize - LogicalSectorSize*/);
				HDD_Write(AllocatedSector, 0/*!!*/, psel->LogicalSector, HDD_SectorSize-2);
				HDD_Write(AllocatedSector, HDD_SectorSize-2, psel->NextElement, 2);

			}
			//update header
			//SECTOR_IDs[NumOfSectorsWritten] = AllocatedSector;//rewrite to point to next sector from the current sector
			//
			

			numOfPhysicalSectorsForFile--;
			numOfPhysicalSectorsWritten++;
		}
		else
		{
			return FS_ERR_FULL;
		}
	}
	//update flags ok
	//write file size to header ok
	//Write header, Copy header data to hdd
	//
	// lintHddOffset    - on which byte of sector start to write
	// lintData	        - ptr on data
	// lintDataLen	    - data len(1-1024)
	//void HDD_WriteHeader(UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen);
	HDD_WriteHeader(0, (UINT8 *)fsFileTable, sizeof(fsFileTable));//??
	return firstFreeFileId;
		
}

// Deletes file on HDD
//
// lintHandler	- file ID
// returns	TRUE - if file could be deleted
BOOL FS_DeleteFile(UINT16 lintHandler)
{
	struct FS_PHYSICAL_SECTOR_ELEMENT psel;
	
	//HDD_ReadHeader(0,&fsHeader,HDD_SectorSize);
	//UINT8 * u8fsFileTable = (UINT8 *)fsFileTable;
	
	HDD_ReadHeader(0,fsFileTable,HDD_SectorSize);
		
	//Delete file from header
	fsFileTable[lintHandler].Flags = 0;
	UINT16 NextEl = fsFileTable[lintHandler].NextElement;
	
	HDD_Read(NextEl,0/*!!*/, psel.LogicalSector, LogicalSectorSize);
	HDD_Read(NextEl, LogicalSectorSize, psel.NextElement, 2);
	
	HDD_DeAlloc(fsFileTable[lintHandler].NextElement);
	HDD_WriteHeader(0, (UINT8 *)fsFileTable, HDD_SectorSize);//??
	
	while (NextEl != NULL)
	{
		//HDD_Read(fsFileTable[psel.NextElement].NextElement,0/*!!*/, psel, HDD_SectorSize);
		//HDD_DeAlloc(fsFileTable.NextElement);
		
		//Load psel from HDD
		//load next sector ID
		//Deallocate original sector id	
		HDD_Read(NextEl, 0, psel.LogicalSector, LogicalSectorSize);
		HDD_Read(NextEl, LogicalSectorSize, psel.NextElement, 2);
				
		HDD_DeAlloc(NextEl);
		NextEl = psel.NextElement;
		
	}
}

// Copy file from HDD to alocated memory
// Coppies individual sectors(HDD_Read) for file(according to fileId) to cache prepared when this function is called
//
//
// lintHandler	- file ID
// lintFilePtr	- ptr on buffer in which u can insert data from HDD
// returns	TRUE - if file was correctly copied

BOOL FS_ReadFile( UINT16 lintHandler, UINT8* lintFilePtr)
{
	struct FS_PHYSICAL_SECTOR_ELEMENT psel;
	UINT16 RemainderLen = 0;
	
	//HDD_ReadHeader(0,&fsHeader,HDD_SectorSize);
	//UINT8 * u8fsFileTable = (UINT8 *)fsFileTable;
	
	HDD_ReadHeader(0,fsFileTable,HDD_SectorSize);
	//Delete file from header
	fsFileTable[lintHandler].Flags = 0;
	UINT16 NextEl = fsFileTable[lintHandler].NextElement;
	
	RemainderLen = fsFileTable[lintHandler].FileSize % LogicalSectorSize;
	UINT16 SectorCount = fsFileTable[lintHandler].FileSize / LogicalSectorSize;
	if(RemainderLen>0)
		SectorCount++;
	
	
	while(SectorCount > 0)
	{
		//RemainderLen = fsFileTable[lintHandler].FileSize % LogicalSectorSize;
		if(RemainderLen == 0)
		{
			HDD_Read(NextEl,0, psel.LogicalSector, LogicalSectorSize);
			HDD_Read(NextEl, LogicalSectorSize, psel.NextElement, 2);
			NextEl = psel.NextElement;
			memcpy(lintFilePtr,psel.LogicalSector, LogicalSectorSize);
		}
		else
		{
			HDD_Read(NextEl,0, psel.LogicalSector, RemainderLen);
			HDD_Read(NextEl, RemainderLen, psel.NextElement, 2);
			NextEl = psel.NextElement;
			memcpy(lintFilePtr,psel.LogicalSector, RemainderLen);
		}
		SectorCount--;
	}
	return TRUE;
}

// Get file size
//
// lintHandler	- file ID
// returns	- file len
UINT32 FS_GetFileSize( UINT16 lintHandler )
{
	//UINT8 * u8fsFileTable = (UINT8 *)fsFileTable;
	HDD_ReadHeader(0,fsFileTable,HDD_SectorSize);
	return fsFileTable[lintHandler].FileSize;
	
}