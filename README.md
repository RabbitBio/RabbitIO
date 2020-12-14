![RabbitIO](rabbitio.png)

RabbitIO: A Efficient and Easy-to-Use I/O Framework for Biological Sequence Data

## TODO
- [X] Add thread pool[y]
- [X] Add build scripts '2020/10/15'[h]
- [X] Add parameter parser[h]
- [X] Fasta dynamic parser[h]
- [ ] osx support
- [ ] document/manual[y]
- [ ] multi-decompress[c/w]
- [ ] process sra directly? 
- [X] delete fastqreader/fastareader class, only keep fastqFileReader 
- [ ] research io dence fastq/fastq data program
- [ ] fastq pair-end procession 


## Example 

``` bash 
cd RabbitIO
mkdir build && cd build
cmake ..
make
#then there is an test file in build file
time ./test -f /home/old_home/song/tool-test/R1.fq -t 10 
```

RabbitIO is about 2G/s I/O speed now
