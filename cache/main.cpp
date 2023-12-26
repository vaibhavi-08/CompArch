//cache size 32kB
//40bit memory address
//block size 64kB
//4 ways
//set associative cache with write-allocate and write-back and lru replacement policy
#include <bits/stdc++.h>
using namespace std;
class Block{
public:
    int tag;
    bool isfull;
    string state;
    long long lruCounter;
    int data[64];
    Block() : lruCounter(0) {}
};
int mem[100000000];
int memoryFetch(int address){
    return mem[address];
}
void memoryUpdate(int address,int data[64]){
    address=(address/64)*64;
    for(int i=0;i<64;i++){
        mem[address+i]=data[i];
    }
}
int main(){
    Block blocks[128][4];//cache
    long long fakeClock=0;
    bool isContinue=true;
    while(isContinue){
        cout << "put 0 for read and 1 for write" << endl;
        int rw;
        cin >> rw;
        vector<pair<int,string>> mshr ;
        if(rw==0){
            //read
            int ans;
            int address;
            cin >> address;
            int index=(address/64)%128;
            int CPUAddressTag=address/8192;
            int blockOffset=address%64;
            bool tagMatch=false;
            int blockNo=-1;
            for(int i=0;i<4;i++){
                if(blocks[index][i].isfull&&blocks[index][i].tag==CPUAddressTag){
                    tagMatch=true;
                    blockNo=i;
                    break;
                }
            }
            if(tagMatch){
                if(blocks[index][blockNo].state=="v"||blocks[index][blockNo].state=="m"){
                    ans=blocks[index][blockNo].data[blockOffset];//hit
                    fakeClock++;
                    blocks[index][blockNo].lruCounter=fakeClock;
                }
                else if(blocks[index][blockNo].state=="mp"){
                    mshr.push_back(make_pair(address,"pending"));
                    ans= memoryFetch(address);
                }
                else{
                    cout << "this condition shouldn't arise according to our code" << endl;
                    ans=-1;
                }
            }
            else{
                int blcNo=-1;
                for(int i=0;i<4;i++){
                    if(!blocks[index][i].isfull){
                        blcNo=i;
                        break;
                    }
                }
                if(blcNo==-1){
                    //LRU replacement
                    long long min=1e18;
                    int blockToBeReplaced=-1;
                    for(int i=0;i<4;i++){
                        if(blocks[index][i].isfull && blocks[index][i].lruCounter<min){
                            min=blocks[index][i].lruCounter;
                            blockToBeReplaced=i;
                        }
                    }
                    //if block modified
                    if(blocks[index][blockToBeReplaced].state=="m"){
                        memoryUpdate(address,blocks[index][blockToBeReplaced].data);
                    }
                    //replacing the block
                    blocks[index][blockToBeReplaced].tag=CPUAddressTag;
                    blocks[index][blockToBeReplaced].state="mp";
                    for(int i=0;i<64;i++){
                        blocks[index][blockToBeReplaced].data[i]= memoryFetch((address/64)*64+i);
                    }
                    blocks[index][blockToBeReplaced].state="v";
                    fakeClock++;
                    blocks[index][blockToBeReplaced].lruCounter=fakeClock;
                    ans=blocks[index][blockToBeReplaced].data[blockOffset];

                }
                else{
                    //updating block data:
                    blocks[index][blcNo].tag=CPUAddressTag;
                    blocks[index][blcNo].state="mp";
                    for(int i=0;i<64;i++){
                        blocks[index][blcNo].data[i]= memoryFetch((address/64)*64+i);
                    }
                    blocks[index][blcNo].state="v";
                    blocks[index][blcNo].isfull=true;
                    fakeClock++;
                    blocks[index][blcNo].lruCounter=fakeClock;
                    ans=blocks[index][blcNo].data[blockOffset];
                }
            }
            cout << ans << endl;
        }
        else if(rw==1){
            //write back with write allocate
            int address,data;
            cin >> address;
            cin >> data;
            int index=(address/64)%128;
            int CPUAddressTag=address/8192;
            int blockOffset=address%64;
            bool tagMatch=false;
            int blockNo=-1;
            for(int i=0;i<4;i++){
                if(!blocks[index][i].isfull&&blocks[index][i].tag==CPUAddressTag){
                    tagMatch=true;
                    blockNo=i;
                    break;
                }
            }
            if(tagMatch){
                if(blocks[index][blockNo].state=="v"||blocks[index][blockNo].state=="m"){
                    blocks[index][blockNo].data[blockOffset]=data;
                    blocks[index][blockNo].state="m";
                    fakeClock++;
                    blocks[index][blockNo].lruCounter=fakeClock;
                }
                else{
                    cout << "this condition shouldn't arise according to our code in write" << endl;
                }
            }
            else{
                int blcNo=-1;
                for(int i=0;i<4;i++){
                    if(!blocks[index][i].isfull){
                        blcNo=i;
                        break;
                    }
                }
                if(blcNo==-1){
                    //LRU replacement
                    long long min=1e18;
                    int blockToBeReplaced=-1;
                    for(int i=0;i<4;i++){
                        if(blocks[index][i].isfull && blocks[index][i].lruCounter<min){
                            min=blocks[index][i].lruCounter;
                            blockToBeReplaced=i;
                        }
                    }
                    //if block modified
                    if(blocks[index][blockToBeReplaced].state=="m"){
                        memoryUpdate(address,blocks[index][blockToBeReplaced].data);
                    }
                    //replacing the block
                    blocks[index][blockToBeReplaced].tag=CPUAddressTag;
                    blocks[index][blockToBeReplaced].state="mp";
                    for(int i=0;i<64;i++){
                        blocks[index][blockToBeReplaced].data[i]= memoryFetch((address/64)*64+i);
                    }
                    fakeClock++;
                    blocks[index][blockToBeReplaced].lruCounter=fakeClock;
                    blocks[index][blockToBeReplaced].data[blockOffset]=data;
                    blocks[index][blockToBeReplaced].state="m";

                }
                else{
                    //updating block data:
                    blocks[index][blcNo].tag=CPUAddressTag;
                    blocks[index][blcNo].state="mp";
                    for(int i=0;i<64;i++){
                        blocks[index][blcNo].data[i]= memoryFetch((address/64)*64+i);
                    }
                    blocks[index][blcNo].isfull=true;
                    fakeClock++;
                    blocks[index][blcNo].lruCounter=fakeClock;
                    blocks[index][blcNo].data[blockOffset]=data;
                    blocks[index][blcNo].state="m";
                }

            }

        }
        else{
            cout << "invalid operation" << endl;
        }
        bool check;
        cout << "0 to break and 1 to continue" << endl;
        cin >> check;
        if(!check){
            isContinue=false;
        }

    }




    return 0;
}