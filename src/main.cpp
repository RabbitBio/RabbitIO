#include "io/FastxIO.h"
#include "io/FastxStream.h"
#include "io/FastxChunk.h"
#include <string>
#include <iostream>

int count_line(mash::fq::FastqChunk* fqchunk){
    return 1000;
}

int main(int argc, char ** argv){
    //std::string file = "/home/old_home/haoz/workspace/QC/fastp_dsrc/out_1.fq";
    if(argc <= 1)
	{
		std::cout << "Please specify filename. Example:" << std::endl;
		std::cout << "./RabbitIO filename " << std::endl;
		exit(0);
	}
	std::string file(argv[1]);
    mash::fq::FastqDataPool *fastqPool = new mash::fq::FastqDataPool();
    mash::fq::FastqFileReader *fqFileReader;
    mash::fq::FastqReader *fastqReader;
    fqFileReader = new mash::fq::FastqFileReader(file, false);
    fastqReader = new mash::fq::FastqReader(*fqFileReader, *fastqPool);  //没有必要再分fastqreader和fastareader了，只要上面的filereader是不同的类型就可以了。函数重载readnextchunk和
    int n_chunks = 0;
    int line_sum = 0;
    while(true){
        mash::fq::FastqChunk *fqchunk = new mash::fq::FastqChunk;
        fqchunk->chunk = fastqReader->readNextChunk();
        if (fqchunk->chunk == NULL) break;
        n_chunks++;
        //line_sum += count_line(fqchunk);
        std::vector<Reference> data;
        line_sum += mash::fq::chunkFormat(fqchunk, data, true);
		fastqPool -> Release(fqchunk->chunk);
    }
    std::cout << "file " << file << " has " << line_sum << " seqs" << std::endl;
}
