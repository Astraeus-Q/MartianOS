#include "PSV.h"
#include "../lib.h"

int32_t set_FM(int32_t idx);
int32_t clear_FM(int32_t idx);
int32_t set_IM(int32_t idx);
int32_t clear_IM(int32_t idx);
int32_t set_DM(int32_t idx);
int32_t clear_DM(int32_t idx);
int32_t get_free_bit_FM();
int32_t get_free_bit_IM();
int32_t get_free_bit_DM();

int32_t get_bitmap(int32_t idx, SType type){
    // Get the value of the idx^th bit of bitmap.
    int32_t val;
    int32_t i;
    switch(type){
        case FOLDER:
            val = ((&super_block)->fnode_map >> (idx % 32)) % 2;
            break;
        
        case FILE:
            i = (int32_t)(idx / 32);
            val = ((&super_block)->inode_map[i] >> (idx % 32)) %2;
            break;
        
        case DB:
            i = (int32_t)(idx / 32);
            val = ((&super_block)->db_map[i] >> (idx % 32)) % 2;
            break;
        
        default:
            val = -1;
            printf("ERROR: In get_bitmap, no such bitmap type!");
            break;            
    }
    return val;
}

int32_t set_bitmap(int32_t idx, SType type){
    int32_t res;
    switch(type){
        case FOLDER:
            res = set_FM(idx);
            break;

        case FILE:
            res = set_IM(idx);
            break;
        
        case DB:
            res = set_DM(idx);
            break;
        
        default:
            res = -1;
            printf("ERRROR: In set_bitmap, no such bitmap type!");
            break;
    }
    return res;
}

int32_t clear_bitmap(int32_t idx, SType type){
    int32_t res;
    switch(type){
        case FOLDER:
            res = clear_FM(idx);
            break;

        case FILE:
            res = clear_IM(idx);
            break;
        
        case DB:
            res = clear_DM(idx);
            break;
        
        default:
            res = -1;
            printf("ERROR: In clear_bitmap, no such bitmap type!");
            break;
    }
    return res;
}

int32_t set_FM(int32_t idx){
    int32_t res;
    if (idx < 0 || idx >= FNODE_NUM_LIM){
        printf("ERROR: In set_FM, index %d of fnode bitmap out of range!", idx);
        res = -1;
    }
    else{
        (&super_block)->fnode_map |= (1 << idx);
        res = 0;
    }
    return res;
}

int32_t clear_FM(int32_t idx){
    int32_t res;
    if (idx < 0 || idx >= FNODE_NUM_LIM){
        printf("ERROR: In clear_FM, index %d of fnode bitmap out of range!\n", idx);
        res = -1;
    }
    else{
        (&super_block)->fnode_map &= ~(1 << idx);
        res = 0;
    }
    return res;
}

int32_t set_IM(int32_t idx){
    int32_t res;
    if (idx < 0 || idx >= INODE_NUM_LIM){
        printf("ERROR: In set_IM, index %d of inode bitmap out of range!", idx);
        res = -1;
    }
    else{
        int32_t i = (int32_t) idx / 32;
        (&super_block)->inode_map[i] |= 1 << idx % 32;
        res = 0;
    }
    return res;
}

int32_t clear_IM(int32_t idx){
    int32_t res;
    if (idx < 0 || idx >= INODE_NUM_LIM){
        printf("ERROR: In clear_IM, index %d of inode bitmap out of range!", idx);
        res = -1;
    }
    else{
        int32_t i = (int32_t) idx / 32;
        (&super_block)->inode_map[i] &= ~(1 << (idx % 32));
        res = 0;
    }
    return res;
}

int32_t set_DM(int32_t idx){
    int32_t res;
    if (idx < 0 || idx >= DB_NUM_LIM){
        printf("ERROR: In set_DM, index %d of db bitmap out of range!", idx);
        res = -1;
    }
    else{
        int32_t i = (int32_t) idx / 32;
        (&super_block)->db_map[i] |= 1 << (idx % 32);
        res = 0;
    }
    return res;
}

int32_t clear_DM(int32_t idx){
    int32_t res;
    if (idx < 0 || idx >= DB_NUM_LIM){
        printf("ERROR: In clear_DM, index %d of db bitmap out of range!", idx);
        res = -1;
    }
    else{
        int32_t i = (int32_t) idx / 32;
        (&super_block)->db_map[i] &= ~(1 << (idx % 32));
        res = 0;
    }
    return res;
}

int32_t get_free_bit(SType type){
    // Get the index of a free bit from bitmap
    int32_t idx;
    switch(type){
        case FOLDER:
            idx = get_free_bit_FM();
            break;

        case FILE:
            idx = get_free_bit_IM();
            break;
        
        case DB:
            idx = get_free_bit_DM();
            break;
        
        default:
            idx = -1;
            printf("ERROR: In clear_bitmap, no such bitmap type!");
            break;
    }
    if (idx != -1){
        set_bitmap(idx, type);  // In corresponding bitmap, set this bit to 1;
        inc_used(type); // Increase the number of used term.
    }
    return idx;
}

int32_t get_free_bit_FM(){
    int32_t idx;
    int32_t found = 0;
    int i;
    for (i = 0; i < FNODE_NUM_LIM; i++){
        if (0 == get_bitmap(i, FOLDER)){
            // The i^th bit is 0;
            found = 1;
            idx = i;
            break;
        }
    }
    if (found == 0){
        printf("ERROR: In get_free_bit_FM, no enough space!");
        idx = -1;
    }
    return idx;
}

int32_t get_free_bit_IM(){
    int32_t idx;
    int32_t found = 0;
    int i;
    for (i = 0; i < INODE_NUM_LIM; i++){
        if (0 == get_bitmap(i, FILE)){
            // The i^th bit is 0;
            found = 1;
            idx = i;
            break;
        }
    }
    if (found == 0){
        printf("ERROR: In get_free_bit_IM, no enough space!");
        idx = -1;
    }
    return idx;
}

int32_t get_free_bit_DM(){
    int32_t idx;
    int32_t found = 0;
    int i;
    for (i = 0; i < DB_NUM_LIM; i++){
        if (0 == get_bitmap(i, DB)){
            // The i^th bit is 0;
            found = 1;
            idx = i;
            break;
        }
    }
    if (found == 0){
        printf("ERROR: In get_free_bit_DM, no enough space!");
        idx = -1;
    }
    return idx;    
}

int32_t idx2LBA(int idx, SType type){
    int32_t LBA;
    int32_t error = 0;
    switch(type){
        case FOLDER:
            if (idx < 0 || idx > FNODE_NUM_LIM){
                error = 1;
            }
            else{
                LBA = idx * 2 + PSV_START_LBA + FNODE_OFFS;
            }
            break;
        
        case FILE:
            if (idx < 0 || idx > FNODE_NUM_LIM){
                error = 1;
            }
            else{
                LBA = idx + PSV_START_LBA + INODE_OFFS;
            }
            break;
        
        case DB:
            if (idx < 0 || idx > FNODE_NUM_LIM){
                error = 1;
            }
            else{
                LBA = idx + PSV_START_LBA + DB_OFFS;
            }
            break;
        
        default:
            printf("ERROR: In idx2LBA, no such type!");
            break;
    }
    if (error == 1){
        printf("ERROR: In idx2LBA, index %d is out of range!", idx);
        LBA = PSV_START_LBA -1;
    }
    return LBA;
}

void inc_used(SType type){
    int32_t error = 0;
    switch (type){
        case FOLDER:
            if ((&super_block)->used_fnode >= FNODE_NUM_LIM){
                error = 1;
            }
            else{
                (&super_block)->used_fnode += 1;
            }
            break;

        case FILE:
            if ((&super_block)->used_inode >= INODE_NUM_LIM){
                error = 1;
            }
            else{
                (&super_block)->used_inode += 1;
            }
            break;

        case DB:
            if ((&super_block)->used_db >= DB_NUM_LIM){
                error = 1; 
            }
            else{
                (&super_block)->used_db += 1;
            }
            break;

        default:
            printf("ERROR: In inc_used, no such type!");
            break;
    }

    if (error == 1){
        printf("ERROR: In inc_used, overflow!");
    } 
    return;
}

void dec_used(SType type){
    int32_t error = 0;
    switch (type){
        case FOLDER:
            if ((&super_block)->used_fnode <= 0){
                error = 1;
            }
            else{
                (&super_block)->used_fnode -= 1;
            }
            break;
        case FILE:
            if ((&super_block)->used_inode <= 0){
                error = 1;
            }
            else{
                (&super_block)->used_inode -= 1;
            }
        case DB:
            if ((&super_block)->used_db <= 0){
                error = 1; 
            }
            else{
                (&super_block)->used_db -= 1;
            }
        default:
            printf("ERROR: In dec_used, no such type!");
            break;
    }

    if (error == 1){
        printf("ERROR: In dec_used, negtive!");
    } 
    return;
}

int32_t get_num_db(file_node_t inode){
    // Get the total number of data blocks in an inode.
    return (int32_t) inode.len / (SECTOR_SIZE * DB_BLKSIZE);
}

//----------------------------------------------------Path----------------------------------------------------//
int32_t is_abs(char* path){
    int32_t abs;
    if (path[0] == '/'){
        abs = 1;
    }
    else{
        abs = 0;
    }
    return abs;
}

path_t split_path(char* path){
    // Split the path by '/'.
    path_t path_sp;
    memset(&path_sp, 0, sizeof(path_sp));
    if (is_abs(path)){
        path_sp.abs = 1; // This is an abosolute path.
        path++;
    }
    else{
        path_sp.abs = 0; // This is an relative path.
    }

    path_sp.num = 0;
    int32_t cur = -1; // A cursor to record the position of the ending of last split name. 
    int32_t i;
    for (i = 0; i < strlen(path); i++){
        if (path[i] == '/'){
            if (i - 1 == cur){
                printf("ERROR: In split_path, invalid path!");
                return path_sp;
            }
            if (path_sp.num < FNODE_NUM_LIM){
                // New part.
                strncpy(path_sp.names[path_sp.num], path+cur+1, i-cur-1);
                path_sp.num++;
                cur = i;
            }
            else{
                printf("ERROR: In split_path, too many parts in path!");
                return path_sp;
            }
        }
        else if (i == strlen(path) - 1){
            if (path_sp.num < FNODE_NUM_LIM){
                // New part.
                strncpy(path_sp.names[path_sp.num], path+cur+1, i-cur);
                path_sp.num++;
                cur = i;    // Does not matter.
            }
            else{
                printf("ERROR: In split_path, too many parts in path!");
                return path_sp;
            }
        }
    }
    return path_sp;
}

void test_split_path(){
    char* path = "/ok/function/is/good/";
    path_t path_sp;
    path_sp = split_path(path);
    printf("Function is absolute? %d\n", path_sp.abs);
    printf("Parts number: %d\n", path_sp.num);
    int i;
    for (i = 0; i < path_sp.num; i++){
        printf("%d: %s\n", i, path_sp.names[i]);
    }
}

/*
 * Input: Parts of a path, assume it is valid.
 * Output: State, 0 for failure, 1 for success.
 * Function: Change the current folder to the folder specified by split path.
 */
int32_t goin_sp(path_t path_sp){
    char buf_folder[SECTOR_SIZE*FNODE_BLKSIZE];
    if (path_sp.abs){
        // Firstly, go to root folder.
        // Disk_SectorRead((uint16_t *)buf_folder, FNODE_BLKSIZE, idx2LBA(ROOT_FOLDER_IDX, FOLDER));
        // memcpy(&current_folder, buf_folder, sizeof(buf_folder));
        read_fnode(&current_folder, ROOT_FOLDER_IDX);
    }
    int32_t i, j;
    int32_t found;
    for (i = 0; i < path_sp.num; i++){
        // Travserse the path.
        found = 0;
        for (j = 0; j < FTERM_NUM_LIM; j++){
            // Traverse the fterm table of the current fnode.
            folder_term_t fterm = current_folder.fterms[j];
            if (fterm.used == 1 && !strncmp(fterm.foldername, path_sp.names[i], NAME_LEN_LIM)){
                // Disk_SectorRead((uint16_t*)buf_folder, FNODE_BLKSIZE, idx2LBA(fterm.fnode_idx, FOLDER));
                // memcpy(&current_folder, buf_folder, sizeof(buf_folder));
                read_fnode(&current_folder, fterm.fnode_idx);
                found = 1;  // Found next part.
                break;
            }
        }
        if (found == 0){
            printf("ERROR: In goin, no such folder %s!\n", path_sp.names[i]);
            return FAIL;
        }
    }
    return SUCCESS;
}

/*
 * Input: Absolute or relative path
 * Output: State, 0 for failure, 1 for success.
 * Function: Change the current folder to the folder specified by path.
 */
int32_t goin(char* path){
    int32_t state;
    path_t path_sp;
    path_sp = split_path(path);
    state = goin_sp(path_sp);
    return state;
}

/*
 * Input: A 32 bytes buffr to fill in filename; Absolute or relative path.
 * Output: NULL
 * Function: Extract the filename from a path (assume it is valid).
 */
void get_filename_from_path(char* filename, char* path){
    path_t path_sp;
    path_sp = split_path(path);
    strncpy(filename, path_sp.names[path_sp.num-1], NAME_LEN_LIM);
    return;
}
//----------------------------------------------------Path----------------------------------------------------//


//----------------------------------------------------Disk I/O API----------------------------------------------------//
void flush_space(){
    // Flush the space necessary for filesystem on disk.
    char buf[SECTOR_SIZE];
    memset(buf, 0, sizeof(buf));
    Disk_SectorWrite((uint16_t *)buf, TOTAL_SECTORS, PSV_START_LBA); // Clear those sectors to 0s.
    return;
}

// Warning: there may be some bugs in disk I/O.
void read_sb(){
    // Read super block from disk.
    char buf_sb[SECTOR_SIZE*SB_BLKSIZE];
    Disk_SectorRead((uint16_t*)buf_sb, SB_BLKSIZE, PSV_START_LBA);
    memcpy(&super_block, (super_block_t*)buf_sb, sizeof(buf_sb));
    return;
}

void write_sb(){
    // Write super block into disk.
    char buf_sb[SECTOR_SIZE*SB_BLKSIZE];
    memcpy(buf_sb, (char*)(&super_block), sizeof(buf_sb));
    Disk_SectorWrite((uint16_t*)buf_sb, SB_BLKSIZE, PSV_START_LBA);
    return;
}

void read_fnode(folder_node_t* fnode_p, int32_t idx){
    // Read fnode specified by index from disk.
    if (0 == get_bitmap(idx, FOLDER)){
        printf("ERROR: In read_fnode, invalid index: %d!\n", idx);
        return;
    }
    char buf_folder[SECTOR_SIZE*FNODE_BLKSIZE];
    Disk_SectorRead((uint16_t*)buf_folder, FNODE_BLKSIZE, idx2LBA(idx, FOLDER));
    memcpy(fnode_p, (folder_node_t*)buf_folder, sizeof(buf_folder));
    return;
}

void write_fnode(folder_node_t* fnode_p, int32_t idx){
    // Write fnode into the disk according to the index of block.
    char buf_folder[SECTOR_SIZE*FNODE_BLKSIZE];
    memcpy(buf_folder, (char*)fnode_p, sizeof(buf_folder));
    Disk_SectorWrite((uint16_t*)buf_folder, FNODE_BLKSIZE, idx2LBA(idx, FOLDER));
    return;
}

void read_inode(file_node_t* inode_p, int32_t idx){
    // Read inode specified by index from disk.
    if (0 == get_bitmap(idx, FILE)){
        printf("ERROR: In read_inode, invalid index!\n");
        return;
    }
    char buf_file[SECTOR_SIZE*INODE_BLKSIZE];
    Disk_SectorRead((uint16_t*)buf_file, INODE_BLKSIZE, idx2LBA(idx, FILE));
    memcpy(inode_p, (file_node_t*)buf_file, sizeof(buf_file));
    return;
}

void write_inode(file_node_t* inode_p, int32_t idx){
    // Write fnode into the disk according to the index of block.
    char buf_file[SECTOR_SIZE*INODE_BLKSIZE];
    memcpy(buf_file, (char*)inode_p, sizeof(buf_file));
    Disk_SectorWrite((uint16_t*)buf_file, INODE_BLKSIZE, idx2LBA(idx, FILE));
    return;
}

void read_db(char* db, int32_t idx){
    // Read data block specified by index from disk.
    if (0 == get_bitmap(idx, DB)){
        printf("ERROR: In read_db, invalid index!");
        return;
    }
    char buf_db[SECTOR_SIZE*DB_BLKSIZE];
    Disk_SectorRead((uint16_t*)buf_db, DB_BLKSIZE, idx2LBA(idx, DB));
    memcpy(db, (char*)buf_db, sizeof(buf_db));
    return;
}

void write_db(char* db, int32_t idx){
    // Write data block into the disk according to the index of block.
    char buf_db[SECTOR_SIZE*DB_BLKSIZE];
    memcpy(buf_db, db, sizeof(buf_db));
    Disk_SectorWrite((uint16_t*)buf_db, DB_BLKSIZE, idx2LBA(idx, DB));
    return;
}
//----------------------------------------------------Disk I/O API----------------------------------------------------//

//----------------------------------------------------Display----------------------------------------------------//
/*
 * Inputs: int32_t decimal number
 * Return Value: void
 * Function: print the binary form of a decimal number.
 */
void print_bin(int32_t dex){
    printf("0b ");
    int i;
    for (i = 31; i >= 0; i--){
        if (i % 4 == 3 && i != 31){
            nb_putc(',');
        }
        if (dex >> i & 1){
            nb_putc('1');
        }
        else{
            nb_putc('0');
        }
    }
    return;
}

void print_sb(){
    printf("Folder bitmap: \n");
    print_bin(super_block.fnode_map);
    printf("\n");

    printf("File bitmap: \n");
    int32_t i;
    for (i = 0; i < INODE_NUM_LIM/32; i++){
        print_bin(super_block.inode_map[i]);
        printf("\n");

    }

    // printf("Data Block bitmap: \n");
    // int i;
    // for (i = 0; i < DB_NUM_LIM/32; i++){
    //     print_bin(super_block.inode_map[i]);
    //     printf("\n");
    // }

    printf("------------------\n");
    return;
}
void print_fnode(folder_node_t fnode){
    printf("Folder name: %s\n", fnode.foldername);
    printf("Number of subfolder: %d\n", fnode.num_fterm);
    printf("Number of files: %d\n", fnode.num_iterm);
    printf("Subfolder:\n");
    int32_t i;
    for (i = 0; i < FTERM_NUM_LIM; i++){
        if (fnode.fterms[i].used == 1){
            print_fterm(fnode.fterms[i]);
        }
    }

    for (i = 0; i < ITERM_NUM_LIM; i++){
        if (fnode.iterms[i].used == 1){
            print_iterm(fnode.iterms[i]);
        }
    }
    printf("------------------\n");
    return;
}

void print_fterm(folder_term_t fterm){
    printf("%s   folder index: %d\n", fterm.foldername, fterm.fnode_idx);
    return;
}

void print_iterm(file_term_t iterm){
    printf("%s   file index: %d\n", iterm.filename, iterm.inode_idx);
    return;
}
//----------------------------------------------------Display----------------------------------------------------//

