#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

int count[4] = {0, 0, 0, 0};

void padding(const char *dir, const char *filename)
{
    struct stat st;

    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, filename);
    if (stat(fullpath, &st) != 0)
    {
        return;
    }
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    const char *user = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    int comp1 = count[0]; // count[0] = number of character / spaces reserved for ===>>> hardlinks
    long long temp = st.st_nlink;
    count[0] = 0;
    do
    {
        count[0]++;
        temp = temp / 10;
    } while (temp != 0);

    if (count[0] < comp1)
    {
        count[0] = comp1;
    }

    temp = count[1]; // count[1]    = number of character / spaces reserved for ===>>> users
    count[1] = strlen(user);
    if (count[1] < temp)
    {
        count[1] = temp;
    }

    temp = count[2]; // count[2]    = number of character / spaces reserved for ===>>> groups
    count[2] = strlen(group);
    if (count[2] < temp)
    {
        count[2] = temp;
    }

    int comp4 = count[3]; // count[3]   = number of character / spaces reserved for ===>>> bytes of file
    temp = st.st_size;
    count[3] = 0;

    do
    {
        count[3]++;
        temp = temp / 10;
    } while (temp != 0);

    if (count[3] < comp4)
    {
        count[3] = comp4;
    }
}

void mode_string(mode_t mode, char *str)
{
    if (S_ISDIR(mode))
        str[0] = 'd';
    else if (S_ISLNK(mode))
        str[0] = 'l';
    else if (S_ISCHR(mode))
        str[0] = 'c';
    else if (S_ISBLK(mode))
        str[0] = 'b';
    else if (S_ISFIFO(mode))
        str[0] = 'p';
    else if (S_ISSOCK(mode))
        str[0] = 's';
    else
        str[0] = '-';

    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

void print_long(const char *dir, const char *name)
{
    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);

    struct stat st;
    if (lstat(fullpath, &st) < 0)
    {
        perror(name);
        return;
    }
    char modes[11];
    mode_string(st.st_mode, modes);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    const char *user = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    char timebuf[64];

    struct tm *tm = localtime(&st.st_mtim.tv_sec);
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

    printf(
        "%s %*lu %*s %*s %*ld %s %s \n",
        modes,
        count[0], (unsigned long)st.st_nlink,
        count[1], user,
        count[2], group,
        count[3], (long)st.st_size,
        timebuf,
        name);
}

int show_all = 0;
int long_format = 0;

int main(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "al")) != -1)
    {
        switch (opt)
        {
        case 'a':
            show_all = 1;
            break;
        case 'l':
            long_format = 1;
            break;
        default:
            fprintf(stderr, "usage: %s [-al] [path]\n", argv[0]);
            return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";

    DIR *dir = opendir(path);
    DIR *innerdir = opendir(path);
    if (!dir || !innerdir)
    {
        perror("opendir");
        return 1;
    }

    struct dirent *entry, *empress;

    while ((entry = readdir(dir)) != NULL)
    {
        while ((empress = readdir(innerdir)) != NULL)
        {
            if (!show_all && empress->d_name[0] == '.')
            {
                continue;
            }
            padding(path, empress->d_name);
        }

        if (!show_all && entry->d_name[0] == '.')
        {
            continue;
        }

        if (long_format)
        {
            print_long(path, entry->d_name);
        }
        else
        {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(innerdir);
    closedir(dir);
    return 0;
}
