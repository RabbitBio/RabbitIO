#include "io/FastxStream.h"
#include "io/FastxChunk.h"
#include <string>
#include <iostream>
#include "thirdparty/CLI11.hpp"
#include "io/DataQueue.h"
#include <thread>
#include "io/Formater.h"
#include <sys/time.h>
#include <cstdint>

double get_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000;
}

typedef rabbit::core::TDataQueue<rabbit::fq::FastqDataChunk> FqChunkQueue;
typedef int pfunc(int, char**);
struct Counter{
	uint64_t A, T, G, C;
};


int producer_se_fastq_task(std::string file, rabbit::fq::FastqDataPool *fastqPool, FqChunkQueue &dq) {
  rabbit::fq::FastqFileReader *fqFileReader;
  fqFileReader = new rabbit::fq::FastqFileReader(file, *fastqPool);
  int n_chunks = 0;
  int line_sum = 0;
  while (true) {
    rabbit::fq::FastqChunk *fqchunk = new rabbit::fq::FastqChunk;
    fqchunk->chunk = fqFileReader->readNextChunk();
    if (fqchunk->chunk == NULL) break;
    n_chunks++;
    //std::cout << "readed chunk: " << n_chunks << std::endl;
    dq.Push(n_chunks, fqchunk->chunk);
  }

  dq.SetCompleted();
  delete fqFileReader;
  return 0;
}

void consumer_se_fastq_task(rabbit::fq::FastqDataPool *fastqPool, FqChunkQueue &dq, Counter *counter) {
  long line_sum = 0;
  rabbit::int64 id = 0;
  rabbit::fq::FastqChunk *fqchunk = new rabbit::fq::FastqChunk;
  while (dq.Pop(id, fqchunk->chunk)) {
		std::vector<neoReference> data1;
		data1.resize(10000);
		rabbit::fq::chunkFormat((rabbit::fq::FastqDataChunk*)(fqchunk->chunk), data1, true);
		for(neoReference &read : data1){
			for(int i = 0; i < read.lseq; i++){
				switch(read.base[read.pseq + i]){
				case 'A':
					counter->A++; break;
				case 'T':
					counter->T++; break;
				case 'G':
					counter->G++; break;
				case 'C':
					counter->C++; break;
				}
			}
		}
    fastqPool->Release(fqchunk->chunk);
  }
}

int main(int argc, char **argv) {
  //std::string file1 = "/home/old_home/haoz/ncbi/public/sra/fastv_test/ERR3460962.sra.fastq";
  std::string file1 = argv[1];
  int th = std::stoi(argv[2]);  // thread number
  rabbit::fq::FastqDataPool *fastqPool = new rabbit::fq::FastqDataPool(256, 1 << 22);
  FqChunkQueue queue1(128, 1);
  std::thread producer(producer_se_fastq_task, file1, fastqPool, std::ref(queue1));
  std::thread **threads = new std::thread *[th];
	Counter* counters[th];
  for (int t = 0; t < th; t++) {
		counters[t] = new Counter{0,0,0,0};
    threads[t] = new std::thread(std::bind(consumer_se_fastq_task, fastqPool, std::ref(queue1), std::ref(counters[t])));
  }
  producer.join();
  for (int t = 0; t < th; t++) {
    threads[t]->join();
  }
	uint64_t ca = 0, ct = 0, cc = 0, cg = 0;
	for(int t = 0; t < th; t++){
		ca += counters[t]->A;
		ct += counters[t]->T;
		cc += counters[t]->C;
		cg += counters[t]->G;
	}
	std::cout << ca << " " << ct << " " << cc << " " << cg << std::endl;
 
  delete fastqPool;
  for (int t = 0; t < th; t++) {
    delete threads[t];
  }
  return 0;
}
