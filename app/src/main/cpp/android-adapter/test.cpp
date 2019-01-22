#include <Engine/StdH.h>
#include <android/log.h>
#include <Engine/Templates/DynamicContainer.h>
#include <Engine/Templates/DynamicContainer.cpp>
#include <Engine/Base/Synchronization.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/Stream.h>
#include <Engine/Base/Unzip.h>
#include <dirent.h>

#define  LOG_TAG    "seriousSamNative"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

void ciao() {
//    void *a = (void *) &UNZIPOpen_t;
//    CTFileStream stream;
//    UNZIPOpen_t("ciao");
    CTString str = "test 1 2 3";
    int a, b, c;
    int t = str.ScanF("test %i %i %i", &a, &b, &c);
    int asd = 0;


//    DIR *dir;
//    struct dirent *ent;
//    if ((dir = opendir("/data/user/0/com.github.aarcangeli.serioussamandroid")) != NULL) {
//        /* print all the files and directories within directory */
//        while ((ent = readdir(dir)) != NULL) {
//            printf("%s\n", ent->d_name);
//        }
//        closedir(dir);
//    } else {
//        int myError = errno;
//        int asdasd = 0;
//        /* could not open directory */
//    }

//    CDynamicContainer<int> prova;
//    prova.Add(new int{21});

//    CTCriticalSection section;

//    int t = 0;

//    CTimer timer;

//    CTString str;
//    str = "ciao";
//    str += " mondo";
//    int t = 0;
}
