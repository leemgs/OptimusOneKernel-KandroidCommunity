

#ifndef _LINUX_CODA_FS_I
#define _LINUX_CODA_FS_I

#include <linux/types.h>
#include <linux/list.h>
#include <linux/coda.h>


struct coda_inode_info {
        struct CodaFid	   c_fid;	
        u_short	           c_flags;     
	struct list_head   c_cilist;    
	unsigned int	   c_mapcount;  
	unsigned int	   c_cached_epoch; 
	vuid_t		   c_uid;	
        unsigned int       c_cached_perm; 
	struct inode	   vfs_inode;
};


#define CODA_MAGIC 0xC0DAC0DA
struct coda_file_info {
	int		   cfi_magic;	  
	struct file	  *cfi_container; 
	unsigned int	   cfi_mapcount;  
};

#define CODA_FTOC(file) ((struct coda_file_info *)((file)->private_data))


#define C_VATTR       0x1   
#define C_FLUSH       0x2   
#define C_DYING       0x4   
#define C_PURGE       0x8

int coda_cnode_make(struct inode **, struct CodaFid *, struct super_block *);
struct inode *coda_iget(struct super_block *sb, struct CodaFid *fid, struct coda_vattr *attr);
int coda_cnode_makectl(struct inode **inode, struct super_block *sb);
struct inode *coda_fid_to_inode(struct CodaFid *fid, struct super_block *sb);
void coda_replace_fid(struct inode *, struct CodaFid *, struct CodaFid *);

#endif
