![RabbitIO](rabbitio.png)

# RabbitIO: A Efficient and Easy-to-Use I/O Framework for Biological Sequence Data

## Installation

## Case study

- [RabbitTrim]()
- [RabbitQC]()
- [RabbitIO-MashScreen]()

## Runing Example in main.cpp and TestCount.cpp

``` bash 
cd RabbitIO
mkdir build && cd build
cmake ..
make
#then there is an test file in build file
time ./test 
time ./testcount
```

## FASTA data example 
this is an example of reading and processing FASTA file

- example code of using only one thread (count chunk number of input file):
``` c++
int proces_fasta_task(std::string file) {
  rabbit::fa::FastaDataPool *fastaPool = new rabbit::fa::FastaDataPool(256, 1 << 22);
  rabbit::fa::FastaFileReader *faFileReader;
  faFileReader = new rabbit::fa::FastaFileReader(file, *fastaPool, false);
  int n_chunks = 0;
  int line_sum = 0;
  while (true) {
    rabbit::fa::FastaChunk *fachunk = new rabbit::fa::FastaChunk;
    fachunk = faFileReader->readNextChunkList();
    if (fachunk == NULL) break;
    n_chunks++;
    //-----relaease
    rabbit::fa::FastaDataChunk *tmp = fachunk->chunk;
    do {
      fastaPool->Release(tmp);
      tmp = tmp->next;
    } while (tmp != NULL);
    // line_sum += rabbit::fa::chunkFormat(*fachunk, data);
  }
  std::cout << "file " << file << " has " << line_sum << " lines" << std::endl;
  return 0;
}
```

## FASTQ data example 

### Single-end data processing example

1. Example of define a mult-threading task

``` c++
int test_fastq_se(int argc, char** argv){
  std::string file = "/home/old_home/haoz/workspace/QC/out_1.fq";
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
  rabbit::fq::FastqDataPool *fastqPool = new rabbit::fq::FastqDataPool(32, 1<<22);
  rabbit::core::TDataQueue<rabbit::fq::FastqDataChunk> queue1(64, 1);
  std::thread producer(producer_fastq_task, filename, fastqPool, std::ref(queue1));
  std::thread** threads = new std::thread*[th];
  for(int t = 0; t < th; t++){
    threads[t] = new std::thread(std::bind(consumer_fastq_task, fastqPool, std::ref(queue1)));
  }
  producer.join();
  for(int t = 0; t < th; t++){
    threads[t]->join();
  }
  //-----free
  delete fastqPool;
  delete[] threads;
  return 0;
}
```
2. example of define producer and consumer task
``` c++
int producer_fastq_task(std::string file, rabbit::fq::FastqDataPool* fastqPool, rabbit::core::TDataQueue<rabbit::fq::FastqDataChunk> &dq){
  rabbit::fq::FastqFileReader *fqFileReader;
  fqFileReader = new rabbit::fq::FastqFileReader(file, *fastqPool);
  rabbit::int64 n_chunks = 0; 
  while(true){ 
	rabbit::fq::FastqDataChunk* fqdatachunk;// = new rabbit::fq::FastqDataChunk;
    fqdatachunk = fqFileReader->readNextChunk(); 
    if (fqdatachunk == NULL) break;
    n_chunks++;
    //std::cout << "readed chunk: " << n_chunks << std::endl;
    dq.Push(n_chunks, fqdatachunk);
  }
  dq.SetCompleted();
  std::cout << "file " << file << " has " << n_chunks << " chunks" << std::endl;
  return 0;
}

void consumer_fastq_task(rabbit::fq::FastqDataPool* fastqPool, rabbit::core::TDataQueue<rabbit::fq::FastqDataChunk> &dq){
    long line_sum = 0;
    rabbit::int64 id = 0;
    std::vector<neoReference> data;
	rabbit::fq::FastqDataChunk* fqdatachunk;// = new rabbit::fq::FastqDataChunk;
    data.resize(10000);
    while(dq.Pop(id, fqdatachunk)){
      line_sum += rabbit::fq::chunkFormat(fqdatachunk, data, true);
      fastqPool->Release(fqdatachunk);
    }
    std::cout << "line_sum: " << line_sum << std::endl;
}

```
RabbitIO is about 2G/s I/O speed now


## TODO

- [ ] support sam/bam file process
- [ ] support vcf file process
