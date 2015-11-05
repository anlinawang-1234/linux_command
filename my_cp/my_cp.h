
#include <apue.h>
#include <libgen.h>

#define RWRWRW (S_IRUSR  | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#define BUFSIZE  (1024)
#define FILENUM  (64)
#define FILENAME (256)
#define P   (1)
#define R   (2)
#define FORP  ('p')
#define FORR  ('r')

void print_usage(char *str)               ;
void cp_property(char *src, char *tar)    ;
void cp_file_to_file(char *src, char *tar);
void cp_file_to_dir(char *src, char *dir) ;
void cp_dir_to_dir(char *src, char *tar)  ;
void *Malloc(size_t size)                 ;
void cp_type(void)                        ;
void do_work(int ac, char **av)           ;
