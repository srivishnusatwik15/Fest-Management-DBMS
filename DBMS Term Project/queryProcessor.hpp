#ifndef __QUERY_PROCESSOR_H_
#define __QUERY_PROCESSOR_H_

#include "bufferManager.hpp"
using namespace std;    

#define LRU 1
#define MRU 2
#define CLOCK 3
#define FIFO 4
#define PAGE_SIZE 4096

// process a select from query
typedef struct {
    int empID;
    char name[20];
    int age;
    int salary;
    int deptID; // Department ID will be the common column
} Employee;

// Define the Department structure
typedef struct {
    int deptID;
    char name[20];
    char location[20];
} Department;


class QueryProcessor{
    private:
   
    ReplacementPolicy* bufferManager;
    public:
     int numFrames;
    QueryProcessor(int num_Frames, int replacementPolicy);
    void processSelectQuery(FILE *fp, int col1, string value);
    void processJoinQuery(FILE *fp1, FILE *fp2, int col1, int col2);
    void process_selfJoinQuery(FILE *fp1,FILE*fp2, int col1,int col2);
};


#endif