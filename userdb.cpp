/*! \file usersdb.c
\brief There is the functionality
       which dmr id to the entry from the users.csv in flash reads
       the first line is the size im byte
*/

#include <stdlib.h>
#include <string.h>
#include "userdb.h"
#include <stdio.h>
#include <stdlib.h>

/* All user database data is accessed through getdata.
 * This makes it easier to adapt to different kinds of sources.
 */


static char * getdata(char * dest, const char *  src, int count) {
    memcpy(dest, src, count);
    return dest;
}

/* copies a line of text starting at in[] that is terminated
 * with a linefeed '\n' or '\0' to out[]. At most outsize characters
 * are written to out[] (including null terminator). Lines that
 * don't fit into out[] are truncated. out[] will always be
 * null terminated if outsize > 0.
 */
void readline(char *out, const char *in, int outsize)
{
    if( outsize <= 0 ) return;
    char buff[64];
    const int blen = sizeof (buff);
    outsize -= 1; // for null terminator
    while (outsize > 0) {
        int chunk = outsize > blen ? blen : outsize;
        getdata(buff, in, chunk);
        for (int i = 0; i < chunk; ++i) {
            char c = buff[i];
            if( c == '\0' || c == '\n' ) {
                *out++ = '\0';
                return;
            }
            *out++ = c;
        }
        in += chunk;
        outsize -= chunk;
    }
    *out = '\0';
}

/* searches for a newline character starting at *p and returns
 * the pointer to the character following that newline
 */
const char* next_line_ptr(const char* p) {
    char buffer[64];
    const int blen = sizeof(buffer);
    for (;;) {
        getdata(buffer, p, blen);
        int linefeedidx = 0;
        while (linefeedidx < blen && buffer[linefeedidx] != '\n') {
            ++linefeedidx;
        }
        if (linefeedidx < blen) {
            return p + linefeedidx + 1;
        }
        p += blen;
    }
}

/* parse number as text and return its numerical value
 */
static long getfirstnumber(const char * p) {
  char buffer[64];
  return (atol(getdata(buffer, p, 60)));
}

/* tries to locate and copy the line of text that starts with the
 * number dmr_search to the output buffer outstr. The return value
 * indicates whether the DMR ID was found (1) or not (0).
 */
int find_dmr(char *outstr, long dmr_search,
                    const char *dmr_begin, const char *dmr_end,
                    int outsize)
{
    /* As long as there is at least one line of text between
       offsets dmr_begin and dmr_end... */
    while (dmr_begin != dmr_end) {
        const char* dmr_test = next_line_ptr(dmr_begin + (dmr_end - dmr_begin) / 2);
        if (dmr_test == dmr_end) { dmr_test = next_line_ptr(dmr_begin); }
        if (dmr_test == dmr_end) { dmr_test = dmr_begin; }
        long id = getfirstnumber(dmr_test);
        if (id == dmr_search || dmr_search == 0) {
            readline(outstr, dmr_test, outsize);
            return 1;
        }
        if (dmr_search < id) {
            dmr_end = dmr_test;
        } else {
            dmr_begin = next_line_ptr(dmr_test);
        }
    }
    return 0;
}

static int find_dmr_user(char *outstr, int dmr_search, const char *data, int outsize)
{
    const long datasize = getfirstnumber(data);

    // filesize @ 20160420 is 2279629 bytes
    //          @ 20170213 is 2604591 bytes
    if (datasize == 0 || datasize > 7340031){  // 7 Meg sanity limit
       printf("Error: user data maxsize fault");
       return(0);
    }

    const char *data_start = next_line_ptr(data);
    const char *data_end = data_start + datasize; // exclusive
    return find_dmr(outstr, dmr_search, data_start, data_end, outsize);
}

//#define _IS_TRAIL(buf, i, l) ((i > 0 && buf[i-1] == ',' && buf[i] == ' ') || (i < l && buf[i+1] == ',' && buf[i] == ' '))
//
//uint8_t get_dmr_user_field(uint8_t field, char *outstr, int dmr_search, int outsize)
//{
//    char buf[BSIZE];
//    uint8_t pos = 0;
//    uint8_t found = 0;
//    if ( find_dmr_user(buf, dmr_search, (void *) 0x100000, BSIZE) ) {
//        for (uint8_t i = 0; i < BSIZE; i++) {
//          if (buf[i] == 0 || pos >= outsize) {
//              break;
//          }
//          if (buf[i] == ',') {
//              found++;
//          }
//          if (found >= (field - 1) && buf[i] != ',' && !_IS_TRAIL(buf, i, BSIZE)) {
//              outstr[pos] = buf[i];
//              pos++;
//          }
//          if (found == field) {
//              break;
//          }
//        }
//    }
//    return pos;
//}

void usr_splitbuffer(user_t *up)
{
    char *cp = up->buffer ;
    char *start = up->buffer ;

    for(int fld=0;fld<8;fld++) {

        while(1) {
            if( *cp == 0 ) {
                break ;
            }
            if( *cp == ',' ) {
                *cp = 0 ;
                cp++ ;
                break ;
            }
            cp++ ;
        }
        
		switch (fld) {
		case 0:
			up->id = start;
			break;
		case 1:
				if (start[0] == '|') {
					up->callsign = start+1;
					up->fUserType = 1;
				}
				else {
					up->callsign = start;
					up->fUserType = 0;
				}
                break ;
            case 2 :
                up->name = start ;
                break ;
            case 3 :
                up->place = start ;
                break ;
            case 4 :
                up->state = start ;
                break ;
            case 5 :
                up->firstname = start ;
                break ;
            case 6 :
                up->country = start ;
                break ;
        }
        
        start = cp ;
    }
}

void usr_tobuffer(user_t* up, char* data)
{
    char* buf = data;
    if(strlen(up->id)>0){
        strcpy((char*)buf, up->id);
        buf=buf+strlen(up->id)+1;
        *(char*)(buf-1)=',';
    }else{
        buf[0]=',';
        buf=buf+1;
    }

    if(strlen(up->callsign)>0){
        strcpy((char*)buf, up->callsign);
        buf=buf+strlen(up->callsign)+1;
        *(char*)(buf-1)=',';
    }else{
        buf[0]=',';
        buf=buf+1;
    }

    if(strlen(up->name)>0){
        strcpy((char*)buf, up->name);
        buf=buf+strlen(up->name);
        *(char*)(buf-1)=',';
    }else{
        buf[0]=',';
        buf=buf+1;
    }

    if(strlen(up->place)>0){
        if(up->place[0]=='\x01'){
            memcpy((char*)buf, up->place, 3);
            buf[3]=',';
            buf=buf+4;
        }else{
            strcpy((char*)buf, up->place);
            buf=buf+strlen(up->place)+1;
            *(char*)(buf-1)=',';
        }
    }else{
        buf[0]=',';
        buf=buf+1;
    }

    if(strlen(up->state)>0){
        if(up->state[0]=='\x01'){
            memcpy((char*)buf, up->state, 3);
            buf[3]=',';
            buf=buf+4;
        }else{
            strcpy((char*)buf, up->state);
            buf=buf+strlen(up->state)+1;
            *(char*)(buf-1)=',';
        }
    }else{
        buf[0]=',';
        buf=buf+1;
    }

    if(strlen(up->firstname)>0){
        if(strlen(up->firstname)>0){
            strcpy((char*)buf, up->firstname);
            buf=buf+strlen(up->firstname)+1;
            *(char*)(buf-1)=',';
        }else{
            *(char*)(buf)=',';
            buf=buf+1;
        }
    }else{
        buf[0]=',';
        buf=buf+1;
    }

    if(strlen(up->country)>0){
        if(up->country[0]=='\x01'){
            memcpy((char*)buf, up->country, 3);
            buf[3]='\\';
            buf[4]='\n';
            buf=buf+5;
        }else{
            strcpy((char*)buf, up->country);
            buf=buf+strlen(up->country)+2;
            *(char*)(buf-2)='\\';
            *(char*)(buf-1)='\n';
        }
    }else{
        buf[0]=',';
        buf[1]='\\';
        buf[2]='\n';
        buf=buf+3;
    }
    
}

int usr_find_by_dmrid( user_t *up, int dmrid, char* data )
{
    if( !find_dmr_user(up->buffer, dmrid, data, BSIZE) ) {
        // safeguard
        up->buffer[0] = '?' ;
        up->buffer[1] = 0 ;
        usr_splitbuffer(up);
    
        return 0 ;
    }
    
    // safeguard
    up->buffer[BSIZE-1] = 0 ;
    
    usr_splitbuffer(up);
    return 1 ;
}
