/*
	finfo: file info get, check, show..

	Zhang Lihui <swordhuihui@gmail.com>
	2011.11.10
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "finfo.h"

#define FILE_MODE_MASK 	0x1FF 

//
// get file information.
// return 0 if OK. others fail
//

int finfo_get(const char* pName, FINFO* pInfo)
{
	struct stat 	info;
	int 			mask_test;
	long int		t_modify;
    long int		iTestSize;
    char            csTestSum[128];
	char			*pOutName;

    //type 'F' , name, mode, LMT, size, cksum
	//printf("file name is :%s\n",csFileName);
	if(lstat(pName, &info)!=0)
	{
		//printf("File is not exist!\n");
		return (1);
	}

	pInfo->pName=pName;
	pInfo->size=info.st_size;
	pInfo->lastModTime=info.st_mtime;
	pInfo->mask=info.st_mode&FILE_MODE_MASK;
	pInfo->uid=info.st_uid;
	pInfo->gid=info.st_gid;

	if(S_ISLNK(info.st_mode))
	{
		//Links.
		pInfo->type='S';
	}
	else if(S_ISDIR(info.st_mode))
	{
		//Dir.
		pInfo->type='D';
	}
	else
	{
		//file.
		pInfo->type='F';

		//get md5 sum
		MDFile(pName, pInfo->csMD5, 128);
	}

	return 0;

}


int finfo_getFromCSVSeg(const char* segs[], int iMaxSeg, FINFO* pInfo)
{
	int iret=0;
	int iTemp;

	pInfo->type=segs[0][0];
	pInfo->pName=segs[1];

	if(pInfo->type == 'S')
	{
		//symbol link
		sscanf(segs[2], "%ld", &pInfo->size);
		sscanf(segs[3], "%o", &pInfo->mask);
		sscanf(segs[4], "%d", &iTemp);
		pInfo->uid=(unsigned short)iTemp;
		sscanf(segs[5], "%d", &iTemp);
		pInfo->gid=(unsigned short)iTemp;
		sscanf(segs[6], "%ld", &pInfo->lastModTime);
		pInfo->csMD5[0]='0';
	}
	else if(pInfo->type == 'D')
	{
		//Directory 
		sscanf(segs[2], "%ld", &pInfo->size);
		sscanf(segs[3], "%o", &pInfo->mask);
		sscanf(segs[4], "%d", &iTemp);
		pInfo->uid=(unsigned short)iTemp;
		sscanf(segs[5], "%d", &iTemp);
		pInfo->gid=(unsigned short)iTemp;
		sscanf(segs[6], "%ld", &pInfo->lastModTime);
		pInfo->csMD5[0]='0';
	}
	else if(pInfo->type == 'F')
	{
		//File.
		sscanf(segs[2], "%ld", &pInfo->size);
		sscanf(segs[3], "%o", &pInfo->mask);
		sscanf(segs[4], "%d", &iTemp);
		pInfo->uid=(unsigned short)iTemp;
		sscanf(segs[5], "%d", &iTemp);
		pInfo->gid=(unsigned short)iTemp;
		sscanf(segs[6], "%ld", &pInfo->lastModTime);
		strcpy(pInfo->csMD5, segs[7]);
	}
	else
	{
		//unknown line
		iret=1;
	}


	return iret;
}

int finfo_show(const char* pName, FINFO* pInfo)
{
	return 0;
}

//return 0 means match.
int finfo_cmp(FINFO* src, FINFO* dst, int ckmask)
{
	int iRet=0;

	//first, check type.
	if(src->type != dst->type && (ckmask & FINFO_CMP_FAIL_TYPE))
	{
		//type mismatch.
		iRet |= FINFO_CMP_FAIL_TYPE;
		printf("Err! Type Mismatch, file: %s, real: %c, record: %c\n",
			src->pName, src->type, dst->type);
		goto ENDFUNC;
	}

	//check size.
	if(src->size != dst->size && (ckmask & FINFO_CMP_FAIL_SIZE))
	{
		//type mismatch.
		iRet |= FINFO_CMP_FAIL_SIZE;
		printf("Err! Size Mismatch, file: %s, real: %ld, record: %ld\n",
			src->pName, src->size, dst->size);
	}

	//check time.
	if(src->lastModTime != dst->lastModTime && (ckmask & FINFO_CMP_FAIL_TIME))
	{
		//time mismatch.
		iRet |= FINFO_CMP_FAIL_TIME;
		printf("Err! Time Mismatch, file: %s, real: %ld, record: %ld\n",
			src->pName, src->lastModTime, dst->lastModTime);
	}

	//check mask.
	if(src->mask != dst->mask && (ckmask & FINFO_CMP_FAIL_MASK))
	{
		//mask mismatch.
		iRet |= FINFO_CMP_FAIL_MASK;
		printf("Err! Mask Mismatch, file: %s, real: %04o, record: %04o\n",
			src->pName, src->mask, dst->mask);
	}

	//check uid.
	if(src->uid != dst->uid && (ckmask & FINFO_CMP_FAIL_UID))
	{
		//uid mismatch.
		iRet |= FINFO_CMP_FAIL_UID;
		printf("Err! UID Mismatch, file: %s, real: %d, record: %d\n",
			src->pName, src->uid, dst->uid);
	}

	//check gid.
	if(src->gid != dst->gid && (ckmask & FINFO_CMP_FAIL_GID))
	{
		//gid mismatch.
		iRet |= FINFO_CMP_FAIL_GID;
		printf("Err! GID Mismatch, file: %s, real: %d, record: %d\n",
			src->pName, src->gid, dst->gid);
	}

	if(strcmp(src->csMD5, dst->csMD5) && (ckmask & FINFO_CMP_FAIL_MD5))
	{
		//md5 mismatch.
		iRet |= FINFO_CMP_FAIL_MD5;
		printf("Err! MD5 Mismatch, file: %s, real: %s, record: %s\n",
			src->pName, src->csMD5, dst->csMD5);
	}

ENDFUNC:

	return iRet;
}
