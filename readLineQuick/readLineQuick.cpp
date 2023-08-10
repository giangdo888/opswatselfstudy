#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

using namespace std;
using namespace chrono;
const int ALLOWED_MEMORY_SIZE = 10000;

void getNextFileName(string &skeleton, int &fileNumberConter);

int main()
{
    int timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count();
    string skeleton;
    int fileNumberConter = 0;

    getNextFileName(skeleton, fileNumberConter);
    int fileDescriptor = open(skeleton.c_str(), O_SYNC | 0 | O_RDONLY);
    while(fileDescriptor != -1) {
        size_t length = lseek(fileDescriptor, 0, SEEK_END);
        int count = 0;
        int offset = 0;
        char buffer[ALLOWED_MEMORY_SIZE + 1];
        int size = ALLOWED_MEMORY_SIZE;

        while (1) {
            buffer[ALLOWED_MEMORY_SIZE] = {'\0'};
            pread(fileDescriptor, buffer, size, offset);
            char *occ = strchr(buffer, '\n');
            while (occ != NULL) {
                count++;
                occ = strchr(occ+1, '\n');
            }

            offset += ALLOWED_MEMORY_SIZE;
            if (offset + ALLOWED_MEMORY_SIZE >= length) {
                size = length - offset;
                buffer[size + 1] = {'\0'};
            }
            if (offset > length) {
                break;
            }
        }
        count++;
        cout << count <<" lines were counted from file " << skeleton << endl;
    
        getNextFileName(skeleton, fileNumberConter);
        fileDescriptor = open(skeleton.c_str(), O_SYNC | 0 | O_RDONLY);
    }
    timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count() - timer;
    cout << fileNumberConter - 1 << " files were counted in " << timer << "s" << endl;
    return 0;
}

void getNextFileName(string &skeleton, int &fileNumberConter)
{
    skeleton = "test_@@@.txt";
    fileNumberConter++;
    skeleton.replace(skeleton.find("@@@"), 3, to_string(fileNumberConter));
}