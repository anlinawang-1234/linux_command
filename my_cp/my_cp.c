#include "my_cp.h"

char *file[FILENUM];
char *target;
int opt;

void print_usage(char *str)
{
    printf("Usage : %s <filename | dirname>... <dirname>\n", str);
}
void cp_property(char *src, char *tar)
{
    /*
     * First : use a struct to reserve the source mode & uid & gid &time
     * Second : use chmod chown & futimens funtion to change the target attribution
     * */
    struct stat src_buf,tar_buf;
    int fd = -1;
    mode_t tar_mode;
    uid_t tar_uid;
    gid_t tar_gid;
    struct timespec time[2];

    if(NULL == src || NULL == tar){
        return ;
    }
    if(0 ==  lstat(src, &src_buf)){
        tar_mode = src_buf.st_mode;
        tar_uid  = src_buf.st_uid;
        tar_gid  = src_buf.st_gid;
        time[0] = src_buf.st_atim;
        time[1] = src_buf.st_mtim;
    }
    chmod(tar, tar_mode);
    chown(tar, tar_uid, tar_gid);
    fd = open(tar, O_RDWR);
    if(0 > fd){
        printf("open '%s' error [%d]\n", tar, errno);
    }
    futimens(fd, time);
}
void cp_file_to_file(char *src, char *tar)
{
    int n = 0;
    char buf[BUFSIZE];
    int src_fd = -1;
    int tar_fd = -1;

    if(NULL == src || NULL == tar){
        return ;
    }
    src_fd = open(src, O_RDONLY);
    if(0 > src_fd){
        printf("open '%s' error [%d]\n", src, errno);
        exit(0);
    }
    /*
     * You can not justify this two fd at the same time 
     * because if one open failed you can't close the other
     * effectively
     * */
    tar_fd = open(tar, O_RDWR | O_CREAT, RWRWRW);
    if(0 > tar_fd){
        close(src_fd);
        printf("open '%s' error [%d]\n", tar, errno);
        exit(0);
    }
    if(read(tar_fd, buf, 1)){
        printf("my_cp : overwrite '%s'\n", tar);
        scanf("%s", buf);
        if(!strcmp("yes", buf) || !strcmp("y", buf) ||
           !strcmp("YES", buf) || !strcmp("Y", buf)){
            ;
        }else{
            close(tar_fd);
            close(src_fd);
            return ;
        }
    }
    memset(buf, '\0', BUFSIZE);
    while(0 < (n = read(src_fd, buf, BUFSIZE))){
        if(n != write(tar_fd, buf, n)){
            printf("write error [%d]\n", errno);
        }
    }
    if(P & opt){
        cp_property(src, tar);
    }
    if(0 < src_fd){
       close(src_fd);
    }
    if(0 < tar_fd){
        close(tar_fd);
    }

}
void cp_file_to_dir(char *src, char *dir)
{
    char buf[BUFSIZE];
    DIR *dp;

    if(NULL == src || NULL == dir){
        return ;
    }
    dp = opendir(dir);
    if(NULL == dp && errno == ENOTDIR){
        /*
         * if open directory failed & the error number show this directory 
         * does not exist  you should mkdir first
         * */
        mkdir(dir, RWRWRW);
    }
    memset(buf, '\0', BUFSIZE);
    strcpy(buf, dir);
    //strcat(buf, "/");
    strcat(buf, basename(src));
    cp_file_to_file(src, buf);
}
void cp_dir_to_dir(char *src, char *tar)
{
    struct dirent *dirp;
    struct stat buf;
    struct stat stat;
    DIR *dp;
    DIR *dp2;
    DIR *temp;
    int len = 0;
    char dirname[BUFSIZE];
    char tar_buf[BUFSIZE];
    char src_buf[BUFSIZE];

    if(NULL == src || NULL == tar){
        return ;
    }
    dp = opendir(src);
    if(NULL == dp){
        printf("open '%s' error [%d]\n", src, errno);
        exit(0);
    }
    strcpy(tar_buf, tar);
    strcat(tar_buf, basename(src));
    strcat(tar_buf, "/");
    dp2 = opendir(tar_buf);
    if(NULL == dp2){
        mkdir(tar_buf, RWRWRW);
    }
    while(NULL != (dirp = readdir(dp))){
        if(!strcmp(dirp->d_name, ".") ||
           !strcmp(dirp->d_name, "..")){
            continue;
        }
        /*To do target directory name */
        /*To do source directory name*/
        sprintf(src_buf, "%s/%s", src, dirp->d_name);
        if(0 == lstat(src_buf, &stat)){
            if(S_ISREG(stat.st_mode)){
                cp_file_to_dir(src_buf, tar_buf);
            }else if(S_ISDIR(stat.st_mode)){
                strcat(src_buf, "/");
                //sprintf(src_buf,"%s/", dirp->d_name);
                cp_dir_to_dir(src_buf, tar_buf);
            }
        }else{
            printf("mt_cp : faile to accesss '%s'\n", src_buf);
        }
    }
    closedir(dp);
}
void *Malloc(size_t size)
{
    void *ret = malloc(size);
    if(NULL == ret){
        fprintf(stderr, "Over flow\n");
        exit(0);
    }
}
void cp_type(void)
{
    int i = 0;
    int len = 0;
    struct stat dest_buf, src_buf;
    char buf[BUFSIZE];

    /* get target dirtectory/ file first
     * for if it does not exist, you can return error or make it 
     * */
    if(0 != lstat(target, &dest_buf)){
        len = strlen(target);
        if('/' == target[len - 1]){
            printf("my_cp : fail to access '%s' : Not a directory\n", target);
        }else{
             memset(buf, '\0', BUFSIZE);
             strcpy(buf, dirname(target));
             strcat(buf, "/");
             strcat(buf, target);
             if(0 > open(buf, O_RDWR | O_CREAT, RWRWRW)){
                 printf("open error [%d]\n", errno);
                 exit(0);
             }else{
                 if(0 != lstat(buf, &dest_buf)){
                     printf("get '%s' error [%d]\n", buf, errno);
                     exit(0);
                 }
             }
        }
    }
    /* maybe there are more than one file, so you should use loop*/
    while(file[i]){
        if(0 == lstat(file[i], &src_buf)){
            if(S_ISREG(src_buf.st_mode) && S_ISREG(dest_buf.st_mode)){
                cp_file_to_file(file[i], target);
            }else if(S_ISREG(src_buf.st_mode) && S_ISDIR(dest_buf.st_mode)){
                cp_file_to_dir(file[i], target);
            }else if(S_ISDIR(src_buf.st_mode) && S_ISDIR(dest_buf.st_mode)){
                if(R & opt){
                    cp_dir_to_dir(file[i], target);
                }else{
                    printf("my_cp : omitting directory '%s'\n", file[i]);
                }
            }else if(S_ISDIR(src_buf.st_mode) && S_ISREG(dest_buf.st_mode)){
                printf("my_cp :omiting directory '%s'\n", file[i]);
            }
        }else{
            printf("my_cp : cannot stat '%s' : No such file or directory\n", file[i]);
        }
        i++;
    }
}

void do_work(int ac, char **av)
{
    int len = 0;
    int i = 0;
    int j = 1;
    int loop = 0;

    if(3 > ac){
        print_usage(av[0]);
        exit(0);
    }
    for(loop = 1; loop < ac; ++loop){
        /* to justify if it is option 
         * if yes use  bit operater */
        if('-' == av[loop][0]){
            while(av[loop][j]){
                switch (av[loop][j]){
                    case FORP : opt |= P; break;
                    case FORR : opt |= R; break;
                    default : 
                             printf("my_cp : Invalid option -- '%c'\n", av[loop][j]);
                }
                j++;
            }
            continue;
        }
        len = strlen(av[loop]);
        file[i] = Malloc(len);
        file[i++] = av[loop];
    }
    target = Malloc(strlen(file[i - 1]));
    target = file[i - 1];
    file[i - 1] = 0;
    cp_type();
}

int main(int ac, char **av)
{
    do_work(ac, av);
    return 0;
}
