# File Recovery from Raw Storage

> **TIPE 2024-2025 · Cardot Clément & William Guerin-Garnier · MPI · Lycée Louis Thuillier**

## What is a TIPE?

In France, students in *classes préparatoires* (two-year intensive post-baccalaureate programs preparing for the *grandes écoles* competitive exams) must complete a **TIPE** (*Travail d'Initiative Personnelle Encadré* — supervised personal research project). It is evaluated as a standalone oral exam (~15 min presentation + questions) during the national *concours* and accounts for a significant portion of the final score.

The TIPE must be original, scientifically rigorous, and — for the MPI track (Mathematics, Physics, Computer Science) — rooted in at least one of those three disciplines. Each year a broad theme is announced; students must anchor their work to it.

This project was presented as part of the theme **"Transition, Transformation, Conversion"** (2024-2025): deleted files are not truly gone — they are raw binary data waiting to be transformed back into usable files.

---

## Project overview

**Research question:** Is it theoretically and practically possible to recover supposedly lost files from the raw data of a storage medium?

When a file is deleted on most operating systems, only its metadata (the pointer in the file system table) is erased — the actual data blocks remain on disk until overwritten. This project explores whether those blocks can be found and reconstructed without any metadata.

The work is split into two complementary parts:

- **Clément's part**  — design and implementation of a **simulated storage system** in C with an SQLite backend, acting as a virtual file system to provide a controlled environment for developing and testing recovery algorithms
- **William's part**  — testing recovery on **real storage devices** and benchmarking against professional tools

---

## Clément's part — Storage system simulator (C + SQLite)

### What it does

This program simulates a minimal file system from scratch. Files are stored as a flat hexadecimal dump (`fichier_hexadecimal.txt`) alongside an SQLite database (`database.db`) that holds the metadata table — mimicking the separation between raw data blocks and the file allocation table found on real drives.

**Simulating deletion** means simply dropping the file's row from the SQL table. The hex data remains untouched in the flat file, exactly as it would on a real disk. Recovery algorithms can then be run on that raw file without any database access.

### Data model

Each file is stored in the SQLite table `fichiers` with the following fields:

| Field | Type | Description |
|-------|------|-------------|
| `id` | INTEGER | Auto-incremented primary key |
| `nom` | TEXT | File name (without extension) |
| `extension` | TEXT | File extension (jpeg, png, gif…) |
| `adresse` | TEXT | `offset:length` in the hex dump |

The `adresse` field encodes both where the file starts in the hex dump (`offset`) and how many bytes it occupies (`length`), separated by a colon — a deliberate design choice to keep the schema minimal.

### Available operations

The program exposes an interactive terminal menu with the following operations:

| Option | Function | Description |
|--------|----------|-------------|
| 1 | `ls_database` | List all files in the database |
| 2 | `ajouter_fichier` | Add a file: convert to hex, append to dump, insert metadata, delete original |
| 3 | `ouvrir_data` | Reconstruct a file from the hex dump using its stored offset/length |
| 4 | `recherche` | Search files by extension |
| 5 | `renommer` | Rename a file entry in the database |
| 6 | `supprimer` | Delete a file: remove DB entry, compact the hex dump, update all offsets |
| 7 | — | Quit |

### Key implementation details

**Adding a file** (`ajouter_fichier`): the binary file is read byte by byte, each byte written as two hex characters appended to `fichier_hexadecimal.txt`. The current file position before writing gives the offset; the byte count gives the length. Both are stored in the DB. The original file is then deleted from disk.

**Opening a file** (`ouvrir_data`): the offset and length are read from the DB, the hex dump is seeked to that position, and bytes are read two characters at a time and written back as binary — reversing the conversion exactly.

**Deleting a file** (`supprimer`): beyond removing the DB row, the hex dump is compacted in memory (the deleted block is excised from a buffer) and rewritten, and all subsequent offsets in the DB are decremented accordingly. This keeps the dump consistent.

**SQL injection mitigation**: the interface uses `snprintf`-built queries with user input — a known limitation noted during development after testing with `DROP TABLE` inputs. Parameterised queries (`sqlite3_bind_*`) would be the correct fix.

### Build & run

```bash
gcc -o storage_sim storage_sim.c -lsqlite3
./storage_sim
```

**Dependencies:** `sqlite3` (system library)

```bash
# Ubuntu/Debian
sudo apt install libsqlite3-dev
```

---

## William's part

William's part focuses on running file carving algorithms on **real storage devices** (USB drives, hard disks with deleted partitions) and comparing results against professional recovery tools such as TestDisk, Recuva, and Photorec.

His code and results will be available in his own repository — link to be added here.

---

## Security implications

A key takeaway of this project: **standard deletion is not secure**. Dropping a metadata entry leaves data fully intact on the storage medium. This highlights the importance of secure erase methods (multi-pass overwriting, encryption before deletion) for sensitive data — a point that applies equally to the simulated system and real drives.

---

## Repository structure

```
.
├── storage_sim.c       # Full C source — storage simulator
├── docs/
│   ├── presentation.pdf
│   └── MCOT.pdf
└── README.md
```

---

## Academic context

- **Programme:** MPI, 1st year
- **Theme:** *Transition, Transformation, Conversion* — 2024-2025
- **Thematic positioning:** Computer Science (practical)
- **Keywords:** File carving · File-system simulation · Search algorithm · Sequential reading · Database
- **Group work:** Cardot Clément & William Guerin-Garnier

This project was presented as a TIPE oral examination in 2025 at the IUT de Paris — Université Paris Cité, as part of the MPI *classes préparatoires* competitive admissions (*concours*).

---

## References

1. Steven Alexander — *Understanding Deleted Files and What They Mean*, HG Experts
2. Golden Richard III — *Scalpel: A Frugal, High Performance File Carver*, DFRWS 2005
3. Gary Kessler — *File Signatures Table* — https://www.garykessler.net/library/file_sigs.html
4. Royal E. Frazier Jr. — *All About GIF89a*
5. Kevin Engel — *Comment vérifier l'intégrité d'un fichier ?*, Tech2Tech
6. Thomas Laurenson — *Performance Analysis of File Carving Tools*, INRIA 2017
7. Tanenbaum & Bos — *Modern Operating Systems*, Pearson
8. Nikita Patel et al. — *SQL Injection Attacks: Techniques and Protection Mechanisms*
