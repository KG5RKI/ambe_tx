#define BSIZE 100

typedef struct {
    char buffer[BSIZE];
    char *id ;
    char *callsign ;
    char *firstname ;
    char *name ;
    char *place ;
    char *state ;
    char *country ;
	char fUserType;
} user_t ;

int usr_find_by_dmrid( user_t *up, int dmrid, char* data );
int find_dmr(char *outstr, long dmr_search,
                    const char *dmr_begin, const char *dmr_end,
                    int outsize);
void usr_splitbuffer(user_t *up);
const char* next_line_ptr(const char* p);
void readline(char *out, const char *in, int outsize);
void usr_tobuffer(user_t* up, char* data);