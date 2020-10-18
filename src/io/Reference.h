#ifndef __REFERENCE_H_
#define __REFERENCE_H_

#include <string> 
#include <vector>

struct Reference{
    std::string name;
    std::string comment;
    std::string seq;
    std::string quality;
    std::string strand;
    uint64_t length;
    uint64_t gid;
};

typedef std::vector<Reference> SeqInfos;
typedef Reference OneSeqInfo;


#endif