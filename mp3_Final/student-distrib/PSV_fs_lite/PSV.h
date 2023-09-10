/* 
 * Version: PSV.0.5
 * This is a readable & writable filesystem mounted on disk. 
 * Its structure is multilaminate which supports the management of folders and files.
 * It contains 4 chunks for 1 super block, 32 folder nodes, 64 file nodes and 2048 data blocks.
 * 
 */

#ifndef PSV_H
#define PSV_H
#include "../types.h"
//#include "../math.h"
#include "../lib.h"
#include "../harddisk.h"
#include "../fs.h"

// Limitations
#define NAME_LEN_LIM 32     // The length of a file name is at most 32B.
#define FTERM_NUM_LIM 8    // At most 10 fterms (folder term) under a folder, within which 2 are reserved for current and parent folder (. and ..). 
                            // So at most 8 subfolder within the folder.
#define ITERM_NUM_LIM 16   // At most 20 iterm (file term) in a folder, namely, at most 20 files under a folder.

#define FNODE_NUM_LIM 32    // At most 32 folders in this filesystem. 
#define INODE_NUM_LIM 64    // At most 64 file_nodes (64 files) in this filesystem. 
#define DB_NUM_LIM 2048     // At most 2048 data blocks (1MB) in this filesystem.
#define FILE_BLKSIZE_LIM 126   // The size of a file is at most 126 blocks (63 KB).      
#define MAX_LEN SECTOR_SIZE*FILE_BLKSIZE_LIM    // Maximal size of a file.   

// Positions
#define PSV_START_LBA 1000   // The LBA of super block.
#define TOTAL_SECTORS (1+FNODE_NUM_LIM*2+INODE_NUM_LIM+DB_NUM_LIM)
#define FNODE_OFFS 1
#define INODE_OFFS 65
#define DB_OFFS 129
#define ROOT_FOLDER_IDX 0
#define SELF_IDX 0          // Index of fterm of self in fterm table.
#define PARENT_IDX 1        // Index of fterm of parent in fterm table.
#define SUB_IDX 2           // First index of fterm of subfolder of in fterm table.

// Reservation
#define SB_REV 136
#define FNODE_REV 24
#define INODE_REV 4

// Size (in sector)
#define SB_BLKSIZE 1
#define FNODE_BLKSIZE 2
#define INODE_BLKSIZE 1
#define DB_BLKSIZE 1

// State
#define SUCCESS 1
#define FAIL    0

typedef enum Struct_Type {FOLDER, FILE, DB} SType; // Define 3 types of structures in PSV filesystem.
//----------------------------------------------------Structure----------------------------------------------------//
typedef struct folder_term {
    // fterm
    char used;           // 1 for used, 0 for free.
    char foldername[NAME_LEN_LIM];
    int32_t fnode_idx;  // The index of block that contains corresponding folder_node.
} folder_term_t; 

//typedef enum File_Type {SOCKET, REGULAR, ELF} FType;
typedef struct file_term {
    // iterm
    int32_t used;       // 1 for used, 0 for free.
    char filename[NAME_LEN_LIM];
    //FTYPE type;       // There are 3 possible types for a file, see above.
    int32_t inode_idx;  // The index of block that contains corresponding file_node
} file_term_t;

typedef struct folder_node {
    // fnode
    char foldername[NAME_LEN_LIM];
    int32_t num_fterm;  // Number of subfolder, including self and parent.
    int32_t num_iterm;
    folder_term_t fterms[FTERM_NUM_LIM];
    file_term_t iterms[ITERM_NUM_LIM];
    char rev[FNODE_REV];    // This space is reserved for 512B aligned.
} folder_node_t;    // Size: 1024B

typedef struct file_node {
    // inode
    int32_t len;
    int32_t data_blocks[FILE_BLKSIZE_LIM];  // The index of blocks that contain file content.
    char rev[INODE_REV];                // This space is reserved for 512B aligned.
} file_node_t;  // Size: 512B

typedef struct super_block {
    // Signature
    char    name[NAME_LEN_LIM];        // The name of the filesystem

    // Statistics
    int32_t total_fnode;    // Total number of fnodes in the filesystem.
    int32_t used_fnode;     // Number of used fnodes.
    int32_t total_inode;    // Total number of inodes in the filesystem.
    int32_t used_inode;     // Number of used inodes.
    int32_t total_db;       // Total number of data blocks in the filesystem.
    int32_t used_db;        // Number of used data blocks.

    // Positions
    int32_t fnode_offs;     // The offset (in sector) of the folder node chunk. Always 1, because sb takes up 1 block.
    int32_t inode_offs;     // The offset (in sector) of the file node chunk. Always 1+32*2 = 65, because fnode chunk takes up 32*2 blocks.
    int32_t db_offs;        // The offset (in sector) of the data block chunk. Always 1+32*2+64 = 129, because inode chunk takes up 64 blocks.

    // Bitmaps
    int32_t fnode_map;     // Bitmap recording the usage of 32 folder node blocks.
    int32_t inode_map[INODE_NUM_LIM/32];   // Bitmap recording the usage of 64 file node blocks.
    int32_t db_map[DB_NUM_LIM/32];     // Bitmap recording the usage of 2048 data blocks.

    folder_term_t root;
    // Reservation
    char rev[SB_REV];          // This space is reserved for 512B aligned.
} super_block_t;    // Size: 512B

typedef struct path{
    int32_t num; // Number of parts split form path.
    int32_t abs; // 1 for absolute path, 0 for relative path.
    char names[FNODE_NUM_LIM][NAME_LEN_LIM]; // At most FNODE_NUM_LIM parts in a path.
} path_t;

//----------------------------------------------------Structure----------------------------------------------------//

//----------------------------------------------------Cache----------------------------------------------------//
/* Global variables defined in PSV.c */
extern super_block_t super_block;
extern folder_node_t current_folder;      // Cache for current folder node that the process is in.
extern file_node_t current_file;      // Cache for current file node that the process is in. 
//----------------------------------------------------Cache----------------------------------------------------//

//----------------------------------------------------I/O Operations----------------------------------------------------//
typedef struct PSV_folder_operations {
    int32_t (*read)(file_entry* fp, void* buf, int32_t nbytes);
    int32_t (*write)(file_entry* fp, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* path);
    int32_t (*close)(file_entry* fp);
} PSV_folder_ops_t;

typedef struct PSV_file_operations {
    int32_t (*read)(file_entry* fp, void* buf, int32_t nbytes);
    int32_t (*write)(file_entry* fp, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* path);
    int32_t (*close)(file_entry* fp);
} PSV_file_ops_t;

/* Global variables defined in PSV.c */
extern PSV_folder_ops_t PSV_folder_ops;
extern PSV_file_ops_t PSV_file_ops;
//----------------------------------------------------I/O Operations----------------------------------------------------//


// filesystem.c
void PSV_setup();
void PSV_init();
void PSV_mount();
// Folder operations:
int32_t PSV_create_folder(char* path);
int32_t create_folder(folder_node_t* fnode_p, char* name);
folder_term_t gen_fterm(char* name, int32_t idx);
int32_t insert_fterm(folder_node_t* fnode_p, folder_term_t fterm);
folder_node_t gen_fnode(char* name, int32_t idx, folder_term_t parent);
int32_t PSV_delete_folder(char* path);
int32_t delete_folder(folder_node_t* fnode_p, char* name);
int32_t delete_fterm(folder_node_t* fnode_p, char* name);
// File operations:
int32_t PSV_create_file(char* path);
int32_t create_file(folder_node_t* fnode_p, char* name);
file_term_t gen_iterm(char* name, int32_t idx);
int32_t insert_iterm(folder_node_t* fnode_p, file_term_t iterm);
file_node_t gen_inode();

int32_t delete_file(folder_node_t* fnode_p, char* name);
// Content (data block) operations:
int32_t delete_db();

// I/O operations:
int32_t read_content(int32_t inode_idx, int32_t offset, char* buf, int32_t length);
int32_t write_content(int32_t inode_idx, char* buf, int32_t length);
int32_t get_inode_idx_by_path (char* path);
file_entry PSV_folder_open(const char* path);
file_entry PSV_file_open(const char* path);
int32_t PSV_folder_read(file_entry* fp, char* buf, uint32_t length);
int32_t PSV_file_read(file_entry* fp, char* buf, uint32_t length);
int32_t PSV_folder_write(file_entry* fp, int8_t* buf, uint32_t length);
int32_t PSV_file_write(file_entry* fp, int8_t* buf, uint32_t length);
int32_t PSV_folder_close(file_entry* fp);
int32_t PSV_file_close(file_entry* fp);



// helper.c
int32_t get_bitmap(int32_t idx, SType type);
int32_t set_bitmap(int32_t idx, SType type);
int32_t clear_bitmap(int32_t idx, SType type);
int32_t get_free_bit(SType type);

int32_t idx2LBA(int idx, SType type);

void inc_used(SType type);
void dec_used(SType type);
int32_t get_num_db(file_node_t inode);

int32_t is_abs(char* path);
int32_t goin_sp(path_t path_sp);
int32_t goin(char* path);
path_t split_path(char* path);
void test_split_path();
void get_filename_from_path(char* filename, char* path);

void flush_space();
void read_sb();
void write_sb();
void read_fnode(folder_node_t* fnode_p, int32_t idx);
void write_fnode(folder_node_t* fnode_p, int32_t idx);
void read_inode(file_node_t* inode_p, int32_t idx);
void write_inode(file_node_t* inode_p, int32_t idx);
void read_db(char* db, int32_t idx);
void write_db(char* db, int32_t idx);

void print_bin(int32_t dex);
void print_sb();
void print_fnode(folder_node_t fnode);
void print_iterm(file_term_t fterm);


#endif  /* PSV.h */
