


#ifndef MINI_OPT_H
#define MINI_OPT_H

#ifdef __cplusplus
extern "C" {
#endif




#define MINIOPT_MAXKEY	128	
typedef struct miniopt {

	
	const char* name;		
	const char* flags;		
	bool longflags;		
	bool opt_end;		

	

	int consumed;		
	bool positional;
	bool good_int;		
	char opt;
	char key[MINIOPT_MAXKEY];
	char* valstr;		
	uint uval;		
	int  val;		
} miniopt_t;

void miniopt_init(miniopt_t *t, const char* name, const char* flags, bool longflags);
int miniopt(miniopt_t *t, char **argv);






#ifdef __cplusplus
	}
#endif

#endif  
