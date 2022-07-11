#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#define FILE_NAME_LEN   255

#define GIGABYTE_IN_BYTES 1073741824.0
#define MEGABYTE_IN_BYTES 1048576.0
#define KILOBYTE_IN_BYTES 1024.0

typedef struct {
    char accessMode[11];
    int nLink;
    char uid[12];
    char gid[12];
    char size[16];
    char mTime[32];
    char fileName[FILE_NAME_LEN];
} fileProp_t;

fileProp_t *files;

int getNumFilesInDir(const char *dir);
void accessModeToStr(int mode, char *str);
char* uidToName(uid_t uid);
char *gidToName(gid_t gid);
void getFileProperties(fileProp_t *file, const char *dir);
void display(fileProp_t *files, int numOfFiles);

int f_humanize;
int f_longform;
int f_reversesort;

int getNumFilesInDir(const char *dir) {
    DIR * dirp;
    struct dirent * entry;

    int counter = 0;
    dirp = opendir(dir);
    while ((entry = readdir(dirp)) != NULL) {
        if (strcmp(entry->d_name, (const char*)".") && strcmp(entry->d_name, (const char*)"..")) {
            counter++;
        }
    }
    closedir(dirp);

    return counter;
}

void accessModeToStr(int mode, char *str) {

    strcpy(str, "----------");

    if (S_ISDIR(mode)) {
        str[0] = 'd';
    }
    if (S_ISCHR(mode)) {
        str[0] = 'c';
    }
    if (S_ISBLK(mode)) {
        str[0] = 'b';
    }
    if (mode & S_IRUSR) {
        str[1] = 'r';
    }
    if (mode & S_IWUSR) {
        str[2] = 'w';
    }
    if (mode & S_IXUSR) {
        str[3] = 'x';
    }
    if (mode & S_IRGRP) {
        str[4] = 'r';
    }
    if (mode & S_IWGRP) {
        str[5] = 'w';
    }
    if (mode & S_IXGRP) {
        str[6] = 'x';
    }
    if (mode & S_IROTH) {
        str[7] = 'r';
    }
    if (mode & S_IWOTH) {
        str[8] = 'w';
    }
    if (mode & S_IXOTH) {
        str[9] = 'x';
    }
}

#include <pwd.h>
char* uidToName(uid_t uid) {
    struct passwd *getwuid(), *pwPtr;
    static char numStr[12];

    if ((pwPtr = getpwuid(uid)) == NULL) {
        sprintf(numStr, "%d", uid);

        return numStr;
    } else {

        return pwPtr->pw_name;
    }
}

#include <grp.h>
char *gidToName(gid_t gid) {
    struct group *getgrgid(gid_t), *grpPtr;
    static char numStr[12];

    if ((grpPtr = getgrgid(gid)) == NULL) {
        sprintf(numStr, "%d", gid);

        return numStr;
    } else {

        return grpPtr->gr_name;
    }
}

void getFileProperties(fileProp_t *file, const char *path) {
    struct stat fileProperties;

    char *fullPathFileName = malloc(strlen(path) + strlen(file->fileName) + 1);
    strcpy(fullPathFileName, path);
    strcpy(fullPathFileName + strlen(path), file->fileName);

    if (stat(fullPathFileName, &fileProperties) == -1) {
        printf("get file properties error!\n");

        exit(EXIT_FAILURE);
    }
    accessModeToStr(fileProperties.st_mode, file->accessMode);                                      // accessMode
    file->nLink = fileProperties.st_nlink;                                                          // nLink
    strcpy(file->uid, uidToName(fileProperties.st_uid));                                            // uid
    strcpy(file->gid, gidToName(fileProperties.st_gid));                                            // gid
    sprintf(file->size, "%d", fileProperties.st_size);                                              // size
    strcpy(file->mTime, 4 + ctime(&(fileProperties.st_mtime)));                                     // mTime

    free(fullPathFileName);
}

int sf (const void *a, const void *b) {
    char fileName1[FILE_NAME_LEN];
    char fileName2[FILE_NAME_LEN];

    const char *aStr = ((fileProp_t*)a)->fileName;
    const char *bStr = ((fileProp_t*)b)->fileName;

    for (int i = 0; i < strlen(aStr); i++) {
        fileName1[i] = tolower(aStr[i]);
    }

    for (int i = 0; i < strlen(bStr); i++) {
        fileName2[i] = tolower(bStr[i]);
    }

    if (f_reversesort) {
        return strcmp(fileName2, fileName1);
    } else {
        return strcmp(fileName1, fileName2);
    }
}

void display(fileProp_t *files, int numOfFiles) {
    size_t fileSize;
    double dFileSize;

    if (f_humanize) {
        for (int i = 0; i < numOfFiles; i++) {
            fileSize = atoi(files[i].size);
            if (fileSize > GIGABYTE_IN_BYTES) {
                dFileSize = fileSize / GIGABYTE_IN_BYTES;
                sprintf(files[i].size, "%.1fG", dFileSize);
            } else if (fileSize > MEGABYTE_IN_BYTES) {
                dFileSize = fileSize / MEGABYTE_IN_BYTES;
                sprintf(files[i].size, "%.1fM", dFileSize);
            } else if (fileSize > KILOBYTE_IN_BYTES) {
                dFileSize = fileSize / KILOBYTE_IN_BYTES;
                sprintf(files[i].size, "%.1fK", dFileSize);
            }
        }
    }

    qsort(files, numOfFiles, sizeof(files[0]), sf);

    if (f_longform) {
        for (int i = 0; i < numOfFiles; i++) {
            printf("%s", files[i].accessMode);
            printf("%2d ", files[i].nLink);
            printf("%-4s ", files[i].uid);
            printf("%-4s", files[i].gid);
            printf("%8s ", files[i].size);
            printf("%.12s ", files[i].mTime);
            printf("%s\n", files[i].fileName);
        }
    } else {
        for (int i = 0; i < numOfFiles; i++) {
            printf("%s  ", files[i].fileName);
        }
        printf("\n");
    }
}

void usage(void) {

    printf("usage: ls [-hlr] [file ...]\n");

    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    DIR *dptr;
    struct dirent *ds;
    char *dir;
    int ch;

    if (argc == 1 || argc == 2) {
        dir = malloc(3);
        dir = "./";
    } else if (argc == 3) {
        dir = malloc(sizeof(char) * strlen(argv[2]) + 1);
        dir = argv[2];
    } else {
        usage();
    }

    while ((ch = getopt(argc, argv, "hlr")) != -1) {
        switch (ch) {
            case 'h':
                f_humanize = 1;
                break;
            case 'l':
                f_longform = 1;
                break;
            case 'r':
                f_reversesort = 1;
                break;

            default:
                usage();
        }
    }

    dptr = opendir(dir);
    if (dptr == NULL) {
        printf("open directory error!\n");

        exit(EXIT_FAILURE);
    }

    int numFilesInDir = getNumFilesInDir(dir);
    files = malloc(sizeof(fileProp_t) * numFilesInDir);

    int fileIndex = 0;
    while ((ds = readdir(dptr)) != 0) {

        if (strcmp(ds->d_name, (const char*)".") && strcmp(ds->d_name, (const char*)"..")) {
            strcpy(files[fileIndex].fileName, ds->d_name);
            getFileProperties(&files[fileIndex], dir);
            fileIndex++;
        }
    }
    closedir(dptr);

    display(files, numFilesInDir);

    free(files);

    return 0;
}
