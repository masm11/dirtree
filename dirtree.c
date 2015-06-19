#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/fcntl.h>

struct dir_t {
    struct dir_t *parent;
    char next[1024], cur[1024];
};

static void print_tree_iter(struct dir_t *cur, char *buf)
{
    if (cur != NULL) {
	print_tree_iter(cur->parent, buf);
	strcat(buf, cur->next[0] == '\0' ? "  " : "| ");
    }
}

static void print_tree(struct dir_t *cur)
{
    char buf[1024] = { 0, };
    print_tree_iter(cur->parent, buf);
    strcat(buf, cur->next[0] == '\0' ? "`-" : "+-");
    printf("%s", buf);
}

static void iter(struct dir_t *parent, const char *name)
{
    struct dir_t curdir;
    int fd;
    
    curdir.parent = parent;
    curdir.next[0] = '\0';
    
    fd = open(".", O_RDONLY);
    if (fd == -1) {
	perror(".");
	return;
    }
    
    if (chdir(name) == -1) {
	perror(name);
	close(fd);
	return;
    }
    
    DIR *dir = opendir(".");
    if (dir == NULL) {
	perror(".");
	close(fd);
	return;
    }
    
    {
	struct dirent *dp = readdir(dir);
	strcpy(curdir.next, dp->d_name);
    }
    
    while (1) {
	struct dirent *dp = readdir(dir);
	
	strcpy(curdir.cur, curdir.next);
	if (dp != NULL)
	    strcpy(curdir.next, dp->d_name);
	else
	    curdir.next[0] = '\0';
	
	const char *name = curdir.cur;
	
	if (name[0] == '\0')
	    break;
	
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
	    continue;
	
	char info[1024] = { 0, };
	
	int isdir = 0;
	
	struct stat st;
	if (lstat(name, &st) == -1) {
	    strcat(info, "???? ????");
	} else {
	    if (S_ISREG(st.st_mode)) {
		strcat(info, "FILE ");
	    } else if (S_ISDIR(st.st_mode)) {
		strcat(info, "DIR  ");
		isdir = 1;
	    } else if (S_ISCHR(st.st_mode)) {
		strcat(info, "CHR  ");
	    } else if (S_ISBLK(st.st_mode)) {
		strcat(info, "BLK  ");
	    } else if (S_ISFIFO(st.st_mode)) {
		strcat(info, "FIFO ");
	    } else if (S_ISLNK(st.st_mode)) {
		strcat(info, "LINK ");
	    } else if (S_ISSOCK(st.st_mode)) {
		strcat(info, "SOCK ");
	    } else {
		strcat(info, "???? ");
	    }
	    
	    char buf[128];
	    sprintf(buf, "%04o ", st.st_mode & 07777);
	    strcat(info, buf);
	}
	
	strcat(info, name);
	
	print_tree(&curdir);
	printf("%s\n", info);
	
	if (isdir) {
	    iter(&curdir, name);
	}
    }
    
    closedir(dir);
    
    fchdir(fd);
    close(fd);
}

int main(void)
{
    iter(NULL, "/");
}
