#include "io/FastxIO.h"
#include "io/FastxStream.h"
#include "io/FastxChunk.h"
#include <string>
#include <iostream>

int count_line(mash::fq::FastqChunk* fqchunk){
    return 1000;
}

int producer_fastq_task(std::string file){
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
    return 0;
}

void print_chunk(mash::fa::FastaDataChunk *chunk){
    std::cout << "chunk size: " << chunk->size << std::endl;
    std::cout << "chunk head: " << std::string((char*)chunk->data.Pointer(), 100) << std::endl;
}
void print_fachunkpart_info(mash::fa::FastaChunk *fachunk){
    std::cout << "------------------chunk info-----------------" << std::endl;
    print_chunk(fachunk->chunk);
    mash::fa::FastaDataChunk *current_chunk = fachunk->chunk;
    while(current_chunk->next != NULL){
        std::cout << "next" << std::endl;
        current_chunk = current_chunk->next;
        print_chunk(current_chunk);
    }
}

int producer_fasta_task(std::string file){
    mash::fa::FastaDataPool *fastaPool = new mash::fa::FastaDataPool();
    mash::fa::FastaFileReader *faFileReader;
    //mash::fq::FastqReader *fastqReader;
    faFileReader = new mash::fa::FastaFileReader(file, *fastaPool, false);
    //fastqReader = new mash::fq::FastqReader(*fqFileReader, *fastqPool);  //没有必要再分fastqreader和fastareader了，只要上面的filereader是不同的类型就可以了。函数重载readnextchunk和
    int n_chunks = 0;
    int line_sum = 0;
    while(true){
        mash::fa::FastaChunk *fachunk = new mash::fa::FastaChunk;
        fachunk = faFileReader->readNextChunkList();
        //fachunk = faFileReader->readNextChunk();
        if (fachunk == NULL) break;
        n_chunks++;
        //line_sum += count_line(fqchunk);
        std::vector<Reference> data;
        print_fachunkpart_info(fachunk);
        //-----relaease
        mash::fa::FastaDataChunk * tmp = fachunk->chunk;
        do{
            fastaPool->Release(tmp);
            tmp = tmp->next;
        }while(tmp != NULL);
        //------release
        //line_sum += mash::fa::chunkFormat(*fachunk, data);
    }
    std::cout << "file " << file << " has " << line_sum << " lines" << std::endl;
    return 0;

    //result record: readnextchunklist: 2.85//3.25
    //               readnextchunk:     2.76
}
int main(){
    //std::string file = "/home/old_home/haoz/workspace/QC/fastp_dsrc/out_1.fq";
    std::string file = "/home/old_home/haoz/workspace/data/hg19/hg19.fa";
    producer_fasta_task(file);
    /*
    FastqReader processer( );
    io::data::chunk<FastqChunk> fqchunk = processer.get_chunk(file);
    //or 
    io::data::seq<FastqSeq>  fqseqs = processer.get_formated_seq(file);

    processer.process_seq(worker_num, func, param);

    thread_pool.process(producer_task, file);
    thread_pool.process(consumer_task, file);
    */
    return 0;
}
