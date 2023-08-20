#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <filesystem>
#include <atomic>

using namespace std;
using namespace chrono;
namespace fs = std::filesystem;

const int ALLOWED_MEMORY_SIZE = 100000;
const int THREAD_NUMBER = 6;
const string PATH_TO_SOURCE{ "./sourceDir" };
const string PATH_TO_DEST{ "./destDir" };
const string TEMPLATE_FILE("./template.txt");

pthread_mutex_t lock;
atomic_int fileNumberCounter = 0;

//consumer funcs
string getNextFileName();
void *countLines(void *attr);

//producer funcs
string generateNextFileName();
void *generateFiles(void *attr);

int main()
{
    int rc;
    pthread_t threads[THREAD_NUMBER];

    for (int i = 0; i < THREAD_NUMBER; i++) {
        if (i < (THREAD_NUMBER / 2)) {
            rc = pthread_create(&threads[i], NULL, countLines, NULL);  
        } else {
            rc = pthread_create(&threads[i], NULL, generateFiles, NULL);  
        }
    }
    for (int i = 0; i < THREAD_NUMBER; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

string getNextFileName()
{
    fs::directory_iterator fileListIt;

    while (fs::is_empty(PATH_TO_SOURCE)){
        sleep(1);
    };

    fileListIt = fs::directory_iterator(PATH_TO_SOURCE);
    return fileListIt->path();
}

void *countLines(void *attr) {

    pthread_mutex_lock(&lock);
    string filePath = getNextFileName();
    //move to be counted file to dest location
    string fileName = filePath.substr(filePath.find_last_of('/'));
    string newFilePath = PATH_TO_DEST + fileName;
    fs::rename(filePath, newFilePath);
    cout << "[MOVED] " << fileName << ": Moved to " << PATH_TO_DEST << endl;

    int fileDescriptor = open(newFilePath.c_str(), O_SYNC | 0 | O_RDONLY);
    if (fileDescriptor == -1) {
        cout << "[ERROR] " << newFilePath << ": Read error" << endl;
    }
    pthread_mutex_unlock(&lock);

    int timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count();
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
    timer = duration_cast<milliseconds> (system_clock::now().time_since_epoch()).count() - timer;
    cout << "[COUNTED] " << newFilePath << ": " << count << " lines in "<< timer << "ms" << endl;

    countLines(NULL);
    return NULL;
}

string generateNextFileName()
{
    string autoGenName = "autogenFile_@@@.txt";
    fileNumberCounter++;
    autoGenName.replace(autoGenName.find("@@@"), 3, to_string(fileNumberCounter));
    return autoGenName;
}

void *generateFiles(void *attr)
{
    while(1) {
        string auoGenFile = PATH_TO_SOURCE + '/' + generateNextFileName();
        fs::copy(TEMPLATE_FILE, auoGenFile);
        cout << "[GENERATED] " << auoGenFile << endl;

        sleep(5);
    }

    return NULL;
}