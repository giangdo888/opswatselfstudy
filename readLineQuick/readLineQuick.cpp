#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
//#include <string>
//#include <algorithm>
using namespace std;
using namespace chrono;

int main()
{
    int timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count();
    int file_write = open("test.txt", O_CREAT | O_RDWR, 
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    posix_fallocate(file_write, 0, 7000000);

    auto first = reinterpret_cast<char*>(
        mmap(NULL, 7000000, PROT_READ, MAP_SHARED, file_write, 0)
    );

    // string fileString = first;
    // char key = '\n';
    // int count = count_if(fileString.begin(), fileString.end(), [&key] (char fromString) {
    //     return key == fromString;
    // });

    int count = 0;
    char *occ = strchr(first, '\n');
    while (occ != NULL) {
        count++;
        occ = strchr(occ+1, '\n');
    }
    timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count() - timer;
    cout << count <<" were counted in " << timer << endl;
    return 0;
}