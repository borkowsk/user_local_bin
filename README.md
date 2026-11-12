# user_local_bin

A small collection of useful Linux utilities for backups, permissions, Processing, and archive indexing.

## Included tools

- `cd_backup` — copies a directory tree into a timestamped backup folder under `~/backups/`.
- `cd_resque` — tries to recover a disk or block device using GNU `ddrescue`.
- `chmod_r` — recursively changes permissions without disturbing file timestamps.
- `datefixer` — recursively repairs old or broken directory timestamps using the newest item in each directory; ignores files named `label`.
- `archliberator` — finds archives nested inside other archives and extracts them into sibling `<archive>.archs/` directories. Supports ZIP, RAR, 7z, TAR, TGZ, TAR.GZ, TAR.BZ2, TAR.XZ, and ARJ archives.
- `archivetools/` — C++ utilities for indexing archive contents (`archivtool`), generating checksums (`hashforge`), and finding duplicate files (`dupminer`).
- `processing` — launches Processing, optionally with a file path.

## Using the shell tools

```sh
cd_backup [directory]
cd_resque <file_name_root> [block_device]
chmod_r [directory] [chmod_options]
datefixer <directory>
archliberator <directory>
processing [file]
```

`cd_backup` copies the specified directory, or the current directory if no argument is given, into `~/backups/<timestamp>/`. The timestamp is derived from the source directory's modification date. The script preserves directory structure and file permissions.

`cd_resque` uses GNU `ddrescue` to create an ISO image and a log/map file for a block device. The required name root defines the output file names; the optional block device defaults to `/dev/sda`.

`chmod_r` recursively applies the given `chmod` mode to the target directory and its contents. Without arguments it uses the current directory and the mode `u+w` (adds write permission for the owner). It can also preserve the original file timestamps.

`datefixer` sets the timestamp of each directory to that of its newest direct child, walking the tree from the deepest directories upward. It ignores entries named `label`. If a directory has no eligible items, it receives a fallback timestamp.

`archliberator` repeatedly scans the specified directory tree for nested ZIP, RAR, 7z, TAR, TGZ, TAR.GZ, TAR.BZ2, TAR.XZ, and ARJ archives. It extracts each archive's contents into a sibling directory named `<archive>.archs/`.

`processing` launches the installed Processing binary from `/opt/processing/bin/Processing`. If a file path is provided, it converts it to an absolute path and passes it to Processing for opening.

## Build archive tools

```sh
cd archivetools
./build.sh
```

This builds the tools and copies them to `~/.local/bin`. Make sure this directory is in your `PATH`.

For details and examples, see [`archivetools/README.txt`](archivetools/README.txt).

[Przeczytaj po polsku](CZYTAJ_TO.md)
