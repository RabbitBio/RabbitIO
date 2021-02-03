#include <cstdio>
#include <vector>
#include <map>
#include <cstdio>

#include "Reference.h"
#include "Formater.h"
#include "Buffer.h"
#include "FastxStream.h"

using namespace std;

namespace rabbit {

namespace fa {

// FIXME:support enter = \n only
string getSequence(FastaDataChunk *&chunk, uint64 &pos) {
  int start_pos = pos;
  char *data = (char *)chunk->data.Pointer();
  // cerr << "start pos: " << pos << endl << flush;
  // cerr << "chunk size: " << chunk->size << endl << flush;
  // cerr << "data[pos]: " << (int)data[pos] << endl << flush;
  string res = "";

  while (pos < chunk->size - 1) {
    if (data[pos] == '\n') {
      res += string(data + start_pos, pos - start_pos);
      pos++;
      start_pos = pos;
      if (data[pos] == '>') return res;
    } else {
      pos++;
    }
  }

  // deal with last char
  if (pos == chunk->size - 1) {
    if (data[pos] == '\n')
      res += string(data + start_pos, pos - start_pos);
    else
      res += string(data + start_pos, pos - start_pos + 1);

    return res;
  }

  return "";
}

// only support uinx-like '\n'
string getLine(FastaDataChunk *&chunk, uint64 &pos) {
  int start_pos = pos;
  char *data = (char *)chunk->data.Pointer();

  while (pos < chunk->size) {
    if (data[pos] == '\n') {
      pos++;
      return string(data + start_pos, pos - start_pos - 1);
    } else {
      pos++;
    }
  }

  return "";
}

int chunkFormat(FastaChunk &fachunk, vector<Reference> &refs) {
  uint64 pos = 0;
  bool done = false;
  // cerr << "into chunkFormat" << endl;
  while (true) {
    Reference ref = getNextSeq(fachunk, done, pos);
    if (done) break;
    refs.push_back(ref);
  }

  ASSERT(refs.size() == fachunk.nseqs);

  return refs.size();
}

// design to filter out kmer apps
int chunkFormat(FastaChunk &fachunk, vector<Reference> &refs, int kmerSize) {
  uint64 pos = 0;
  bool done = false;
  // cerr << "into chunkFormat" << endl;
  uint64 short_count = 0;  // counter for seqs shorter than kmer
  while (true) {
    Reference ref = getNextSeq(fachunk, done, pos);
    if (done) break;
    if (ref.seq.length() < kmerSize)
      short_count++;
    else
      refs.push_back(ref);
  }

  ASSERT(refs.size() + short_count == fachunk.nseqs);

  return refs.size();
}

Reference getNextSeq(FastaChunk &fachunk, bool &done, uint64 &pos) {
  Reference ref;
  if (pos >= fachunk.chunk->size - 1) {
    done = true;
    return ref;
  }

  char *data = (char *)fachunk.chunk->data.Pointer();

  // while(data[pos] == '\n') pos++;//jump empty lines

  if (data[pos] != '>') {
    ref.seq = getSequence(fachunk.chunk, pos);
    ref.length = ref.seq.size();
    ref.gid = fachunk.start;
    fachunk.start++;
  } else {
    string line = getLine(fachunk.chunk, pos);
    int str_pos = line.find_first_of(' ');
    ref.name = line.substr(1, str_pos - 1);  // remove '>' and ' '
    if (str_pos < line.size()) ref.comment = line.substr(str_pos + 1);
    // cerr << "name: " << ref.name << endl << flush;
    ref.seq = getSequence(fachunk.chunk, pos);
    // cerr << "seq: " << ref.seq << endl << flush;
    ref.length = ref.seq.size();
    ref.gid = fachunk.start;
    fachunk.start++;
  }

  return ref;
}

}  // namespace fa

namespace fq {

void print_read(neoReference &ref) {
  std::cout << std::string((char *)ref.base + ref.pname, ref.lname) << std::endl;
  std::cout << std::string((char *)ref.base + ref.pseq, ref.lseq) << std::endl;
  std::cout << std::string((char *)ref.base + ref.pstrand, ref.lstrand) << std::endl;
  std::cout << std::string((char *)ref.base + ref.pqual, ref.lqual) << std::endl;
}

int chunkFormat(FastqChunk *&fqChunk, std::vector<neoReference> &data, bool mHasQuality = true) {
  FastqDataChunk *chunk = fqChunk->chunk;
  uint64_t seq_count = 0;
  uint64_t line_count = 0;
  uint64_t pos_ = 0;
  neoReference ref;
  while (true) {
    ref.base = chunk->data.Pointer();
    ref.pname = pos_;
    if (neoGetLine(chunk, pos_, ref.lname)) {
      ref.pseq = pos_;
    } else {
      break;
    }
    neoGetLine(chunk, pos_, ref.lseq);
    ref.pstrand = pos_;
    neoGetLine(chunk, pos_, ref.lstrand);
    ref.pqual = pos_;
    neoGetLine(chunk, pos_, ref.lqual);
    seq_count++;
    // print_read(ref);
    data.emplace_back(ref);
  }

  return seq_count;
}

int chunkFormat(FastqChunk* &fqChunk, std::vector<Reference> &data, bool mHasQuality = true){
	//format a whole chunk and return number of reads
	FastqDataChunk * chunk = fqChunk->chunk;
	int seq_count = 0;
	int line_count = 0;
	int pos_ = 0;
	Reference ref;

	while(true){
		string name = getLine(chunk, pos_);
		if(name.empty()) break;//dsrc guarantees that read are completed!
		//std::cerr << name << std::endl;

		string sequence = getLine(chunk, pos_);
		//std::cerr<< sequence << std::endl;

		string strand = getLine(chunk, pos_);
		//std::cerr << strand << std::endl;
		ref.name = name;
		ref.seq = sequence;
		ref.strand = strand;

		if(!mHasQuality){
			string quality = string(sequence.length(), 'K');
			//std::cerr << quality << std::endl;
			//data.push_back(new Read(name, sequence, strand, quality));
			//data.emplace_back(Sketch::Reference(name, "", sequence, strand, quality));
			ref.quality = quality;
			data.push_back(ref);
			seq_count++;

		}else{
			string quality = getLine(chunk, pos_);
			//std::cerr << quality << std::endl;
			//data.push_back(new Read(name, sequence, strand, quality));
			//data.emplace_back(Sketch::Reference(name, "", sequence, strand, quality));
			ref.quality = quality;
			data.push_back(ref);
			seq_count++;
		}
	}

	return seq_count;
}

string getLine(FastqDataChunk *&chunk, int &pos) {
  int start_pos = pos;
  char *data = (char *)chunk->data.Pointer();

  while (pos <= (chunk->size + 1)) {
    if (data[pos] == '\n' || data[pos] == '\r' || pos == (chunk->size + 1)) {
      // find a line
      pos++;
      return string(data + start_pos, pos - start_pos - 1);
    } else {
      pos++;
    }
  }
  return "";
}
int neoGetLine(FastqDataChunk *&chunk, uint64_t &pos, uint64_t &len) {
  int start_pos = pos;
  const char *data = (char *)chunk->data.Pointer();
  const uint64_t chunk_size = chunk->size + 1;
  /*
  while (pos <= chunk_size && data[pos] != '\n'){
    pos++;
  }
  len = pos - start_pos - 1;
  return len;
  */
  while (pos <= chunk_size) {
    if (data[pos] == '\n') {
      // find a line
      pos++;
      len = pos - start_pos - 1;
      return len;
    } else {
      pos++;
    }
  }
  return 0;
}
}  // namespace fq

}  // namespace rabbit
