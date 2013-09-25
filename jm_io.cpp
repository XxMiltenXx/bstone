#include "id_heads.h"

#include "jm_io.h"
#include "jm_cio.h"
#include "jm_lzh.h"
#include "jm_error.h"

//--------------------------------------------------------------------------
// IO_FarRead()
//--------------------------------------------------------------------------
boolean IO_FarRead(int handle, void* dest, int length)
{
    int read_result;

    read_result = read(handle, dest, length);

    return read_result == length;
}

//--------------------------------------------------------------------------
// IO_FarWrite()
//--------------------------------------------------------------------------
boolean IO_FarWrite(int handle, const void* source, int length)
{
    int write_result;

    write_result = write(handle, source, (unsigned)length);

    return write_result == length;
}

#if DEMOS_EXTERN

//--------------------------------------------------------------------------
// IO_WriteFile()
//--------------------------------------------------------------------------
boolean IO_WriteFile(char *filename, void *ptr, Sint32 length)
{
	Sint16 handle;
	Sint32 size;

	handle = open(filename,O_CREAT | O_BINARY | O_WRONLY,
				S_IREAD | S_IWRITE | S_IFREG);

	if (handle == -1)
		return(false);

	if (!IO_FarWrite (handle,ptr,length))
	{
		close (handle);
		return(false);
	}
	close (handle);
	return(true);
}

#endif				 

//--------------------------------------------------------------------------
// IO_LoadFile()
//--------------------------------------------------------------------------
int IO_LoadFile (const char* filename, void** dst)
{
	char buffer[5]={0,0,0,0,0};
	Sint16 handle;
	Sint32 size=0;

	if ((handle = open(filename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		return(size);

	read(handle,buffer,4);
	if (!strcmp(buffer,"JAMP"))
	{
		struct JAMPHeader head;

		read(handle,&head,sizeof(head));
		size = head.OriginalLen;
		switch (head.CompType)
		{
			case ct_LZH:
				LZH_Startup();

                *dst = malloc(head.OriginalLen);

// FIXME
#if 0
				if (mmerror)
					return(0);
#endif // 0

// FIXME
#if 0
				LZH_Decompress((void *)handle,*dst,size,head.CompressLen,SRC_FILE|DEST_MEM);
#endif // 0

                {
                std::auto_ptr<Uint8> buffer(new Uint8[head.CompressLen]);

                ::LZH_Decompress(buffer.get(), *dst,
                    size, head.CompressLen);
                }

				LZH_Shutdown();
			break;

			case ct_LZW:
				IO_ERROR(IO_LOADFILE_NO_LZW);
			break;

			default:
				IO_ERROR(IO_LOADFILE_UNKNOWN);
			break;
		}
	}
	else
	{
		lseek(handle,0,SEEK_SET);
		size = filelength(handle);
        *dst = malloc(size);
		if (!IO_FarRead(handle,*dst,size))
		{
			close(handle);
			return(size);
		}
	}

	close(handle);

	return(size);
}

#if 0

//--------------------------------------------------------------------------
// IO_CopyFile()
//--------------------------------------------------------------------------
void IO_CopyFile(char *sFilename, char *dFilename)
{
	Sint16 sHandle,dHandle;
	Uint16 length;

// Allocate memory for buffer.
//
	if ((sHandle = open(sFilename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		IO_ERROR(IO_COPYFILE_OPEN_SRC);

	if ((dHandle=open(dFilename,O_CREAT|O_RDWR|O_BINARY,S_IREAD|S_IWRITE))==-1)
		IO_ERROR(IO_COPYFILE_OPEN_DEST);

// Copy that file!
//
	IO_CopyHandle(sHandle,dHandle,-1);

// Close files.
//
	close(sHandle);
	close(dHandle);
}

#endif

//--------------------------------------------------------------------------
// IO_CopyHandle()
//--------------------------------------------------------------------------
void IO_CopyHandle(int sHandle, int dHandle, int num_bytes)
{
	extern boolean bombonerror;

	#define CF_BUFFER_SIZE 8192

	int fsize;
	void* src;

	unsigned length;

// Allocate memory for buffer.
//
    src = malloc(CF_BUFFER_SIZE);
	if (num_bytes == -1)
		fsize=filelength(sHandle);
	else
		fsize=num_bytes;

// Copy that file!
//
	do
	{
	// Determine length to read/write.
	//
		if (fsize >= CF_BUFFER_SIZE)
			length = CF_BUFFER_SIZE;
		else
			length = fsize;

	// Read it, write it and decrement length.
	//
		IO_FarRead(sHandle,src,length);
		IO_FarWrite(dHandle,src,length);
		fsize -= length;
	}
	while (fsize);

// Free buffer.
//
    free(src);
}

