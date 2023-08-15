#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <filesystem>

using namespace std;
using namespace chrono;
namespace fs = std::filesystem;

const int ALLOWED_MEMORY_SIZE = 100000;
const int THREAD_NUMBER = 4;
const string pathToSourceFiles{ "./sourceFiles" };
const string pathToDestFiles{ "./destFiles" };

pthread_mutex_t lock;
bool doneFlag = false;

string getNextFileName();
void *countLines(void *attr);

int main()
{
    int rc;
    pthread_t threads[THREAD_NUMBER];

    for (int i = 0; i < THREAD_NUMBER; i++) {
        rc = pthread_create(&threads[i], NULL, countLines, NULL);
    }
    for (int i = 0; i < THREAD_NUMBER; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

string getNextFileName()
{
    fs::directory_iterator fileListIt;

    while (fs::is_empty(pathToSourceFiles)){};

    fileListIt = fs::directory_iterator(pathToSourceFiles);
    return fileListIt->path();
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

    string filePath = getNextFileName();
    string newFilePath = pathToDestFiles + filePath.substr(filePath.find_last_of('/'));
    fs::rename(filePath, newFilePath);

    string fileName = filePath.substr(filePath.find_last_of('/')+1);
    int fileDescriptor = open(fileName.c_str(), O_SYNC | 0 | O_RDONLY);
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
    cout << newFilePath << " has " << count << " lines. Counted in "<< timer << "ms by thread " << pthread_self() << endl;

    countLines(NULL);
    return NULL;
}