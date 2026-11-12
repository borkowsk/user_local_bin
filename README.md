# user_local_bin

A small collection of useful Linux utilities for backups, permissions, Processing, and archive indexing.

## Included tools

- `cd_backup` — copies a directory tree into a timestamped backup under `~/backups/`.
- `cd_resque` — tries to recover a disk or block device using GNU `ddrescue`.
- `chmod_r` — recursively changes permissions without disturbing file timestamps.
- `processing` — launches Processing, optionally with a file path.
- `datefixer` — recursively repairs old/broken directory timestamps using the newest item in each directory; ignores files named `label`.
- `archliberator` — finds archives nested inside other archives and extracts them into sibling `<archive>.archs/` directories. Supports ZIP, RAR, 7z, TAR, TGZ, TAR.GZ, TAR.BZ2, TAR.XZ, and ARJ archives; requires `7z` and its supported format plugins.
- `archivetools/` — C++ utilities for indexing archive contents (`archivtool`), generating hashes (`hashforge`), and finding duplicate files (`dupminer`).

## Using the shell tools

```sh
datefixer <directory>
archliberator <directory>
```

`datefixer` sets directory timestamps based on the newest direct child, traversing the tree from the deepest directories upward. If a directory has no eligible items, it receives the fallback timestamp `1970-01-02 00:00:00`.

`archliberator` repeatedly extracts nested archives found under the given directory, placing extracted archive files in folders named after their source archive with the `.archs` suffix. Install `7z` and the plugins needed for the formats you want to process (on Ubuntu, the script suggests `7zip`, `p7zip-rar`, and `arj`).

## Build archive tools

```sh
cd archivetools
./build.sh
```

This builds the tools and copies them to `~/.local/bin`. Make sure that directory is in your `PATH`.

For details and examples, see [`archivetools/README.txt`](archivetools/README.txt).

[Przeczytaj po polsku](CZYTAJ_TO.md)
