#include "common.h"
#include "userdb.h"
#include <sys/socket.h>

#include <fstream>
#include <string.h>
#include <string>

#include <vector>
#define _BSD_SOURCE
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

void printUser(user_t* user){
    printf("\r\nDMRID: %s", user->id);
    printf("\r\nCallsign: %s", user->callsign);
    printf("\r\nFirstName: %s", user->firstname);
    printf("\r\nName: %s", user->name);
    printf("\r\nCountry: %s", user->country);
    printf("\r\nPlace: %s", user->place);
    printf("\r\nState: %s", user->state);
    printf("\r\n----------------");
}



typedef struct _dictentry {
    string entry;
    int count;
    _dictentry(){
        entry="";
        count=1;
    }
    bool operator== (const _dictentry & rhs) const {
        return (entry == rhs.entry);
    }
    
} dictentry;

int find (vector<_dictentry> *ent, _dictentry* nent)
{
    for(int i=0; i<ent->size(); i++)
    {
        if (ent->at(i).entry == nent->entry) 
            return i;
    }
    return -1;
}

int processDict (vector<_dictentry> *main, vector<_dictentry> *toadd)
{
    for(int i=0; i<toadd->size(); i++)
    {
        if (toadd->at(i).count >= 5){
            main->push_back(toadd->at(i));
        }
    }
    return -1;
}

int getDictIndex (vector<_dictentry> *main, string srch)
{
    for(int i=0; i<main->size(); i++)
    {
        if (main->at(i).entry == srch){
            return i;
        }
    }
    return -1;
}

enum TLV { TAG_BEGIN_TX=0, TAG_AMBE=1, TAG_END_TX=2, TAG_TG_TUNE=3, TAG_PLAY_AMBE=4, TAG_REMOTE_CMD=5, TAG_AMBE_49=6, TAG_AMBE_72=7, TAG_SET_INFO=8, TAG_DMR_TEST=9 };

using namespace std;

void paddr(unsigned char *a) { printf("%d.%d.%d.%d\n", a[0], a[1], a[2], a[3]); }

struct hostent *hp; /* host information */
struct sockaddr_in servaddr; 
#define BUFLEN 512  //Max length of buffer

int main(int argc, char** argv){

    struct hostent *hp; char *host = "74.91.126.116"; int i;

    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];
    
    FILE* fuserdb = fopen(argv[1], "rb");
    long flen = 0;

    if(!fuserdb){
        printf("Error opening %s\r\n", argv[1]);
        fgetc(stdin);
        exit(1);
    }

    hp = gethostbyname(host); 
    if (!hp) { fprintf(stderr, "could not obtain address of %s\n", host); return 0; } 

    for (i=0; hp->h_addr_list[i] != 0; i++) 
        paddr((unsigned char*) hp->h_addr_list[i]);

  
    s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
     int broadcast=0;
  setsockopt(s, SOL_SOCKET, SO_BROADCAST,
        &broadcast, sizeof broadcast);

        memset(&si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(44300);
    
    memcpy(&si_other.sin_addr, hp->h_addr_list[0], 4);

    fseek (fuserdb, 0, SEEK_END);
    flen=ftell (fuserdb);
    fseek (fuserdb, 0, SEEK_SET);


    char* filebuffer = (char*)malloc(flen);
    fread(filebuffer, flen, 1, fuserdb);

    fclose (fuserdb);

    ssize_t ret = 0;
    unsigned int talkgroup_num = 9;
	unsigned int radio_id = 1112526;
    static int REPEATER_ID = 1112526;

    char header_rptr_info[] = {0x00,
     0x0C, 
    ((char *)&radio_id)[2], 
        ((char *)&radio_id)[1], 
    ((char *)&radio_id)[0],                                                            0x00, 
      ((char *)&REPEATER_ID)[2], 
       ((char *)&REPEATER_ID)[1], 
             ((char *)&REPEATER_ID)[0], 
            ((char *)&talkgroup_num)[2], 
            ((char *)&talkgroup_num)[1], 
               ((char *)&talkgroup_num)[0], 
                  0x02, 
                  0x01};

    ret = sendto(s, (void *)header_rptr_info, 14, 0, (const sockaddr*)&si_other, slen);
     printf("sending start tlv.. %d\r\n", ret);
    for(int i=0; i<flen; i+=9*3){
        usleep(60000);

        buf[0]=TAG_AMBE_72;
        buf[1]=0x1C;
        buf[2]=0x02;
        memcpy(&buf[3], filebuffer+i, 9*3);
        ret = sendto(s, buf, 30, 0, (const sockaddr*)&si_other, slen);
        printf("sending.. %d\r\n", ret);        
    }

    usleep(60000);
    printf("Sending END TLV");
	char TERM[3] = {0x02, 0x01, 0x02};
	ret = sendto(s, (void *)TERM, 3, 0, (const sockaddr*)&si_other, slen);
	printf("%d\r\n", ret);

    close(s);

    free(filebuffer);
    printf("\r\nDone");
    fgetc(stdin);
    return 0;
}