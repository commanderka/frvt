#include "frvtWrapper.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

int main(int arc, const char* argv[]){

    void* pImplPtr = nullptr;

    loadLibrary("libfrvt_1N_dermalog_008.so","_ZN7FRVT_1N9Interface17getImplementationEv",&pImplPtr);
    

    

    for (int nIteration = 0; nIteration<1000; nIteration++){

        std::cout<<"Iteration:"<<nIteration<<std::endl;
        void* pImageHandle = nullptr;
        std::ifstream is;
        is.open ("/mnt/d/coding/out.image", std::ios::binary );
        // get length of file:
        is.seekg (0, std::ios::end);
        int length = is.tellg();
        is.seekg (0, std::ios::beg);
        // allocate memory:
        std::vector<char> abBuffer;
        abBuffer.resize(length);
        // read data as a block:
        is.read (&abBuffer[0],length);
        is.close();

        createImageHandle(&pImageHandle);
        int res = loadImage(pImageHandle, 1024, 575, 24, (unsigned char*) &abBuffer[0], 0);
        std::cout<<"Error code:"<<res<<std::endl;
        destroyImageHandle(&pImageHandle);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    


}
