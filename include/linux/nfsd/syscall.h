

#ifndef NFSD_SYSCALL_H
#define NFSD_SYSCALL_H

# include <linux/types.h>
#ifdef __KERNEL__
# include <linux/in.h>
#endif 
#include <linux/posix_types.h>
#include <linux/nfsd/const.h>
#include <linux/nfsd/export.h>
#include <linux/nfsd/nfsfh.h>


#define NFSCTL_VERSION		0x0201


#define NFSCTL_SVC		0	
#define NFSCTL_ADDCLIENT	1	
#define NFSCTL_DELCLIENT	2	
#define NFSCTL_EXPORT		3	
#define NFSCTL_UNEXPORT		4	


#define NFSCTL_GETFD		7	
#define	NFSCTL_GETFS		8	


struct nfsctl_svc {
	unsigned short		svc_port;
	int			svc_nthreads;
};


struct nfsctl_client {
	char			cl_ident[NFSCLNT_IDMAX+1];
	int			cl_naddr;
	struct in_addr		cl_addrlist[NFSCLNT_ADDRMAX];
	int			cl_fhkeytype;
	int			cl_fhkeylen;
	unsigned char		cl_fhkey[NFSCLNT_KEYMAX];
};


struct nfsctl_export {
	char			ex_client[NFSCLNT_IDMAX+1];
	char			ex_path[NFS_MAXPATHLEN+1];
	__kernel_old_dev_t	ex_dev;
	__kernel_ino_t		ex_ino;
	int			ex_flags;
	__kernel_uid_t		ex_anon_uid;
	__kernel_gid_t		ex_anon_gid;
};


struct nfsctl_fdparm {
	struct sockaddr		gd_addr;
	char			gd_path[NFS_MAXPATHLEN+1];
	int			gd_version;
};


struct nfsctl_fsparm {
	struct sockaddr		gd_addr;
	char			gd_path[NFS_MAXPATHLEN+1];
	int			gd_maxlen;
};


struct nfsctl_arg {
	int			ca_version;	
	union {
		struct nfsctl_svc	u_svc;
		struct nfsctl_client	u_client;
		struct nfsctl_export	u_export;
		struct nfsctl_fdparm	u_getfd;
		struct nfsctl_fsparm	u_getfs;
		
		void *u_ptr;
	} u;
#define ca_svc		u.u_svc
#define ca_client	u.u_client
#define ca_export	u.u_export
#define ca_getfd	u.u_getfd
#define	ca_getfs	u.u_getfs
};

union nfsctl_res {
	__u8			cr_getfh[NFS_FHSIZE];
	struct knfsd_fh		cr_getfs;
};

#ifdef __KERNEL__

extern int		exp_addclient(struct nfsctl_client *ncp);
extern int		exp_delclient(struct nfsctl_client *ncp);
extern int		exp_export(struct nfsctl_export *nxp);
extern int		exp_unexport(struct nfsctl_export *nxp);

#endif 

#endif 
