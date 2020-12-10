/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss

struct config
{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

/* you can define the cache class here, or design your own data structure for L1 and L2 cache */
class cache
{
private:
    config cfg;
    unsigned long tag1, index1, tag2, index2;

public:
    cache(config cfg)
    {
        this->cfg = cfg;

        unsigned long offset1 = (unsigned long)log2(cfg.L1blocksize);
        unsigned long offset2 = (unsigned long)log2(cfg.L2blocksize);

        index1 = (unsigned long)(log2(cfg.L1size) + 10 - log2(cfg.L1setsize) - log2(cfg.L1blocksize));
        index2 = (unsigned long)(log2(cfg.L2size) + 10 - log2(cfg.L2setsize) - log2(cfg.L2blocksize));

        tag1 = 32 - offset1 - index1;
        tag2 = 32 - offset2 - index2;
    }

    vector<unsigned long> getParameters()
    {
        vector<unsigned long> parameters(4, 0);
        parameters[0] = tag1;
        parameters[1] = index1;
        parameters[2] = tag2;
        parameters[3] = index2;

        return parameters;
    }

    vector<vector<long> > getL1()
    {
        return vector<vector<long> >(cfg.L1setsize, vector<long>((long)pow(2, index1), -1));
    }

    vector<vector<long> > getL2()
    {
        return vector<vector<long> >(cfg.L2setsize, vector<long>((long)pow(2, index2), -1));
    }

    vector<vector<bool> > getValidBits1()
    {
        return vector<vector<bool> >(cfg.L1setsize, vector<bool>((unsigned long)pow(2, index1), false));
    }

    vector<vector<bool> > getValidBits2()
    {
        return vector<vector<bool> >(cfg.L2setsize, vector<bool>((unsigned long)pow(2, index2), false));
    }
};

int main(int argc, char *argv[])
{

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while (!cache_params.eof()) // read config file
    {
        cache_params >> dummyLine;
        cache_params >> cacheconfig.L1blocksize;
        cache_params >> cacheconfig.L1setsize;
        cache_params >> cacheconfig.L1size;
        cache_params >> dummyLine;
        cache_params >> cacheconfig.L2blocksize;
        cache_params >> cacheconfig.L2setsize;
        cache_params >> cacheconfig.L2size;
    }

    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like

    // full associative
    if (cacheconfig.L1setsize == 0) {
        cacheconfig.L1setsize = cacheconfig.L1size * pow(2, 10) / cacheconfig.L1blocksize;
    }

    if (cacheconfig.L2setsize == 0) {
        cacheconfig.L2setsize = cacheconfig.L2size * pow(2, 10) / cacheconfig.L2blocksize;
    }

    cache c(cacheconfig);

    vector<unsigned long> parameters = c.getParameters();

    vector<vector<long> > L1 = c.getL1();
    vector<vector<long> > L2 = c.getL2();

    vector<vector<bool> > validBits1 = c.getValidBits1();
    vector<vector<bool> > validBits2 = c.getValidBits2();

    int long setsize1 = cacheconfig.L1setsize;
    int long setsize2 = cacheconfig.L2setsize;

    vector<unsigned long> counter1(L1[0].size(), 0);
    vector<unsigned long> counter2(L2[0].size(), 0);

    int L1AcceState = 0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = 0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;

    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string("output.txt");

    traces.open(argv[2]);
    tracesout.open(outname.c_str());

    string line;
    string accesstype;     // the Read/Write access type from the memory trace;
    string xaddr;          // the address from the memory trace store in hex;
    unsigned int addr;     // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;

    if (traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        { // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr))
            {
                break;
            }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32>(addr);

            long tag1, index1, tag2, index2;

            tag1 = bitset<32>(
                    accessaddr
                    .to_string<char, std::string::traits_type, std::string::allocator_type>()
                    .substr(0, parameters[0]))
                    .to_ulong();
            index1 = bitset<32>(
                    accessaddr
                    .to_string<char, std::string::traits_type, std::string::allocator_type>()
                    .substr(parameters[0], parameters[1]))
                    .to_ulong();

            tag2 = bitset<32>(
                    accessaddr
                    .to_string<char, std::string::traits_type, std::string::allocator_type>()
                    .substr(0, parameters[2]))
                    .to_ulong();
            index2 = bitset<32>(
                    accessaddr
                    .to_string<char, std::string::traits_type, std::string::allocator_type>()
                    .substr(parameters[2], parameters[3]))
                    .to_ulong();

            bool l1Hits = false, l2Hits = false;

            for (int i = 0; i < setsize1 && !l1Hits; i++)
            {
                if (L1[i][index1] == tag1 && validBits1[i][index1])
                {
                    l1Hits = true;
                }
            }

            for (int i = 0; i < setsize2 && !l1Hits && !l2Hits; i++)
            {
                if (L2[i][index2] == tag2 && validBits2[i][index2])
                {
                    l2Hits = true;
                }
            }

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0)
            {
                //Implement by you:
                // read access to the L1 Cache,
                //  and then L2 (if required),
                //  update the L1 and L2 access state variable;
                if (l1Hits)
                {
                    L1AcceState = RH;
                    L2AcceState = NA;
                }
                else if (l2Hits)
                {
                    L1AcceState = RM;
                    L2AcceState = RH;

                    /* non inclusive / NINE:
                        copy the data from L2 to L1, update tag in L1
                        1. Even if L1[index] is dirty, discard the evicted data directly
                        2. L1[index] is not dirty, no more changes */
                    int setIdx1 = counter1[index1];
                    L1[setIdx1][index1] = (unsigned long)tag1;
                    validBits1[setIdx1][index1] = true;

                    counter1[index1] = (counter1[index1] + 1) % setsize1;
                }
                else
                {
                    L1AcceState = RM;
                    L2AcceState = RM;

                    // non inclusive / NINE, place in both L1 and L2 when both miss
                    // place in L2
                    int setIdx2 = counter2[index2];
                    for (int i = 0; i < setsize2 && L2[setIdx2][index2] != -1; i++)
                    {
                        counter2[index2] = (counter2[index2] + 1) % setsize2;
                        setIdx2 = counter2[index2];
                    }

                    L2[setIdx2][index2] = tag2;
                    validBits2[setIdx2][index2] = true;

                    counter2[index2] = (counter2[index2] + 1) % setsize2;

                    // place in L1
                    int setIdx1 = counter1[index1];
                    for (int i = 0; i < setsize1 && L1[setIdx1][index1] != -1; i++)
                    {
                        counter1[index1] = (counter1[index1] + 1) % setsize1;
                        setIdx1 = counter1[index1];
                    }

                    L1[setIdx1][index1] = tag1;
                    validBits1[setIdx1][index1] = true;

                    counter1[index1] = (counter1[index1] + 1) % setsize1;
                }
            }
            else
            {
                //Implement by you:
                // write access to the L1 Cache,
                //and then L2 (if required),
                //update the L1 and L2 access state variable;
                if (l1Hits)
                {
                    // write hit in L1, tag1 remains
                    // don't care about data
                    L1AcceState = WH;
                    L2AcceState = NA;
                }
                else if (l2Hits)
                {
                    L1AcceState = WM;
                    L2AcceState = WH;

                    // write back trigered in L2
                    // in this program's view, no nore changes needed.
                }
                else
                {
                    // no alocate -> forward write to next level
                    L1AcceState = WM;
                    L2AcceState = WM;
                }
            }

            tracesout << L1AcceState << " " << L2AcceState << endl; // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
