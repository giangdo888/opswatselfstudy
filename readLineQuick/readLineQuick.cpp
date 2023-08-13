#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>

using namespace std;
using namespace chrono;
const int ALLOWED_MEMORY_SIZE = 10000;
const int THREAD_NUMBER = 4;

int fileNumberCounter = 0;
pthread_mutex_t lock;
bool doneFlag = false;

string getNextFileName(int &fileNumberCounter);
void *countLines(void *attr);

int main()
{
    int timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count();
    string skeleton;
    int rc;

    pthread_t threads[THREAD_NUMBER];
    for (int i = 0; i < THREAD_NUMBER; i++) {
        rc = pthread_create(&threads[i], NULL, countLines, NULL);
    }
    for (int i = 0; i < THREAD_NUMBER; i++) {
        pthread_join(threads[i], NULL);
    }

    timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count() - timer;
    cout << fileNumberCounter - 1 << " files were counted in " << timer << "ms" << endl;
    return 0;
}

string getNextFileName(int &fileNumberCounter)
{
    string skeleton = "test_@@@.txt";
    fileNumberCounter++;
    skeleton.replace(skeleton.find("@@@"), 3, to_string(fileNumberCounter));
    return skeleton;
}

void *countLines(void *attr) {
    int timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count();
    int count = 0;
    int offset = 0;
    char buffer[ALLOWED_MEMORY_SIZE + 1];
    int size = ALLOWED_MEMORY_SIZE;

    pthread_mutex_lock(&lock);
    if (doneFlag == true) {
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
    }
    string skeleton = getNextFileName(fileNumberCounter);
    int fileDescriptor = open(skeleton.c_str(), O_SYNC | 0 | O_RDONLY);
    if (fileDescriptor == -1) {
        doneFlag = true;
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
    }
    pthread_mutex_unlock(&lock);
    size_t length = lseek(fileDescriptor, 0, SEEK_END);

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
    timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count() - timer;
    cout << count <<" lines were counted from file " << skeleton << " in " << timer << "ms by thread " << pthread_self() << endl;
    countLines(NULL);
}