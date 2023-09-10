#include "PSV.h"

super_block_t super_block;
folder_node_t current_folder;   // Cache for current folder node that the process is in. 
file_node_t current_file;   // Cache for current file node that the process is in. 

// Operation table for directories and files in PSV.
PSV_folder_ops_t PSV_folder_ops;  
PSV_file_ops_t PSV_file_ops;

void PSV_init(){
    // PSV_setup();
    PSV_mount();
    //test_split_path();

    // print_fnode(current_folder);
    
    //PSV_delete_folder("/home/student");

    // PSV_create_folder("/home/");
    // PSV_create_folder("/home/student");
    // PSV_create_file("/Mars.txt");
    // PSV_create_folder("/home/student/Group/");
    // PSV_create_file("/home/student/Group/group.doc");
    // char text[100];
    // strncpy(text, "Be the best of myself!\n", 100);
    // int32_t inode_idx;
    // inode_idx = get_inode_idx_by_path("/home/student/Group/group.doc");
    // write_content(inode_idx, text, 100);
    // inode_idx = get_inode_idx_by_path("/Mars.txt");
    // strncpy(text, "Mars is the fourth planet from the Sun.\n", 100);
    // write_content(inode_idx, text, 100);


    // char out[30];
    // read_content(inode_idx, 0, out, 30);
    // printf("For myself: %s", out);

    // PSV_create_file("/home/student/NetID/yul9.doc");
    // PSV_create_file("/home/student/NetID/qi11.doc");
    // PSV_create_file("/home/student/NetID/yifeis.doc");
    // goin("/home");

    // print_fnode(current_folder);
    // print_sb();
    
    // print_fnode(current_folder);
    //print_sb();
    //goin("home");
    //PSV_create_folder("NetID");
    
    //goin("NetID");
    //PSV_create_folder("ZBIGNEW");
    //PSV_create_folder("yul9");
    //PSV_create_folder("yueq4");
    //print_fnode(current_folder);
    //PSV_delete_folder("/home/NetID/ZBIGNEW");
    //print_fnode(current_folder);


    return;
}

void PSV_setup(){
    // For developer to found the primitive configuration of filesystem on the hard disk.
    printf("Setting up filesystem on disk......\n");

    // Flush the necessary space on disk for the filesystem. 
    flush_space();




    // Set up the filesystem in the harddisk
    //--------Super Block Setup--------//
    super_block_t sb;
    memset(&sb, 0, sizeof(sb)); // Clear all parts.
    // Signature
    strncpy(sb.name, "Perseverance", NAME_LEN_LIM);

    // Statistics
    sb.total_fnode = FNODE_NUM_LIM;
    sb.total_inode = INODE_NUM_LIM;
    sb.total_db = DB_NUM_LIM;

    // Positions
    sb.fnode_offs = FNODE_OFFS;
    sb.inode_offs = INODE_OFFS;
    sb.db_offs = DB_OFFS;
    //--------Super Block Setup--------//
    
    //--------Root Folder Setup--------//
    // Set up folder_term in sb.
    folder_term_t root_t;
    memset(&root_t, 0, sizeof(root_t)); // Clear all parts
    root_t.used = 1;    // This term is in use.
    strncpy(root_t.foldername, "/", NAME_LEN_LIM);
    root_t.fnode_idx = ROOT_FOLDER_IDX;

    sb.root = root_t;
    sb.fnode_map = 1;   // Set the fnode bitmap accordingly.

    // Set up root fnode.
    folder_node_t root;
    memset(&root, 0, sizeof(root)); // Clear all parts.
    strncpy(root.foldername, "/", NAME_LEN_LIM); 
    root.num_fterm = 1; // Just root folder itself.
    root.num_iterm = 0; // Currently, no file under the root folder.
    strncpy(root_t.foldername, ".", NAME_LEN_LIM);  // "." for self
    root.fterms[0] = root_t; // Set the very first term to self.
    sb.used_fnode = 1;
    //--------Root Folder Setup--------//
    
    // Write the disk.
    char buf_sb[SECTOR_SIZE*SB_BLKSIZE], buf_root[SECTOR_SIZE*FNODE_BLKSIZE];
    memcpy(buf_sb, &sb, sizeof(buf_sb));
    memcpy(buf_root, &root, sizeof(buf_root));
    Disk_SectorWrite((uint16_t*)buf_sb, SB_BLKSIZE, PSV_START_LBA);        // Write the super block into disk.
    Disk_SectorWrite((uint16_t*)buf_root, FNODE_BLKSIZE, idx2LBA(ROOT_FOLDER_IDX, FOLDER));    // Write the root fnode into disk.
    
    return;
}

void PSV_mount(){
    // Examination
    // char buf_sb[SECTOR_SIZE*SB_BLKSIZE], buf_root[SECTOR_SIZE*FNODE_BLKSIZE];
    // Disk_SectorRead((uint16_t *)buf_sb, SB_BLKSIZE, PSV_START_LBA);
    // Disk_SectorRead((uint16_t *)buf_root, FNODE_BLKSIZE, idx2LBA(ROOT_FOLDER_IDX, FOLDER));
    // super_block_t* sb = (super_block_t*)buf_sb;
    // folder_node_t root;
    // memcpy(&root, buf_root, sizeof(buf_root));
    // memcpy(&current_folder, buf_root, sizeof(buf_root));
    // memcpy(&super_block, buf_sb, sizeof(buf_sb));
    read_sb();
    read_fnode(&current_folder, ROOT_FOLDER_IDX);

    printf("Welcome to %s!\n\n", super_block.name);

    // Define I/O operations for directories and files.
    PSV_folder_ops.read = (void*)PSV_folder_read;
    PSV_folder_ops.write = (void*)PSV_folder_write;
    PSV_folder_ops.open = (void*)PSV_folder_open;
    PSV_folder_ops.close = (void*)PSV_folder_close;

    PSV_file_ops.read = (void*) PSV_file_read;
    PSV_file_ops.write = (void*) PSV_file_write;
    PSV_file_ops.open = (void*) PSV_file_open;
    PSV_file_ops.close = (void*) PSV_file_close;

    // printf("Total fnodes: %d\n", sb->total_fnode);
    // printf("Used fnodes: %d\n", sb->used_fnode);
    // printf("Total inodes: %d\n", sb->total_inode);
    // printf("Used inodes: %d\n", sb->used_inode);
    // printf("Total data blocks: %d\n", sb->total_db);
    // printf("Used data blocks: %d\n", sb->used_db);
    // printf("folder_map: ");
    // print_bin(super_block.fnode_map);
    // printf("\n");
    // printf("------------------\n");

    // printf("Root folder name: %s\n", root.foldername);
    // printf("Number of subfolder: %d\n", root.num_fterm);
    // printf("Subfolder: %s\n\n", root.fterms[0].foldername);

    // printf("Size of Super Block: %dB\n", sizeof(super_block_t), sizeof(folder_node_t), sizeof(file_node_t));
    // printf("Size of fnode: %dB\n", sizeof(folder_node_t));
    // printf("Size of inode: %dB\n", sizeof(file_node_t));
}

/*
 * Input: Absolute/relative path for new folder. 
 * Output: State, 0 for failure, 1 for success.
 * Function: Create a new folder by path.
 */
int32_t PSV_create_folder(char* path){
    // Both absolute and relative path are acceptable.
    // This function can only create one new folder. It cannot create folders recursively.
    int32_t state; 
    int32_t origin_idx; // Used to store the idx of original current folder.
    origin_idx = current_folder.fterms[SELF_IDX].fnode_idx;
    path_t path_sp;
    path_sp = split_path(path);
    if (path_sp.num <= 0){
        printf("ERROR: In PSV_create_folder, invalid path!\n");
        return FAIL;
    }
    
    char new_foldername[NAME_LEN_LIM];
    strncpy(new_foldername, path_sp.names[path_sp.num-1], NAME_LEN_LIM);
    path_sp.num--;
    state = goin_sp(path_sp); // Change the current folder to the parent folder of new folder to create.
    if (state == FAIL){
        return FAIL;
    }

    state = create_folder(&current_folder, new_foldername); // Create the new folder under current folder.

    // Go back to original current folder.
    read_fnode(&current_folder, origin_idx);
    return state;
}



/*
 * Input: Pointer of fnode; name of new folder. 
 * Output: State, 0 for failure, 1 for success.
 * Function: Create a new folder under the folder with fnode_p.
 */
int32_t create_folder(folder_node_t* fnode_p, char* name){
    if (fnode_p->num_fterm >= FTERM_NUM_LIM){
        printf("ERROR: In create_folder, fterm table is full!\n");
        return FAIL;
    }

    // Create new fterm correspondingly.
    folder_term_t  new_fterm;
    int32_t new_idx;
    new_idx = get_free_bit(FOLDER);
    if (new_idx == -1){
        // ERROR: No space for new folder!
        return FAIL;
    }
    new_fterm = gen_fterm(name, new_idx); // Generate a new fterm.

    // Update parent folder's fterm table.
    int32_t state;
    state = insert_fterm(fnode_p, new_fterm); // Insert the new fterm into parent (current) folder's fterms table.
    if (state == FAIL){
        clear_bitmap(new_idx, FOLDER);    // Clear the bit back to 0;
        return FAIL;
    }
    // Create a new fnode correspondingly.
    folder_node_t new_fnode;
    new_fnode = gen_fnode(name, new_idx, fnode_p->fterms[SELF_IDX]);

    // Write the super block and new fnode back to disk.
    write_sb();
    write_fnode(&new_fnode, new_idx);

    return SUCCESS;
}

/*
 * Input: foldername; fnode block index.
 * Output: fterm.
 * Function: Generate an instance of fterm according to input. 
 */
folder_term_t gen_fterm(char* name, int32_t idx){
    folder_term_t new_fterm;
    memset(&new_fterm, 0, sizeof(new_fterm));
    new_fterm.used = 1;
    strncpy(new_fterm.foldername, name, NAME_LEN_LIM);
    new_fterm.fnode_idx = idx;
    return new_fterm;
}

/*
 * Input: Pointer of fnode; fterm to insert.
 * Output: State, 0 for failure, 1 for success.
 * Function: Insert the fterm into fterm table of fnode. 
 */
int32_t insert_fterm(folder_node_t* fnode_p, folder_term_t fterm){
    int32_t pos = -1;
    int32_t i;
    
    for (i = SUB_IDX; i < FTERM_NUM_LIM; i++){
        // Note that first two term is for self and parent.
        if (0 == strncmp(fnode_p->fterms[i].foldername, fterm.foldername, NAME_LEN_LIM)){
            printf("ERROR: In insert_fterm, folder already exists!\n");
            return FAIL;
        }
    }
    
    for (i = SUB_IDX; i < FTERM_NUM_LIM; i++){
        // Note that first two term is for self and parent.
        if (fnode_p->fterms[i].used == 0){
            pos = i;
            break;
        }
    }


    if (pos == -1){
        printf("ERROR: In insert_fterm, no space for new folder!\n");
        return FAIL;
    }

    fnode_p->fterms[pos] = fterm; // Add new fterm into fterm table.   
    fnode_p->num_fterm++;
    write_fnode(fnode_p, fnode_p->fterms[SELF_IDX].fnode_idx);

    return SUCCESS;
}

/*
 * Input: foldername; block index of self; parent fterm.
 * Output: fnode.
 * Function: Generate an instance of fnode according to input. 
 */
folder_node_t gen_fnode(char* name, int32_t idx, folder_term_t parent){
    // Generate a new fnode according to input.
    folder_node_t new_fnode;
    memset(&new_fnode, 0, sizeof(new_fnode));
    strncpy(new_fnode.foldername, name, NAME_LEN_LIM);
    new_fnode.num_fterm = 2; // Self and parent.
    new_fnode.num_iterm = 0;

    // Insert self and parent fterms.
    folder_term_t self_t, parent_t;
    self_t = gen_fterm(".", idx);
    parent_t = parent;
    strncpy(parent_t.foldername, "..", NAME_LEN_LIM); // Change the name to "..";
    new_fnode.fterms[SELF_IDX] = self_t;
    new_fnode.fterms[PARENT_IDX] = parent_t;

    return new_fnode;
}

/*
 * Input: Absolute/relative path for the folder to delete. 
 * Output: State, 0 for failure, 1 for success.
 * Function: Delete the folder specified by path.
 */
int32_t PSV_delete_folder(char* path){
    // Both absolute and relative path are acceptable.
    // But the name of folder to delete could be neither "." nor "..".
    // It CAN delete the folder and all subfolders and files under it recursively.
    int32_t state;
    int32_t origin_idx;
    origin_idx = current_folder.fterms[SELF_IDX].fnode_idx;
    path_t path_sp;
    path_sp = split_path(path);
    if (path_sp.num <= 0){
        printf("ERROR: In PSV_delete_folder, invalid path!\n");
        return FAIL;
    }

    char del_foldername[NAME_LEN_LIM];
    strncpy(del_foldername, path_sp.names[path_sp.num-1], NAME_LEN_LIM);
    path_sp.num--;
    state = goin_sp(path_sp); // Change the current folder to the parent folder of new folder to create.
    if (state == FAIL){
        return FAIL;
    }

    state = delete_folder(&current_folder, del_foldername); // Delete the folder under current folder.
    if (state == FAIL){
        return FAIL;
    }

    // Go back to original current folder.
    read_fnode(&current_folder, origin_idx);

    return state;
}
 
/*
 * Input: Pointer of fnode; the name of the folder to delete. 
 * Output: State, 0 for failure, 1 for success.
 * Function: Delete the folder under the folder with fnode_p.
 */
int32_t delete_folder(folder_node_t* fnode_p, char* name){
    if (!strncmp(name, ".", NAME_LEN_LIM) || !strncmp(name, "..", NAME_LEN_LIM)){
        printf("ERROR: In delete_folder, invalid filename!\n");
        return FAIL;
    }

    // Delete fterm correspondingly.
    int32_t idx;
    idx = delete_fterm(fnode_p, name);  // Get the idx of corresponding fterm of fterm table.
    if (idx == -1){
        return FAIL;
    }

    folder_node_t del_folder;
    read_fnode(&del_folder, fnode_p->fterms[idx].fnode_idx);  // Get the fnode of the folder to delete.
    // Delete all files under the folder to delete.
    int32_t i, state;
    file_term_t* iterm_p;
    for (i = 0; i < ITERM_NUM_LIM; i++){
        iterm_p = &(del_folder.iterms[i]);
        if (iterm_p->used == 1){
            state = delete_file(&del_folder, iterm_p->filename);
            if (state == FAIL){
                return FAIL;
            }
        }
    }
    // Delete all subfolders under the folder to delete.
    folder_term_t* fterm_p;
    for (i = SUB_IDX; i < FTERM_NUM_LIM; i++){
        fterm_p = &(del_folder.fterms[i]);
        if (fterm_p->used == 1){
            state = delete_folder(&del_folder, fterm_p->foldername);
            if (state == FAIL){
                return FAIL;
            }
        }
    }

    // Delete fnode correspondingly. Just clear the bit in fnode bitmap.
    clear_bitmap(fnode_p->fterms[idx].fnode_idx, FOLDER);

    // Write the super block back to disk.
    write_sb();

    return SUCCESS;
}

/*
 * Input: pointer of fnode; name of the folder to delete.
 * Output: Index of the fterm to delete, but -1 for failure.
 * Function: Delete the fterm in fterm table of fnode.
 */
int32_t delete_fterm(folder_node_t* fnode_p, char* name){
    int32_t i;
    int32_t idx = -1;
    folder_term_t* fterm_p;
    for (i = SUB_IDX; i < FTERM_NUM_LIM; i++){
        fterm_p = &(fnode_p->fterms[i]);
        if ((fterm_p->used == 1) && (0 == strncmp(fterm_p->foldername, name, NAME_LEN_LIM))){
            // Found the fterm.
            idx = i;
            fterm_p->used = 0;
        }
    }
    if (idx == -1){
        printf("ERROR: In delete_fterm, folder not found!\n");
        return -1;
    }
    fnode_p->num_fterm--;
    write_fnode(fnode_p, fnode_p->fterms[SELF_IDX].fnode_idx);
    return idx;
}

/*
 * Input: Absolute/relative path for new file. 
 * Output: State, 0 for failure, 1 for success.
 * Function: Create a new file by path.
 */
int32_t PSV_create_file(char* path){
    // Both absolute and relative path are acceptable.
    // This function can create one new file.
    int32_t state;
    int32_t origin_idx; // Used to store the idx of original current folder.
    origin_idx = current_folder.fterms[SELF_IDX].fnode_idx;
    path_t path_sp;
    path_sp = split_path(path);
    if (path_sp.num < 0){
        printf("ERROR: In PSV_create_file, invalid path!\n");
        return FAIL;        
    }

    char new_filename[NAME_LEN_LIM];
    strncpy(new_filename, path_sp.names[path_sp.num-1], NAME_LEN_LIM);
    path_sp.num--;
    state = goin_sp(path_sp);   // Change the current folder to the parent folder of new file to create.
    if (state == FAIL){
        return FAIL;
    }

    state = create_file(&current_folder, new_filename);  // Create the new file under current folder.


    // Go back to original current folder.
    read_fnode(&current_folder, origin_idx);
    return state;
}

/*
 * Input: Name of new file. 
 * Output: State, 0 for failure, 1 for success.
 * Function: Create a new file under the folder with fnode_p.
 */
int32_t create_file(folder_node_t* fnode_p, char* name){
    if (fnode_p->num_iterm >= ITERM_NUM_LIM){
        printf("ERROR: In create_file, iterm table is full!\n");
        return FAIL;
    }

    // Create ne iterm correspondingly.
    file_term_t new_iterm;
    int32_t new_idx;
    new_idx = get_free_bit(FILE);
    if (new_idx == -1){
        // ERROR: No space for new folder!
        return FAIL;
    }
    new_iterm = gen_iterm(name, new_idx);   // Generate a new iterm.

    // Update folder's iterm table.
    int32_t state;
    state = insert_iterm(fnode_p, new_iterm);
    if (state == FAIL){
        clear_bitmap(new_idx, FILE);
        return FAIL;
    }

    // Create a new inode correspondingly.
    file_node_t new_inode;
    new_inode = gen_inode();

    // Write the super block and new inode back to disk.
    write_sb();
    write_inode(&new_inode, new_idx);

    return SUCCESS;
}

/*
 * Input: filename; inode block index.
 * Output: iterm.
 * Function: Generate an instance of iterm according to input. 
 */
file_term_t gen_iterm(char* name, int32_t idx){
    file_term_t new_iterm;
    memset(&new_iterm, 0, sizeof(new_iterm));
    new_iterm.used = 1;
    strncpy(new_iterm.filename, name, NAME_LEN_LIM);
    new_iterm.inode_idx = idx;
    return new_iterm;
}

/*
 * Input: Pointer of fnode; iterm to insert.
 * Output: State, 0 for failure, 1 for success.
 * Function: Insert the iterm into iterm table of fnode. 
 */
int32_t insert_iterm(folder_node_t* fnode_p, file_term_t iterm){
    int32_t pos = -1;
    
    int32_t i;
    for (i = 0; i < ITERM_NUM_LIM; i++){
        if (0 == strncmp(fnode_p->iterms[i].filename, iterm.filename, NAME_LEN_LIM)){
            printf("ERROR: In insert_iterm, file already exists!\n");
            return FAIL;
        }
    }

    for (i = 0; i < ITERM_NUM_LIM; i++){
        if (fnode_p->iterms[i].used == 0){
            pos = i;
            break;
        }
    }

    if (pos == -1){
        printf("ERROR: In insert_fterm, no space for new folder!\n");
        return FAIL;
    }

    fnode_p->iterms[pos] = iterm;
    fnode_p->num_iterm++;
    write_fnode(fnode_p, fnode_p->fterms[SELF_IDX].fnode_idx);

    return SUCCESS;
}

/*
 * Input: NULL.
 * Output: inode.
 * Function: Generate an instance of inode. 
 */
file_node_t gen_inode(){
    file_node_t new_inode;
    memset(&new_inode, 0, sizeof(new_inode));
    new_inode.len = 0;
    return new_inode;
}

/*
 * Input: Absolute/relative path for the file to delete. 
 * Output: State, 0 for failure, 1 for success.
 * Function: Delete the file specified by path.
 */
int32_t PSV_delete_file(char* path){
    // Both absolute and relative path are acceptable.
    // This function can delete one file.
    int32_t state;
    int32_t origin_idx;
    origin_idx = current_folder.fterms[SELF_IDX].fnode_idx;
    path_t path_sp;
    path_sp = split_path(path);
    if (path_sp.num <= 0){
        printf("ERROR: In PSV_delete_file, invalid path!\n");
        return FAIL;
    }

    char del_filename[NAME_LEN_LIM];
    strncpy(del_filename, path_sp.names[path_sp.num-1], NAME_LEN_LIM);
    path_sp.num--;
    state = goin_sp(path_sp); // Change the current folder to the parent folder of new folder to create.
    if (state == FAIL){
        return FAIL;
    }

    state = delete_file(&current_folder, del_filename);
    if (state == FAIL){
        return FAIL;
    }
    return SUCCESS;
}

/*
 * Input: The name of the file to delete. 
 * Output: State, 0 for failure, 1 for success.
 * Function: Delete the file under the folder with fnode_p.
 */
int32_t delete_file(folder_node_t* fnode_p, char* name){
    // Delete iterm correspondingly.
    int32_t idx;
    idx = delete_iterm(fnode_p, name);  // Get the idx of corresponding iterm of iterm table.
    if (idx == -1){
        return FAIL;
    }

    file_node_t del_inode;
    read_inode(&del_inode, fnode_p->iterms[idx].inode_idx);
    // Delete all the data blocks accordingly.
    int32_t i, state;
    for (i = 0; i < get_num_db(del_inode); i++){
        state = delete_db(del_inode.data_blocks[i]);
        if (state == FAIL){
            return FAIL;
        }
    }

    // Delete inode correspondingly. Just clear the bit in fnode bitmap.
    clear_bitmap(fnode_p->iterms[idx].inode_idx, FILE);

    // Write the super block back to disk.
    write_sb();
    return SUCCESS;
}

/*
 * Input: pointer of fnode; name of the folder to delete.
 * Output: Index of the fterm to delete, but -1 for failure.
 * Function: Delete the fterm in fterm table of fnode.
 */
int32_t delete_iterm(folder_node_t* fnode_p, char* name){
    int32_t i;
    int32_t idx = -1;
    file_term_t* iterm_p;
    for (i = 0; i < ITERM_NUM_LIM; i++){
        iterm_p = &(fnode_p->iterms[i]);
        if ((iterm_p->used == 1) && (0 == strncmp(iterm_p->filename, name, NAME_LEN_LIM))){
            // Found the iterm.
            idx = i;
            iterm_p->used = 0;
        }
    }
    if (idx == -1){
        printf("ERROR: In delete_iterm, file not found!\n");
        return -1;
    }    
    fnode_p->num_iterm--;
    write_fnode(fnode_p, fnode_p->fterms[SELF_IDX].fnode_idx);
    return idx;
}

int32_t delete_db(int32_t idx){
    // Clear the data block on disk to 0s.
    char* db[SECTOR_SIZE*DB_BLKSIZE];
    memset(db, 0, sizeof(db));
    write_db(db, idx); 
    clear_bitmap(idx, DB);
    write_sb();
    return SUCCESS;
}

/*
 * Input: Pointer of inode; start offset of reading; buffer storing the content to read; length of the content to read.
 * Output: The number of bytes read. But -1 for failure.
 * Function: Read the content of file to a buffer.
 */
int32_t read_content(int32_t inode_idx, int32_t offset, char* buf, int32_t length){
    if (offset < 0 || length <= 0){
        printf("ERROR: In read_content, invalid offset or length!\n");
        return -1;
    }

    if (inode_idx >= INODE_NUM_LIM || inode_idx < 0){
        printf("ERROR: In read content, invalid inode_idx!\n");
        return -1;
    }

    int32_t db_size = SECTOR_SIZE*DB_BLKSIZE;
    int32_t start_idx, start_offs;
    int32_t end_pos, end_idx, end_offs;
    file_node_t inode;
    read_inode(&inode, inode_idx);

    if (inode.len <= offset){
        return 0;
    }

    start_idx = offset / db_size;
    start_offs = offset - (offset / db_size) * db_size;

    end_pos = (offset + length >= inode.len ? inode.len : offset + length);
    end_idx = end_pos / db_size;
    end_offs = end_pos - (end_pos / db_size) * db_size;

    // Start.
    char buf_db[SECTOR_SIZE*DB_BLKSIZE];
    if (end_idx != start_idx){
        read_db(buf_db, inode.data_blocks[start_idx]);
        memcpy(buf, buf_db+start_offs, db_size-start_offs);
        buf += db_size - start_offs;
    }
    else{
        read_db(buf_db, inode.data_blocks[start_idx]);
        memcpy(buf, buf_db+start_offs, end_offs-start_offs);
    }

    int32_t i;
    for (i = start_idx+1; i < end_idx; i++){
        // Read the whole data block;
        read_db(buf, inode.data_blocks[i]);
        buf += db_size;
    }

    // End.
    if (end_idx != start_idx){
        read_db(buf_db, inode.data_blocks[end_idx]);
        memcpy(buf, buf_db, end_offs);
    }

    return end_offs - offset;
}

/*
 * Input: Pointer of inode; buffer storing the content to write; length of the content to read.
 * Output: The number of bytes write. But -1 for failure.
 * Function: Read the content of file to a buffer.
 */
int32_t write_content(int32_t inode_idx, char* buf, int32_t length){
    if (length < 0 || length > MAX_LEN){
        printf("ERROR: In write_content, invalid offset or length: %d!\n", length);
    }

    if (inode_idx >= INODE_NUM_LIM || inode_idx < 0){
        printf("ERROR: In write_content, invalid inode_idx!\n");
        return -1;
    }

    file_node_t inode;
    read_inode(&inode, inode_idx);

    // Delete original file.
    int32_t i, state;
    for (i = 0; i < get_num_db(inode); i++){
        state = delete_db(inode.data_blocks[i]);
        if (state == FAIL){
            return -1;
        }
    }

    inode.len = length;
    if (length == 0){
        // Nothing to write.
        write_inode(&inode, inode_idx);
        return 0;
    }

    int32_t db_size = SECTOR_SIZE*DB_BLKSIZE;
    int32_t end_idx = (length - 1) / db_size;
    int32_t end_offs = length - end_idx * db_size;
    int32_t new_idx;    // Index of new data block of content.
    char buf_db[SECTOR_SIZE*DB_BLKSIZE];
    for (i = 0; i <= end_idx; i++){
        // Update the data_block table of inode.
        new_idx = get_free_bit(DB);
        if (new_idx == -1){
            return -1;
        }
        inode.data_blocks[i] = new_idx;

        // Write content to disk.
        if (i == end_idx){
            memset(buf_db, 0, sizeof(buf_db));
            memcpy(buf_db, buf, end_offs);
            write_db(buf_db, new_idx);
        }
        else{
            memcpy(buf_db, buf, db_size);
            write_db(buf_db, new_idx);
        }
    }
    write_sb();
    write_inode(&inode, inode_idx); // Write the inode back to disk.
    return length;
}

/*
 * Input: Absolute/relative path for new file. 
 * Output: Index of fnode or inode. But -1 for failure.
 * Function: Get the index of inode by path.
 */
int32_t get_inode_idx_by_path (char* path){
    // Both absolute and relative path are acceptable.
    int32_t state;
    int32_t inode_idx;
    int32_t origin_idx; // Used to store the idx of original current folder.
    origin_idx = current_folder.fterms[SELF_IDX].fnode_idx;
    path_t path_sp;
    path_sp = split_path(path);
    if (path_sp.num < 0){
        printf("ERROR: In get_inode_idx_by_path, invalid path!\n");
        return -1;
    }

    char filename[NAME_LEN_LIM];
    strncpy(filename, path_sp.names[path_sp.num-1], NAME_LEN_LIM);
    path_sp.num--;
    state = goin_sp(path_sp);
    if (state == FAIL){
        return -1;
    }

    
    int32_t found = 0;
    file_term_t iterm;
    int32_t i;
    for (i = 0; i < ITERM_NUM_LIM; i++){
        iterm = current_folder.iterms[i];
        if (iterm.used == 1 && 0 == strncmp(current_folder.iterms[i].filename, filename, NAME_LEN_LIM)){
            found = 1;
            inode_idx = iterm.inode_idx;
        }
    }
        

    // Go back to original current folder.
    read_fnode(&current_folder, origin_idx);
    if (found == 0){
        // File not found!
        return -1;
    }
    return inode_idx;
}

// I/O operations:
/* 
 * Inputs: Absolute or relative path containing filename
 * Outputs: File descriptor entry, whose flags == IN_USE for success; flags == OERROR for fail.
 * Function: Open a folder in PSV with path and generate a file descriptor entry accordingly.
 */
file_entry PSV_folder_open(const char* path){
    dentry_t dentry;
    file_entry fde;
    if (read_dentry_by_name(path, &dentry) == -1){
        fde.flags = OERROR;
        printf("Fail to open the folder!");
        return fde;
    }   
    fde.op_ptr = (uint32_t)&dir_ops;
    fde.inode = dentry.nr_inode;
    fde.file_pos = 0;
    fde.flags = IN_USE;
    return fde;
}

/* 
 * Inputs: Absolute or relative path containing filename
 * Outputs: File descriptor entry, whose flags == IN_USE for success; flags == OERROR for fail.
 * Function: Open a file in PSV with path and generate a file descriptor entry accordingly.
 */
file_entry PSV_file_open(const char* path){
    dentry_t dentry;
    file_entry fde;
    if (read_dentry_by_name(path, &dentry) == -1){
        fde.flags = OERROR;
        return fde;
    }
    fde.op_ptr = (uint32_t)&regular_file_ops; // Function table.
    fde.inode = dentry.nr_inode; // The inode number for this file.
    fde.file_pos = 0; // Keep track of where the user is currently reading from in the file.
    fde.flags = IN_USE;
    return fde;    
}

/* 
 * Input: Pointer of file descriptor entry; buffer to fill in the content to read; Length in bytes, content to read.
 * Output: The number of bytes read. Succeed with either 32 (the limitation of name length) or 0. ; But -1 for fail
 * Function: Read a folder with according file descriptor entry. Read a foldername/filename a time.
 */
int32_t PSV_folder_read(file_entry* fp, char* buf, uint32_t length){
    folder_node_t fnode;
    int32_t fnode_idx = fp->inode;
    read_fnode(&fnode, fnode_idx);  // Get the fnode of the folder to read.
    int32_t idx = fp->file_pos; // Index of fterm/iterm to read in fterm+iterm table;

    if (idx < 0){
        printf("ERROR: In PSV_folder_read, file position cannot be negative!\n");
        return -1;
    }
    if (idx >= FTERM_NUM_LIM+ITERM_NUM_LIM){
        return 0;
    }
    
    folder_term_t fterm;
    memset(buf, 0, NAME_LEN_LIM+1); // The size of buffer is always 33.
    // To read a name from fterm table. If no more foldername to read, then read from iterm table.
    if (idx < FTERM_NUM_LIM){
        // fterm_idx = idx;
        for (; idx < FTERM_NUM_LIM; idx++){
            fp->file_pos++;
            fterm = fnode.fterms[idx];
            if (fterm.used == 1){
                // Get a new exist folder.
                strncpy(buf, fterm.foldername, NAME_LEN_LIM);
                return NAME_LEN_LIM;
            }
        }
    }

    // To read a name from iterm table.
    file_term_t iterm;
    if (idx >= FTERM_NUM_LIM){
        int32_t iterm_idx = idx - FTERM_NUM_LIM;
        for (; iterm_idx < ITERM_NUM_LIM; iterm_idx++){
            fp->file_pos++;
            iterm = fnode.iterms[iterm_idx];
            if (iterm.used == 1){
                // Get a new exist file.
                strncpy(buf, iterm.filename, NAME_LEN_LIM);
                return NAME_LEN_LIM;
            }
        }
    }

    // No foldername or filename left to read.
    return 0;
}

/* 
 * Input: Pointer of file descriptor entry; buffer to fill in the content to read; Length in bytes, content to read.
 * Output: Positive value for the number of bytes read; -1 for fail.
 * Function: Read a file with according file descriptor entry.
 */
int32_t PSV_file_read(file_entry* fp, char* buf, uint32_t length){
    int32_t nbytes;
    nbytes = read_content(fp->inode, fp->file_pos, buf, length);
    if (nbytes == -1){
        return -1;  // Read fails.
    }
    fp->file_pos += nbytes;
    return nbytes;
}



int32_t PSV_folder_write(file_entry* fp, int8_t* buf, uint32_t length) {
    return -1;
}

int32_t PSV_file_write(file_entry* fp, int8_t* buf, uint32_t length) {
    int32_t nbytes;
    int i = 0;
    for (; i < length; i++){
        nb_putc(buf[i]);
    }
    nbytes = write_content(fp->inode, buf, length);
    if (nbytes == -1){
        return -1;  // Write fails.
    }
    fp->file_pos += nbytes;
    return nbytes;
}

// "close" operations should delete the file descriptor entry in PCB accordingly.
// But in our OS, we just clear/free the file descriptor table directly in "halt" system call.
// So we do not need to implement "close" operations.
int32_t PSV_folder_close(file_entry* fp) {
    return 0;
}

int32_t PSV_file_close(file_entry* fp) {
    return 0;
}


