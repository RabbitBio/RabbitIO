#include "io/FastxStream.h"
#include "io/FastxChunk.h"
#include <string>
#include <iostream>
#include "cmdparser/CLI11.hpp"
#include "io/DataQueue.h"
#include <thread>
#include "io/Formater.h"

typedef  mash::core::TDataQueue<mash::fq::FastqDataPairChunk> FqChunkQueue;

int count_line(mash::fq::FastqChunk* fqchunk){
    return 1000;
}

int producer_pe_fastq_task(std::string file, std::string file2, mash::fq::FastqDataPool* fastqPool, FqChunkQueue &dq){
    mash::fq::FastqFileReader *fqFileReader;
    //mash::fq::FastqReader *fastqReader;
    fqFileReader = new mash::fq::FastqFileReader(file, *fastqPool, file2, false);
    //fastqReader = new mash::fq::FastqReader(*fqFileReader, *fastqPool);  //没有必要再分fastqreader和fastareader了，只要上面的filereader是不同的类型就可以了。函数重载readnextchunk和
    int n_chunks = 0;
    int line_sum = 0;
    while(true){
        mash::fq::FastqPairChunk *fqchunk = new mash::fq::FastqPairChunk;
        fqchunk->chunk = fqFileReader->readNextPairChunk();
        if (fqchunk->chunk == NULL) break;
        n_chunks++;
        std::cout << "readed chunk: " << n_chunks << std::endl;
        dq.Push(n_chunks, fqchunk->chunk);
        //line_sum += count_line(fqchunk);
        //std::vector<neoReference> data;
        //data.resize(10000);
        //line_sum += mash::fq::chunkFormat(fqchunk, data, true);
        //fastqPool->Release(fqchunk->chunk);
    }
    
		dq.SetCompleted();
    delete fqFileReader;
		std::cout << "file " << file << " has " << n_chunks << " chunks" << std::endl;
    return 0;
}

/*
int producer_fastq_task(std::string file, mash::fq::FastqDataPool* fastqPool, FqChunkQueue &dq){
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
        std::cout << "readed chunk: " << n_chunks << std::endl;
        //dq.Push(n_chunks, fqchunk->chunk);
        //line_sum += count_line(fqchunk);
        //std::vector<neoReference> data;
        //data.resize(10000);
        //line_sum += mash::fq::chunkFormat(fqchunk, data, true);
        //fastqPool->Release(fqchunk->chunk);
    }
    dq.SetCompleted();
    std::cout << "file " << file << " has " << n_chunks << " chunks" << std::endl;
    return 0;
}
*/
void consumer_pe_fastq_task(mash::fq::FastqDataPool* fastqPool, FqChunkQueue &dq){
    long line_sum = 0;
    mash::int64 id = 0;
    std::vector<neoReference> data;
    mash::fq::FastqPairChunk *fqchunk = new mash::fq::FastqPairChunk;
    data.resize(10000);
    while(dq.Pop(id, fqchunk->chunk)){
	    line_sum +=1000;
        fastqPool->Release(fqchunk->chunk->left_part);
        fastqPool->Release(fqchunk->chunk->right_part);
    }
    std::cout << "line_sum: " << line_sum << std::endl;
}
/*
void consumer_fastq_task(mash::fq::FastqDataPool* fastqPool, FqChunkQueue &dq){
    long line_sum = 0;
    mash::int64 id = 0;
    std::vector<neoReference> data;
    mash::fq::FastqChunk *fqchunk = new mash::fq::FastqChunk;
    data.resize(10000);
    while(dq.Pop(id, fqchunk->chunk)){
			//line_sum += mash::fq::chunkFormat(fqchunk, data, true);
        fastqPool->Release(fqchunk->chunk);
    }
    std::cout << "line_sum: " << line_sum << std::endl;
}
*/
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
        //print_fachunkpart_info(fachunk);
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

int main(int argc, char** argv){
    std::string file1 = "/home/old_home/haoz/workspace/data/FD/bigr_1.fq";
    std::string file2 = "/home/old_home/haoz/workspace/data/FD/bigr_2.fq";
    int th = 1; // thread number
    mash::fq::FastqDataPool *fastqPool = new mash::fq::FastqDataPool(256, 1<<22);
    FqChunkQueue queue1(128, 1);
    std::thread producer(producer_pe_fastq_task, file1, file2, fastqPool, std::ref(queue1));
    std::thread** threads = new std::thread*[th];
    for(int t = 0; t < th; t++){
        threads[t] = new std::thread(std::bind(consumer_pe_fastq_task, fastqPool, std::ref(queue1)));
    }
    producer.join();
    for(int t = 0; t < th; t++){
        threads[t]->join();
    }
		delete fastqPool;
    for(int t = 0; t < th; t++){
        delete threads[t];
    }
		return 0;
}

/*
int main_1(int argc, char** argv){
    //std::string file = "/home/old_home/haoz/workspace/QC/fastp_dsrc/out_1.fq";
    std::string file = "/home/old_home/haoz/workspace/data/hg19/hg19.fa";
    //---------------cmd parser----------------
    CLI::App app{"Wellcome to RabbitIO"};
    CLI::Option* opt;
    std::string filename ;
    int th;
    app.add_option("-f, --file", filename, "input file name") 
        ->default_val(file);
    app.add_option("-t, --threads", th, "worktreads") 
        ->default_val(2);
    //----------------------------------------
    CLI11_PARSE(app, argc, argv);
    if(app.count("-f"))
        std::cout << "filename: " << filename << std::endl;
    else{
        std::cout << "-f not find, use default: " << filename << std::endl;

    }
    mash::fq::FastqDataPool *fastqPool = new mash::fq::FastqDataPool(32, 1<<22);
    FqChunkQueue queue1(64, 1);
    std::thread producer(producer_fastq_task, filename, fastqPool, std::ref(queue1));
    std::thread** threads = new std::thread*[th];
    //for(int t = 0; t < th; t++){
    //    threads[t] = new std::thread(std::bind(consumer_fastq_task, fastqPool, std::ref(queue1)));
    //}
    producer.join();
    //for(int t = 0; t < th; t++){
    //    threads[t]->join();
    //}

	//-----freee
	delete fastqPool;
    for(int t = 0; t < th; t++){
        delete threads[t];
    }
    /*
    FastqReader processer( );
    io::data::chunk<FastqChunk> fqchunk = processer.get_chunk(file);
    //or 
    io::data::seq<FastqSeq>  fqseqs = processer.get_formated_seq(file);

    processer.process_seq(worker_num, func, param);

    thread_pool.process(producer_task, file);
    thread_pool.process(consumer_task, file);
    
    return 0;
}*/
