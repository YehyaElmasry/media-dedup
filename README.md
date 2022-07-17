# MediaDedup - A Lightweight Media Files Deduplicator

A lightweight and quick program to remove duplicated media files in a filesystem.

## Overview

MediaDedup recursively searches a provided path for files with media extensions
(such as .jpg, .mp3, .mp4, etc.) and calculates a SHA-512 hash (using OpenSSL)
for each media file. If two or more files have the same hash then they are
duplicates of one another. The file with the shortest path (by lexicographical
order) is kept and duplicates are either:

- moved to a "trash" location provided by the user (recommended) or

- permanently deleted

Although MediaDedup was built with the intention of using it to deduplicate
media files, it can be used for files with other extensions by modifying the
[accepted_extensions.config](accepted_extensions.config).

## Prerequisites

MediaDedup requires:

- `GCC version 8+` or another compiler with `C++17` support, namely support for
  C++17's [filesystem library](https://en.cppreference.com/w/cpp/filesystem)

- `libssl-dev`: Install using ```apt install libssl-dev``` on Debian-based
  Linux systems

## Usage

### To build MediaDedup

```shell
cd ~
git clone https://github.com/yehy4/MediaDedup
cd MediaDedup
make clean && make
```

### To move duplicates to a "trash" path

```shell
mkdir ~/mediadedup_trash
./dedup --media-path /path/to/media --trash-path ~/mediadedup_trash
```

### To permanently delete duplicates

```shell
./dedup --media-path /path/to/media
```

Be careful with this usage pattern, and use it only when necessary. It is safer
to move duplicates to a "trash" path instead of permanently deleting them. Once
it is verified that MediaDedup ran as expected, the trash directory could be
deleted.

### Optional flags

- To view the all duplicates recursively found in `/path/to/media`, use the
  `--print-duplicates` flag. For example: `./dedup --media-path /path/to/media
  --trash-path ../mediadedup_trash --print-duplicates`

- To view all the media files recursively found in `/path/to/media`, use the
  `--print-media` flag. For example: `./dedup --media-path /path/to/media
  --trash-path ../mediadedup_trash --print-media`

- By default, MediaDedup requires user confirmation before moving or
  permanently deleting duplicate files. To bypass this confirmation, use the
  `--no-confirmation` flag. For example: `./dedup --media-path /path/to/media
  --trash-path ../mediadedup_trash --print-duplicates --no-confirmation`

- For more information, run  `./dedup --help`

## Disclaimer

The author and contributors of MediaDedup are not responsible for any data
loss, damage to your system, or other harmful effects as a result of using
MediaDedup. By using MediaDedup, the user agrees to this disclaimer.
