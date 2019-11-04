#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#define FREEW_SPACE 65536 
#define FREEP_SPACE 100000
#define FREENODE_SPACE 1200000
#define WRONGWORD_MAXOCC 10000
#define READC c=fgetc(in)|0b100000;counter++;if(feof(in))break;
#define HASH BKDRHash
#define atoind(c) (c-'a')
#define MOD 50000


#define likely(x)       (x)
#define unlikely(x)     (x)
typedef unsigned int keytype;
typedef struct bnode {
	struct bnode* next[26];
	int isLeaf;
} __attribute__ ((aligned (256))) bnode;
typedef struct wrongplace {
	unsigned int place;
	struct wrongplace* next;
} __attribute__ ((aligned (16))) WrongPlace;

typedef struct wrongspellings {
	char str[36];
	unsigned int num;
	WrongPlace* head;
	WrongPlace* tail;
	struct wrongspellings* hashnext;
} __attribute__ ((aligned (64))) WrongWord;

bnode freenodes[FREENODE_SPACE];
WrongWord freeW[FREEW_SPACE];
WrongPlace freeP[FREEP_SPACE];

inline unsigned int BKDRHash(char *str) {
	unsigned int seed = 9267; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (*str) {
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF) % MOD;
}
inline unsigned int JSHash(const char* str) {
	unsigned int hash = 1315423911;
	while (*str) {
		hash ^= ((hash << 5) + *str++ + (hash >> 2));
	}
	return (hash & 0x7fffffff) % MOD;
}
inline void swap(keytype* a, keytype* b) {
	keytype	c = *a;
	*a = *b;
	*b = c;
}
void bubbleSort(keytype k[ ],int n)
{     

		int i, j, flag=1;
       keytype temp;
       for(i=n-1; i>0 && flag==1; i--){
              flag=0;                      
              for(j=0;j<i;j++) 
                  //  if( k[j]>k[j+1] ){
                  if( freeW[k[j]].num < freeW[k[j+1]].num || (freeW[k[j]].num == freeW[k[j+1]].num && strcmp(freeW[k[j]].str,freeW[k[j+1]].str) > 0 )){
                         temp=k[j];
                         k[j]=k[j+1];
                         k[j+1]=temp;  
                         flag=1;
                    }
        }

 }
 
void quickSort(keytype k[ ],int left,int right)
{

	int i, last;

	if(left<right) {

		last=left;

		for(i=left+1; i<=right; i++) {

			if(freeW[k[i]].num > freeW[k[left]].num || (freeW[k[i]].num == freeW[k[left]].num && strcmp(freeW[k[i]].str,freeW[k[left]].str)<0))
				swap(&k[++last],&k[i]);
		}

		swap(&k[left],&k[last]);

		quickSort(k,left,last-1);

		quickSort(k,last+1,right);

	}

}

int mergeSort(keytype k[ ],int n)
{
    keytype *tmp;
    tmp = (keytype *)malloc(sizeof(keytype) * n);
    if(tmp != NULL) {
        mSort(k, tmp, 0, n-1);
        free(tmp);
    } 

}
void mSort(keytype k[], keytype tmp[], int left, int right)
{
    int center;
    if(left < right){
        center = (left+right)/2;
        mSort(k, tmp, left, center);
        mSort(k, tmp, center+1, right);
        merge(k, tmp, left,center, right);
    }
}

int merge(unsigned short x[ ],unsigned short tmp[ ],int left,int leftend,int rightend)

{     
	int num=0;
    int i=left, j=leftend+1, q=left;

    while(i<=leftend && j<=rightend)

    {
	//	++qnum;
        if( freeW[x[i]].num > freeW[x[left]].num || (freeW[x[i]].num == freeW[x[left]].num && strcmp(freeW[x[i]].str,freeW[x[left]].str)<0) ) 

            tmp[q++]=x[i++];

        else

            tmp[q++]=x[j++];

    }

    while(i<=leftend)

        tmp[q++]=x[i++];

    while(j<=rightend)

        tmp[q++]=x[j++];

    for(i=left; i<=rightend; i++)

        x[i]=tmp[i];

}

int main() {
	WrongWord* hashtable[MOD] = {0};
	printf("%d",sizeof(WrongWord));
	volatile unsigned int freeW_i=0;
	volatile unsigned int freenodes_i=0;
	volatile unsigned int freeP_i=0;
	char buffer[100];
	bnode root[26];
	FILE *dic=fopen("dictionary.txt","r");
	volatile bnode *curnode;
	volatile int ind;
	unsigned int i;
	while(1) {
		fgets(buffer,100,dic);
		if(feof(dic))
			break;
		curnode=&root[atoind(buffer[0])];
		for(i=1; buffer[i]>='a'&&buffer[i]<='z'; i++) {
			ind=atoind(buffer[i]);
			if(likely(curnode->next[ind]==0)) {
				curnode->next[ind]=&freenodes[freenodes_i++];
			}
			curnode=curnode->next[ind];
		}
		curnode->isLeaf = 1;
	}
	FILE *in = fopen("article.txt","r");
	char c;
	volatile unsigned int counter=1;
	c=fgetc(in)|0b100000;
	while(1) {
		if(feof(in))
			break;
		while(c<'a' || c >'z') {
			READC
		}
		curnode=&root[atoind(c)];
		buffer[0]=c;
		i=1;
		while(1) {
			READC
			if(unlikely(c<'a' || c>'z')) {
				buffer[i++] = 0;
				if(unlikely(curnode->isLeaf == 0)) {
					int hashnum = HASH(buffer);
					WrongWord* curnode;

					if(likely(hashtable[hashnum] == NULL)) {
						hashtable[hashnum] = &freeW[freeW_i++];
						curnode = hashtable[hashnum];
						curnode->num = 1;
						curnode->head = &freeP[freeP_i++];
						curnode->head->place = counter - i;
						curnode->tail = curnode->head;
						strcpy(curnode->str,buffer);
					} else {
						curnode = hashtable[hashnum];
						while(1) {
							if(likely(strcmp(curnode->str,buffer) == 0)) {
								curnode->num++;
								curnode->tail->next = &freeP[freeP_i++];
								curnode->tail = curnode->tail->next;
								curnode->tail->place = counter - i;
								break;
							}
							if(unlikely(curnode->hashnext == NULL)) {
								curnode->hashnext = &freeW[freeW_i++];
								curnode = curnode->hashnext;
								curnode->num = 1;
								curnode->head = &freeP[freeP_i++];
								curnode->head->place = counter - i;
								curnode->tail = curnode->head;
								strcpy(curnode->str,buffer);
								break;
							}
							curnode = curnode->hashnext;
						}
					}
					
					break;
				}

				break;
			}

			if(curnode->next[atoind(c)]==0) {
				while(c>='a' && c <= 'z') {
					buffer[i++]=c;
					c=fgetc(in)|0b100000;
					counter++;
				}
				buffer[i++]=0;


				int hashnum = HASH(buffer);
				WrongWord* curnode;

				if(likely(hashtable[hashnum] == NULL)) {
					hashtable[hashnum] = &freeW[freeW_i++];
					curnode = hashtable[hashnum];
					curnode->num = 1;
					curnode->head = &freeP[freeP_i++];
					curnode->head->place = counter - i;
					curnode->tail = curnode->head;
					strcpy(curnode->str,buffer);
					//memset(&(curnode->hashnext),0,3*sizeof(WrongWord*));

				} else {
					curnode = hashtable[hashnum];
					while(1) {
						if(likely(strcmp(curnode->str,buffer) == 0)) {
							curnode->num++;
							curnode->tail->next = &freeP[freeP_i++];
							curnode->tail = curnode->tail->next;
							curnode->tail->place = counter - i;
							break;
						}
						if(unlikely(curnode->hashnext == NULL)) {
							curnode->hashnext = &freeW[freeW_i++];
							curnode = curnode->hashnext;
							curnode->num = 1;
							curnode->head = &freeP[freeP_i++];
							curnode->head->place = counter - i;
							curnode->tail = curnode->head;
							strcpy(curnode->str,buffer);
							//memset(&(curnode->hashnext),0,2*sizeof(WrongWord*));
							break;
						}
						curnode = curnode->hashnext;
					}
				}

				break;
			}
			buffer[i++]=c;
			curnode = curnode->next[atoind(c)];
		}
	}
	FILE* out = fopen("misspelling.txt","w");
	keytype Wlist[FREEW_SPACE];
	keytype j;
	for(j=0; j<freeW_i; j++) {
		Wlist[j] = j;
	}
	quickSort(Wlist,0,freeW_i-1);
//	bubbleSort(Wlist,freeW_i);
	for(j=0; j<freeW_i; j++) {
		fprintf(out, "%s %d ", freeW[Wlist[j]].str, freeW[Wlist[j]].num);
		WrongPlace *place = freeW[Wlist[j]].head;
		while(place!=NULL){
			fprintf(out,"%d ",place->place);
			place = place->next;	
		}
		fputc('\n',out);
	}
//	printf("\n%d",clock());
	
//	out = fopen("misspellingt.txt","w");
//	unsigned short Wlist[FREEW_SPACE];
//	unsigned short j;
/*	for(j=0; j<freeW_i; j++) {
		Wlist[j] = j;
	}
	quickSort(Wlist,0,freeW_i-1);*/
	/*
	for(j=0; j<freeW_i; j++) {
		fprintf(out, "%s %d ", freeW[j].str, freeW[j].num);
		WrongPlace *place = freeW[j].head;
		while(place!=NULL){
			fprintf(out,"%d ",place->place);
			place = place->next;	
		}
		fputc('\n',out);
	}*/
}
