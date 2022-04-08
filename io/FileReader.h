
#include "Buffer.h"
#include "FastxChunk.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <zlib.h>  //support gziped files, functional but inefficient
#include "Reference.h"
#include "igzip_lib.h"

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)  // D_SCL_SECURE
#pragma warning(disable : 4244)  // conversion uint64 to uint32
//# pragma warning(disable : 4267)
#define FOPEN fopen
#define FDOPEN fdopen
#define FSEEK _fseeki64
#define FTELL _ftelli64
#define FCLOSE fclose
#elif __APPLE__  // Apple by default suport 64 bit file operations (Darwin 10.5+)
#define FOPEN fopen
#define FDOPEN fdopen
#define FSEEK fseek
#define FTELL ftell
#define FCLOSE fclose
#else
#if !defined(_LARGEFILE_SOURCE)
#define _LARGEFILE_SOURCE
#if !defined(_LARGEFILE64_SOURCE)
#define _LARGEFILE64_SOURCE
#endif
#endif
#if defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS != 64)
#undef _FILE_OFFSET_BITS
#endif
#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif
#define FOPEN fopen64
#define FDOPEN fdopen
#define FSEEK fseeko64
#define FTELL ftello64
#define FCLOSE fclose
#endif
namespace rabbit{

class FileReader{
private:
	static const uint32 SwapBufferSize = 1 << 20;  // the longest FASTQ sequence todate is no longer than 1Mbp.
	static	const uint32 IGZIP_IN_BUF_SIZE = 1 << 22; // 4M gziped file onece fetch
public:
	FileReader(const std::string &fileName_, bool isZipped){
    if(ends_with(fileName_, ".gz") || isZipped) {
      //mZipFile = gzdopen(fd, "r");
      //// isZipped=true;
      //if (mZipFile == NULL) {
      //  throw RioException("Can not open file to read: ");  
      //}
			//gzrewind(mZipFile);
			mFile = fopen(fileName_.c_str(), "rb");
			if (mFile == NULL){
        throw RioException(
          ("Can not open file to read: " + fileName_).c_str());  
			}
			mIgInbuf = new unsigned char[IGZIP_IN_BUF_SIZE];
			isal_gzip_header_init(&mIgzipHeader);
			isal_inflate_init(&mStream);
			mStream.crc_flag = ISAL_GZIP_NO_HDR_VER;

			mStream.next_in = mIgInbuf;
			mStream.avail_in = fread(mStream.next_in, 1, IGZIP_IN_BUF_SIZE, mFile);

			int ret =0;
			ret = isal_read_gzip_header(&mStream, &mIgzipHeader);
			if( ret != ISAL_DECOMP_OK ){
				cerr << "error invalid gzip header found: " << fileName_ << endl;
				if(mFile != NULL){
					fclose(mFile);
				}
				exit(-1);
			}
			this->isZipped = true;
		}else {
			mFile = FOPEN(fileName_.c_str(), "rb");
			if (fileName_ != "") {
				mFile = FOPEN(fileName_.c_str(), "rb");
				if (mFile == NULL)
					throw RioException("Can not open file to read: ");  
			}
			if (mFile == NULL) {
				throw RioException(
					("Can not open file to read: " + fileName_).c_str());  
			}
		}
	}

  FileReader(int fd, bool isZipped = false){
    if (isZipped) {

    } else {
      mFile = FDOPEN(fd, "rb");
      if (fd != -1) {
        mFile = FDOPEN(fd, "rb");
        if (mFile == NULL)
          throw RioException("Can not open file to read: ");  
      }
      if (mFile == NULL) {
        throw RioException("Can not open file to read: ");  
      }
    }
  }

  int64 igzip_read(FILE* zipFile, byte *memory_, size_t size_){
  	if(mStream.avail_in == 0){
  		mStream.next_in = mIgInbuf;
  		mStream.avail_in = fread(mStream.next_in, 1, IGZIP_IN_BUF_SIZE, zipFile);
  	}
  	mStream.next_out = memory_;;
  	mStream.avail_out = size_;;
  	int ret = isal_inflate(&mStream);
  	if(ret != ISAL_DECOMP_OK){
  		cerr << "decompress error" << endl;
  		return -1;
  	}
  	//cerr << "output size: " << (size_t)(mStream.next_out - memory_) << " size_: " << size_ <<  endl;
  	assert((size_t)(mStream.next_out - memory_) <= size_);
  	return (size_t)(mStream.next_out - memory_);
  }

  int64 Read(byte *memory_, uint64 size_) {
    if (isZipped) {
      //int64 n = gzread(mZipFile, memory_, size_);
			//cerr << "reading " << size_ << " byes" << endl;
 			int64 n = igzip_read(mFile, memory_, size_);
      if (n == -1) std::cerr << "Error to read gzip file" << std::endl;
      return n;
    } else {
      int64 n = fread(memory_, 1, size_, mFile);
      return n;
    }
  }

  bool Eof() const {
		if(eof) return eof;
		return feof(mFile);
	}
	void setEof(){
		eof = true;
	}

private:
	FILE* mFile = NULL;
	gzFile mZipFile = NULL;
	//igzip usage
	unsigned char *mIgInbuf = NULL;
	isal_gzip_header mIgzipHeader;
	inflate_state mStream;
	bool isZipped = false;
	bool eof = false;
};

} //namespace rabbit
