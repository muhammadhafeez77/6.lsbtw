# lsbtw

a from-scratch reimplementation of `ls` in C, built step by step in the video. the
goal isn't a production tool, it's to understand what `ls` actually does: how it
reads a directory and how a single `stat` call gives you everything in an `ls -l` row.

you should be able to take this video and implement more features to `ls`, and other coreutils.

## build / run

```sh
gcc -Wall -Wextra -o lsbtw ls.c

./lsbtw            # list the current directory
./lsbtw -a         # include dotfiles
./lsbtw -l         # long format
./lsbtw -la /etc   # combine flags, list a path
```

## What's implemented

- **`opendir` / `readdir`** — walk a directory and print its entries.
- **`-a`** — show hidden entries (the ones starting with `.`).
- **`-l`** — the long listing, where the real work is. For each entry we `lstat`
  it and decode the `struct stat`:
  - `st_mode` → file type + permission string (`-rw-r--r--`) via `mode_string`
  - `st_nlink` → hard-link count
  - `st_uid` / `st_gid` → owner and group names via `getpwuid` / `getgrgid`
  - `st_size` → size in bytes
  - `st_mtim` → modification time, formatted with `localtime` + `strftime`

## where we left off

the output is correct field-for-field, but it differs from real `ls` in two ways,
both of which are presentation, not filesystem logic:

1. **sorting** entries come out in raw `readdir` order (whatever the filesystem
   hands back). real `ls` sorts them alphabetically.
   *Hint:* qsort is your friend

2. **padding / alignment.** the link count and size columns are single spaced and
   ragged. real `ls` right-aligns them into nice columns.

If you followed the video, you've got everything you need to add both.

## contributing

feel free to contribute any hand written ls args, or specifically the sorting and padding that i displayed at the end of the video.

cheers.

tony, btw
