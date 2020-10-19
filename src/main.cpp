#include "io/FastxIO.h"
#include "io/FastxStream.h"
#include "io/FastxChunk.h"
#include <string>
#include <iostream>

int count_line(mash::fq::FastqChunk* fqchunk){
    return 1000;
}

int main(){
    std::string file = "/home/old_home/haoz/workspace/QC/fastp_dsrc/out_1.fq";
    mash::fq::FastqDataPool *fastqPool = new mash::fq::FastqDataPool();
    mash::fq::FastqFileReader *fqFileReader;
    //mash::fq::FastqReader *fastqReader;
    fqFileReader = new mash::fq::FastqFileReader(file, *fastqPool, false);
    //fastqReader = new mash::fq::FastqReader(*fqFileReader, *fastqPool);  //没有必要再分fastqreader和fastareader了，只要上面的filereader是不同的类型就可以了。函数重载readnextchunk和
    int n_chunks = 0;
    int line_sum = 0;
    while(true){
        mash::fq::FastqChunk *fqchunk = new mash::fq::FastqChunk;
        fqchunk->chunk = fqFileReader->readNextChunk();
        if (fqchunk->chunk == NULL) break;
        n_chunks++;
        //line_sum += count_line(fqchunk);
        std::vector<Reference> data;
        line_sum += mash::fq::chunkFormat(fqchunk, data, true);
    }
    std::cout << "file " << file << " has " << line_sum << " lines" << std::endl;
}