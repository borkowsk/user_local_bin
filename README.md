# user_local_bin

A small collection of useful Linux utilities for backups, permissions, Processing, and archive indexing.

## Included tools

- `cd_backup` — copies a directory tree into a timestamped backup under `~/backups/`.
- `cd_resque` — tries to recover a disk or block device using GNU `ddrescue`.
- `chmod_r` — recursively changes permissions without disturbing file timestamps.
- `processing` — launches Processing, optionally with a file path.
- `archivetools/` — C++ utilities for indexing archive contents (`archivtool`), generating hashes (`hashforge`), and finding duplicate files (`dupminer`).

## Build archive tools

```sh
cd archivetools
./build.sh
```

This builds the tools and copies them to `~/.local/bin`. Make sure that directory is in your `PATH`.

For details and examples, see [`archivetools/README.txt`](archivetools/README.txt).

[Przeczytaj po polsku](CZYTAJ_TO.md)
