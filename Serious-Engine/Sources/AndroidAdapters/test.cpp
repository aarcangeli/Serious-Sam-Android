#include <Engine/StdH.h>
#include <android/log.h>
#include <Engine/Templates/DynamicContainer.h>
#include <Engine/Templates/DynamicContainer.cpp>
#include <Engine/Base/Synchronization.h>
#include <Engine/Base/FileName.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/Stream.h>
#include <Engine/Base/Unzip.h>
#include <Engine/Engine.h>
#include <dirent.h>

void ciao() {
  char buffer[500];

  stringFormatter::format(TRANS("(%s/%s) %s - ZLIB error: %s - %s"), *buffer);

  CTStream::EnableStreamHandling();

  static bool end = false;
  if (end) return;
  end = true;

  CTString str = "test 1 2 3";
  int a, b, c;
  int t = str.ScanF("test %i %i %i", &a, &b, &c);
  int asd = 0;

  // init phase
//    SE_EndEngine();

  try {
    UNZIPReadDirectoriesReverse_t();
  } catch (char *strError) {
    // report warning
    CPrintF(TRANS("There were group file errors:\n%s"), strError);
    return;
  }

  // read zip structure

  // already done in SE_InitEngine
//    _pConsole = new CConsole;

  CTFileName path = CTString("Data/Credits.txt");
  // test file system

  try {
    CTFileStream reader;
    reader.Open_t(path);

    while (!reader.AtEOF()) {
      CTString strLine;
      reader.GetLine_t(strLine);
      InfoMessage("credit: %s\n", strLine.str_String);
    }

    reader.Close();

  } catch (char *err) {
    WarningMessage("Cannot load '%s': '%s'\n", path.str_String, err);
    end = true;
  }

  CTStream::DisableStreamHandling();

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
