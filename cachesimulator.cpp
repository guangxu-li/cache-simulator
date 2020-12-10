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
    unsigned long tag1, index1, offset1, tag2, index2, offset2;
    // vector<vector<unsigned long>> L1, validBits1, L2, validBits2;

public:
    cache(config cfg)
    {
        this->cfg = cfg;

        // offset1 = (unsigned long)log2(cfg.L1blocksize);
        offset2 = (unsigned long)log2(cfg.L2blocksize);

        index1 = (unsigned long)(log2(cfg.L1size) + 10 - log2(cfg.L1setsize) - log2(cfg.L1blocksize));
        index2 = (unsigned long)(log2(cfg.L2size) + 10 - log2(cfg.L2setsize) - log2(cfg.L2blocksize));

        tag1 = 32 - offset1 - index1;
        tag2 = 32 - offset2 - index2;

        /* 
         * initialize validBits and cache
         * m x n -> m sets x n blocks per set
         */

        // vector<vector<unsigned long>> valid1(cfg.L1setsize, vector<unsigned long>((unsigned long)pow(2, index1), 0));
        // vector<vector<unsigned long>> valid2(cfg.L2setsize, vector<unsigned long>((unsigned long)pow(2, index2), 0));
        // vector<vector<unsigned long>> l1(cfg.L1setsize, vector<unsigned long>((unsigned long)pow(2, index1), 0));
        // vector<vector<unsigned long>> l2(cfg.L1setsize, vector<unsigned long>((unsigned long)pow(2, index1), 0));

        // validBits1 = valid1;
        // validBits2 = valid2;
        // L1 = l1;
        // L2 = l2;
    }

    vector<unsigned long> getParameters()
    {
        vector<unsigned long> parameters{tag1, index1, tag2, index2};

        return parameters;
    }

    vector<vector<unsigned long>> getL1()
    {
        vector<vector<unsigned long>> L1(cfg.L1setsize, vector<unsigned long>((unsigned long)pow(2, index1), 0));

        return L1;
    }

    vector<vector<unsigned long>> getL2()
    {
        vector<vector<unsigned long>> L2(cfg.L1setsize, vector<unsigned long>((unsigned long)pow(2, index1), 0));

        return L2;
    }

    vector<vector<bool>> getValidBits1()
    {
        vector<vector<bool>> valid(cfg.L1setsize, vector<bool>((unsigned long)pow(2, index1), false));

        return valid;
    }

    vector<vector<bool>> getValidBits2()
    {
        vector<vector<bool>> valid(cfg.L2setsize, vector<bool>((unsigned long)pow(2, index2), false));

        return valid;
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

    // TODO: full associative
    cache c(cacheconfig);

    vector<unsigned long> parameters = c.getParameters();

    vector<vector<unsigned long>> L1 = c.getL1();
    vector<vector<unsigned long>> L2 = c.getL2();

    vector<vector<bool>> validBits1 = c.getValidBits1();
    vector<vector<bool>> validBits2 = c.getValidBits2();

    int long setsize1 = cacheconfig.L1setsize;
    int long setsize2 = cacheconfig.L2setsize;

    int rows1 = L1[0].size();
    int rows2 = L2[0].size();

    vector<unsigned long> counter1(rows1, 0);
    vector<unsigned long> counter2(rows2, 0);

    int L1AcceState = 0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = 0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;

    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";

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

            tag1 = bitset<32>(accessaddr.to_string().substr(0, parameters[0])).to_ulong();
            index1 = bitset<32>(accessaddr.to_string().substr(parameters[0], parameters[1])).to_ulong();

            tag2 = bitset<32>(accessaddr.to_string().substr(0, parameters[2])).to_ulong();
            index2 = bitset<32>(accessaddr.to_string().substr(parameters[2], parameters[3])).to_ulong();

            bool l1Hits = false,
                 l2Hits = false;

            for (int i = 0; i < setsize1 && !l1Hits; i++)
            {
                if (L1[i][index1] == tag1 && validBits1[i][index1])
                {
                    l1Hits = true;
                }
            }

            for (int i = 0; i < setsize2 && !l1Hits && !l2Hits; i++)
            {
                if (L2[i][index2] == tag2)
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

                    // TODO: read miss in L1 and read hit in L2
                }
                else
                {
                    L1AcceState = RM;
                    L2AcceState = RM;

                    // TODO: read miss in L1 and read miss in L2
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
                    L1AcceState = WH;
                    L2AcceState = NA;
                }
                else if (l2Hits)
                {
                    L1AcceState = WM;
                    L2AcceState = WH;
                }
                else
                {
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
