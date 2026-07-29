# File Recovery from Raw Storage

> **TIPE 2024-2025 · Cardot Clément & William Guerin-Garnier · MPI · Lycée Louis Thuillier**

## What is a TIPE?

In France, students in classes préparatoires (two-year intensive post-baccalaureate programs preparing for the grandes écoles competitive exams) must complete a TIPE (Travail d'Initiative Personnelle Encadré — supervised personal research project). It is evaluated as a standalone oral exam (~15 min presentation + questions) during the national concours and accounts for a significant portion of the final score.

The TIPE must be original, scientifically rigorous, and — for the MPI track (Mathematics, Physics, Computer Science) — rooted in at least one of those three disciplines. Each year a broad theme is announced; students must anchor their work to it.

This project was presented as part of the theme **"Transition, Transformation, Conversion"** (2024-2025): deleted files are not truly gone — they are raw binary data waiting to be transformed back into usable files.

---

## Project overview

**Research question:** Is it theoretically and practically possible to recover supposedly lost files from the raw data of a storage medium?

When a file is deleted on most operating systems, only its metadata (the pointer in the file system table) is erased — the actual data blocks remain on disk until overwritten. This project explores whether those blocks can be found and reconstructed without any metadata.

The work is split into two complementary parts:

- **Clément's part** — design and implementation of a **simulated storage system** (SQLite-based virtual USB drive) to provide a controlled environment for testing recovery algorithms
- **William's part** — testing recovery algorithms on **real storage devices** (USB drives, hard disks) and benchmarking against professional tools (TestDisk, Recuva, Photorec)

---

## How it works

### 1 — Storage simulation (SQLite)

Real raw disk access is complex and hardware-dependent. To develop and test algorithms in a controlled setting, a virtual storage medium was built:

- Files are converted to their **hexadecimal representation** and written to a flat `.txt` file simulating raw memory
- File metadata (name, location, size, type) is stored in an **SQLite database** acting as the file allocation table
- Deleting a file means dropping its entry from the SQL table — the raw hex data remains in the flat file
- Recovery is then attempted on this raw file without access to the database

### 2 — File carving

Recovery relies on **sequential scanning** of the raw binary data for known **file signatures** (magic bytes):

| Format | Header signature | Footer signature |
|--------|-----------------|-----------------|
| JPEG   | `FF D8 FF`      | `FF D9`         |
| PNG    | `89 50 4E 47 0D 0A 1A 0A` | `49 45 4E 44 AE 42 60 82` |
| GIF    | `47 49 46 38`   | `00 3B`         |

When a header is found, the algorithm reads forward until the matching footer, extracts the block, and attempts to reconstruct the file.

### 3 — Integrity validation

Each recovered file is validated against its expected format structure to detect corruption or partial recovery.

### 4 — Directory tree simulation

A simulated directory structure was added to the SQLite schema, allowing recovery of file hierarchy information (paths, folder names) in addition to raw file content.

---

## Security implications

A key finding of this project: **standard deletion is not secure**. As long as data blocks are not overwritten, recovery is possible with basic tools. This highlights the importance of secure erase methods (multi-pass overwriting, encryption before deletion) for sensitive data.

---

## Repository structure

```
.
├── src/
│   ├── simulate_storage.py     # Build the virtual storage (hex dump + SQLite)
│   ├── file_carving.py         # Sequential scan and file reconstruction
│   ├── validate_files.py       # Integrity checks on recovered files
│   └── interface.py            # Minimal GUI to browse recovered files
├── docs/
│   ├── presentation.pdf        # Oral presentation slides
│   └── MCOT.pdf                # Research objectives document (MCOT)
├── examples/
│   └── sample_storage.txt      # Example raw hex storage file
└── README.md
```

---

## Dependencies

| Dependency | Purpose |
|------------|---------|
| Python 3   | Main language |
| `sqlite3`  | Storage simulation (standard library) |
| `tkinter`  | Minimal interface (standard library) |

No external packages required — everything runs on the Python standard library.

---

## Usage

```bash
# Step 1 — build a simulated storage from a folder of files
python src/simulate_storage.py input_folder/ storage.txt storage.db

# Step 2 — delete the database (simulates file deletion)
rm storage.db

# Step 3 — run file carving on the raw storage
python src/file_carving.py storage.txt recovered/

# Step 4 — validate recovered files
python src/validate_files.py recovered/
```

---

## Known limitations

- **Video formats (MOV)** were abandoned early due to truncated and variable signatures — the project focuses on image formats only (JPEG, PNG, GIF)
- **Fragmented files** (blocks not contiguous in memory) are not handled; carving assumes contiguous storage
- **Overwritten data** is unrecoverable by design — the simulation does not model overwriting

---

## Academic context

- **Programme:** MPI, 1st year (*5/2* preparatory year)
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
