#include "bufferManager.hpp"

// constructor for Frame
Frame::Frame(){}

// copy constructor for Frame
Frame::Frame(const Frame &frame){
    this->page_Num = frame.page_Num;
    this->page_Data = new char[PAGE_SIZE];
    memcpy(this->page_Data, frame.page_Data, PAGE_SIZE);
    this->fp = frame.fp;
    this->pinned = frame.pinned;
    this->second_chance = frame.second_chance;
}

// populates a frame in Memory
void Frame::setFrame(FILE*fp, int page_Num, char* page_Data, bool pinned){
    this->page_Num = page_Num;
    this->page_Data = page_Data;
    this->fp = fp;
    this->pinned = pinned;
    this->second_chance = true;
}

// unpin a frame
void Frame::unpinFrame(){
    this->pinned = false;
}

// destructor for Frame
Frame::~Frame(){
    delete[] page_Data;
}


// constructor for LRUBufferManager
LRUBufferManager::LRUBufferManager(int num_Frames): num_Frames(num_Frames) {}

// destructor for LRUBufferManager
LRUBufferManager::~LRUBufferManager(){
    lru.clear();    // calls destructor of Frame so delete of pageData happens
    mp.clear();
}

// get a page from buffer
char* LRUBufferManager::getPage(FILE*fp, int page_Num){

    // check if page present in memory using map
    auto it = mp.find({fp, page_Num});
    if(it!=mp.end()){
        //check later
        stats.accesses+=1;
        stats.pageHits+=1;
        // page present in memory
        lru.push_front(*it->second);
        lru.erase(it->second);
        mp[{fp, page_Num}] = lru.begin();
        lru.begin()->pinned = true;
        return lru.begin()->page_Data;
    }
    // if page is not in memory
    // check if space is there in buffer

    if((int)lru.size() == num_Frames){

        // find last unpinned page and remove it
        auto it = lru.end();
        it--;
        while(it->pinned){
            if(it==lru.begin())return NULL;
            it--;
        }
        // remove page from buffer
        mp.erase({it->fp, it->page_Num});
        lru.erase(it);
    }

    // add the page to buffer
    char* page_Data = new char[PAGE_SIZE];
    fseek(fp, page_Num*PAGE_SIZE, SEEK_SET);
    fread(page_Data, PAGE_SIZE, 1, fp);

    Frame frame = Frame();
    frame.setFrame(fp, page_Num, page_Data, true);
    lru.push_front(frame);

    mp[{fp, page_Num}] = lru.begin();
    stats.accesses++;
    stats.diskreads++;
    char name[20];
    memcpy(name, page_Data, 20);
    return lru.begin()->page_Data;
}

// clear stats
void LRUBufferManager::clearStats(){
    stats.clear();
}

// get stats
BufStats LRUBufferManager::getStats(){
    return stats;
}

// constructor for BufStats
BufStats::BufStats(): accesses(0), diskreads(0), pageHits(0) {}

// clear stats
void BufStats::clear(){
    accesses = 0;
    diskreads = 0;
    pageHits = 0;
}


// unpin a page
void LRUBufferManager::unpinPage(FILE*fp, int page_Num){
    // check if page present in memory using map
    
    auto it = mp.find({fp, page_Num});
    //cout<<"check2\n";
    if(it != mp.end()){
        // page present in memory
        // unpin page
        //cout<<"check3\n";
        it->second->unpinFrame();
    }
    //cout<<"check4\n";
}

// constructor for ClockBufferManager
ClockBufferManager::ClockBufferManager(int num_Frames): num_Frames(num_Frames), clock_hand(0), num_Pages(0){
    bufferPool = new Frame[num_Frames];
}

// destructor for ClockBufferManager
ClockBufferManager::~ClockBufferManager(){
    delete[] bufferPool;
}

// get a page from buffer
char *ClockBufferManager::getPage(FILE* fp, int page_Num){


    // check if the page is present in memory
    for(int i=0;i<num_Pages;++i){
        if(bufferPool[i].fp == fp && bufferPool[i].page_Num == page_Num){
            // page is present in memory
            // update stats
            stats.accesses++;
            // update second chance
            bufferPool[i].second_chance = true;
            bufferPool[i].pinned = true;
            stats.pageHits++;
            return bufferPool[i].page_Data;
        }
    }

    // page is not present in memory
    if(num_Pages < num_Frames){
        fseek(fp, page_Num*PAGE_SIZE, SEEK_SET);
        char* page_Data = new char[PAGE_SIZE];
        fread(page_Data, PAGE_SIZE, 1, fp);
        bufferPool[num_Pages].setFrame(fp, page_Num, page_Data, true);
        num_Pages++;
        stats.accesses++;
        stats.diskreads++;
        return page_Data;
    }
    stats.accesses++;
    // page is not present in memory and memory is full
    while(true){
        if(bufferPool[clock_hand].second_chance){
            // page has second chance
            bufferPool[clock_hand].second_chance = false;
            clock_hand = (clock_hand+1)%num_Frames;
            continue;
        }
        if(bufferPool[clock_hand].pinned){
            // page is pinned
            clock_hand = (clock_hand+1)%num_Frames;
            continue;
        }
        // page is not pinned and does not have second chance
        // seek the page in file
        fseek(fp, page_Num*PAGE_SIZE, SEEK_SET);
        fread(bufferPool[clock_hand].page_Data, PAGE_SIZE, 1, fp);
        bufferPool[clock_hand].fp = fp;
        bufferPool[clock_hand].page_Num = page_Num;
        bufferPool[clock_hand].pinned = true;
        bufferPool[clock_hand].second_chance = true;
        int st = clock_hand;
        clock_hand = (clock_hand+1)%num_Frames;
        stats.diskreads++;
        return bufferPool[st].page_Data;
    }
}

// unpin a page
void ClockBufferManager::unpinPage(FILE* fp, int page_Num){
    
    // check if page is present in memory
    for(int i=0;i<num_Pages;++i){
        if(bufferPool[i].fp == fp && bufferPool[i].page_Num == page_Num){
            // page is present in memory
            // unpin page
            bufferPool[i].unpinFrame();
            return;
        }
    }
}

// clear stats
void ClockBufferManager::clearStats(){
    stats.clear();
}

// get stats
BufStats ClockBufferManager::getStats(){
    return stats;
}

// constructor for MRUBufferManager
MRUBufferManager::MRUBufferManager(int num_Frames): num_Frames(num_Frames) {}

// destructor for MRUBufferManager
MRUBufferManager::~MRUBufferManager(){
    mru.clear();      // the destructor of frame will be automatically called
    mp.clear();
}

// get a page from buffer
char *MRUBufferManager::getPage(FILE* fp, int page_Num){

    // check if page is present in memory
    auto it = mp.find({fp, page_Num});
    if(it!=mp.end()){
        stats.accesses++;
        // present so bring it to first and pin
        mru.push_front(*it->second);
        mru.erase(it->second);
        mp[{fp, page_Num}] = mru.begin();
        mru.begin()->pinned = true;
        stats.pageHits++;
        return mru.begin()->page_Data;
    }

    // not in memory, so check size
    if((int)mru.size() == num_Frames){

        int rmv = 0;
        for(auto it=mru.begin();it!=mru.end();++it){
            if(it->pinned){
                // page is pinned
                continue;
            }
            // page is not pinned
            // remove it from memory
            mp.erase({it->fp, it->page_Num});
            mru.erase(it);
            rmv = 1;
            break;
        }

        if(!rmv)return NULL;
    }

    // add the frame at start
    fseek(fp, page_Num*PAGE_SIZE, SEEK_SET);
    char* page_Data = new char[PAGE_SIZE];
    fread(page_Data, PAGE_SIZE, 1, fp);
    Frame frame = Frame();
        
    frame.setFrame(fp, page_Num, page_Data, true);
    mru.push_front(frame);
    mp[{fp, page_Num}] = mru.begin();
    stats.accesses++;
    stats.diskreads++;
    return mru.begin()->page_Data;
}


// unpin the frame

void MRUBufferManager::unpinPage(FILE *fp, int page_Num){

    // check if page is present in memory
    auto it = mp.find({fp, page_Num});
    if(it!=mp.end()){
        // page is present in memory
        // unpin page
        it->second->unpinFrame();
    }
}

void MRUBufferManager::clearStats(){
    stats.clear();
}

BufStats MRUBufferManager::getStats(){
    return stats;
}

// FIFOBufferManager::FIFOBufferManager(int num_Frames) : num_Frames(num_Frames) {
//     fifo.resize(num_Frames);
//     stats.clear();
//     for (int i = 0; i < num_Frames; ++i) {
//         fifo[i] = Frame(); // Assuming Frame has a default constructor
//     }
// }

// char* FIFOBufferManager::getPage(FILE* fp, int page_Num) {
//     auto key = make_pair(fp, page_Num);
//     if (mp.find(key) != mp.end()) { // Page already in buffer
//         stats.pageHits++;
//         return fifo[mp[key]].page_Data;
//     } else { // Page not in buffer
//         stats.diskreads++;
//         if (fifoQueue.size() == num_Frames) { // Buffer full, need to evict
//             auto evictKey = make_pair(fifo[fifoQueue.front()].fp, fifo[fifoQueue.front()].page_Num);
//             mp.erase(evictKey);
//             fifoQueue.pop();
//         }
//         FILE *f = fp;
//         char* data = new char[PAGE_SIZE];
//         fseek(f, PAGE_SIZE * page_Num, SEEK_SET);
//         fread(data, PAGE_SIZE, 1, f);
//         Frame frame;
//         frame.setFrame(f, page_Num, data, false);
//         fifo.push_back(frame);
//         mp[key] = fifo.size() - 1;
//         fifoQueue.push(fifo.size() - 1);
//         return data;
//     }
// }

// void FIFOBufferManager::unpinPage(FILE* fp, int page_Num) {
//     auto key = make_pair(fp, page_Num);
//     if (mp.find(key) != mp.end()) { // Page is in buffer
//         // Page is unpinned, do nothing for FIFO
//     }
// }

// BufStats FIFOBufferManager::getStats() {
//     return stats;
// }

// void FIFOBufferManager::clearStats() {
//     stats.clear();
// }

// FIFOBufferManager::~FIFOBufferManager() {
//     for (Frame frame : fifo) {
//         if (frame.pinned)
//             frame.unpinFrame();
//         delete[] frame.page_Data;
//     }
// }

FIFOBufferManager::FIFOBufferManager(int num_Frames) : num_Frames(num_Frames) {
    
    fifo.resize(num_Frames);
    for (int i = 0; i < num_Frames; ++i) {
        fifo[i] = Frame(); // Assuming Frame has a default constructor
    }
     cout<<"hello8"; 
}

void FIFOBufferManager::evictOldestPage() {
    if (!fifoQueue.empty()) {
        int oldest_frame_index = fifoQueue.front();
        fifoQueue.pop();
        cout<<"hello1";
        auto& frame = fifo[oldest_frame_index];
        auto key = make_pair(frame.fp, frame.page_Num);
        mp.erase(key); // Remove the entry from the map
    }
}

char* FIFOBufferManager::getPage(FILE* fp, int page_Num) {
     cout<<"hello7"; 
    auto key = make_pair(fp, page_Num);
    if (mp.find(key) != mp.end()) { // Page already in buffer
        stats.accesses++;
        stats.pageHits++;
        return fifo[mp[key]].page_Data;
    } else { // Page not in buffer
        stats.accesses++;
        stats.diskreads++;
        if (fifoQueue.size() == num_Frames) { // Buffer full, need to evict
            cout<<"hello6"; 
            evictOldestPage();
        }
        FILE *f = fp;
        char* data = new char[PAGE_SIZE];
        fseek(f, PAGE_SIZE * page_Num, SEEK_SET);
        if (fread(data, PAGE_SIZE, 1, f) != 1) {
            delete[] data; // Free allocated memory
            return nullptr; // Return nullptr on error
        }
        Frame frame;
        frame.setFrame(f, page_Num, data, false);
        fifo[fifoQueue.size()] = frame;
        mp[key] = fifoQueue.size();
        fifoQueue.push(fifoQueue.size());
       cout<<"hello5"; 
        return data;
    }
}

void FIFOBufferManager::unpinPage(FILE* fp, int page_Num) {
    auto key = make_pair(fp, page_Num);
    if (mp.find(key) != mp.end()) { // Page is in buffer
        // Page is unpinned, do nothing for FIFO
        cout<<"hello2";
    }
}

void FIFOBufferManager::clearStats() {
    stats.clear();
}

BufStats FIFOBufferManager::getStats() {
    return stats;
}

FIFOBufferManager::~FIFOBufferManager() {
    for (auto& frame : fifo) {
        delete[] frame.page_Data;
        cout<<"hello3";
    }
}

