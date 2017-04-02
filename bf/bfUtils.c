#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************************
*				LRULIST FUNCTIONS					*
*****************************************************/
LRU* lru_init(){ 
	LRU *new=malloc(sizeof(LRU));
	new->head=NULL;/*at the beginning all free page are in the FREE list*/
	new->tail=NULL;
	new->number_of_page=0;

	return new; /*the LRU structure is now initialized, this pointer will be used for every funtion using the LRU list*/
}

int lru_add(LRU* lru, BFpage *new_BFpage){
	/*this BFpage become the most recently used, consequently it is the head of the LRU list*/
 	
	if(lru->tail==NULL){ /*first page in the LRU*/
		printf("liste vide \n");
                new_BFpage->nextpage=NULL;
		new_BFpage->prevpage=NULL;
		lru->head=new_BFpage;
		lru->tail=new_BFpage;
			
	}
	else{
		lru->head->prevpage= new_BFpage;/*just add the new BFpage before the head.*/

		new_BFpage->nextpage= lru->head;

		lru->head=new_BFpage; 	/*The new page become consequently the head.*/
		
	}
	lru->number_of_page+=1;
	return BFE_OK;
}

char lru_find(LRU* lru, BFpage *page){ 

	if (lru->head == NULL) return BFE_NOBUF; /*empty list case*/
	
	BFpage *pt= lru->head;
	do{
	     if(pt==page) return BFE_OK;/* one address in memory for a page ==> the pointer are equal if they point the same page */
	     pt=pt->nextpage;
	}while(pt!=NULL); /*stop the loop after the tail*/

	return BFE_PAGENOTINBUF;/* not find*/
}

int lru_remove(LRU* lru,BFpage** victim){
	
	if (lru->tail==NULL) return BFE_PAGENOTINBUF; /*empty list case*/

	/*the tail is the last recently used page*/
	BFpage *pt= lru->tail;

	do{

	     if(pt->count==0){ /* the victim is pointed by pt*/

		/* to remove from the lru list, change pointer is enough */ 
		if (pt != lru->head){
	

  
			if(pt!=lru->tail) {
				pt->prevpage->nextpage=pt->nextpage; 
				pt->nextpage->prevpage=pt->prevpage;
			 }
			
			else{	
				
				pt->prevpage->nextpage=NULL;/*tail victim ==> the page before tail becomes the tail	*/	lru->tail=pt->prevpage;
			}
		}
		else{
			if(pt!=lru->tail) {
					pt->nextpage->prevpage=NULL;/* head victim ==> next page becomes head*/
					lru->head=pt->nextpage;
			 }
			else{/* the head is the tail*/
				lru->head=NULL;
				lru->tail=NULL;
			}
					
		
		}
		/************ victim is removed ********************************/
		pt->nextpage=NULL;
		pt->prevpage=NULL;
		lru->number_of_page-=1;
		(*victim)=pt;
		return BFE_OK;
	    }
		
		
	     pt=pt->prevpage;
	}while(pt!=NULL); /*stop the loop after the tail*/
	
	return BFE_PAGENOTINBUF; /* if NULL is returned then the list is empty (first condition check, or after scanning all list: no page can be choose as a victim*/
}

void lru_print(LRU* lru){
	if(lru->head==NULL){
		printf(" \n EMPTY List \n\n");
		return;
	}
	printf(" There is %d pages in the LRU list \n\n ", lru->number_of_page);

	BFpage *pt= lru->head;

	do{
             printf(" Data: %s             Dirty flag: %d           pin: %d             Fd - page number: %d - %d \n\n", pt->fpage.pagebuf, pt->dirty, pt->count, pt->fd, pt->pageNum);

	     pt=pt->nextpage;
	}while(pt!=NULL); /*stop the loop after the tail*/
	return;
}

int lru_mtu(LRU* lru, BFpage* mtu_page){ /*this mtu_page (considered already in the list) is hit ==> most recently used page which become the head*/
	if (lru->tail==NULL) return BFE_NOBUF; /*empty list case ==> not normal because the mtu_page is supposed to be in the LRU*/
	
	BFpage *pt= lru->head;
	do{
             if(pt==mtu_page) {
		if (pt != lru->head){ /*mtu_page is not the head*/
			/* mtu_page has to be removed from its place in the list */
			if(pt!=lru->tail) {
				pt->prevpage->nextpage=pt->nextpage; 
				pt->nextpage->prevpage=pt->prevpage;
			 }
			
			else{
				pt->prevpage->nextpage=NULL;/*tail victim ==> the page before tail becomes the tail	*/	
				lru->tail=pt->prevpage;
			}
			/* now mtu_page is removed, and become the head of the list */
			mtu_page->nextpage=lru->head;
			lru->head->prevpage=mtu_page;
			lru->head=mtu_page;
			
		}
		else{
			/* mtu_page is already the head.*/
		}
		return BFE_OK;
	     }	     
	     pt=pt->nextpage;
	}while(pt!=NULL); /*stop the loop after the tail*/

	return BFE_PAGENOTINBUF;
}


/****************************************************
*				FREELIST FUNCTIONS					*
*****************************************************/
Freelist* fl_init(int max_size){
	int i;
	BFpage* newPage;
	BFpage* prev;

	Freelist* fl = malloc(sizeof(Freelist)); /*address of the beginning of the free space */
	
	if (fl == NULL){exit(EXIT_FAILURE);}

	fl->max_size = max_size;
	fl->size = 1;
	fl->head = malloc(sizeof(BFpage)); /*allocate space for the first BFpage*/
	if(fl->head == NULL){exit(EXIT_FAILURE);}

	strcpy(fl->head->fpage.pagebuf, "");
	prev = fl->head;
	prev->dirty = FALSE;
	prev->count = 0;
	prev->pageNum = 0;
	prev->fd = 0;

	for(i = 1; i< max_size; i++){
		newPage = malloc(sizeof(BFpage));
		if(newPage == NULL){exit(EXIT_FAILURE);}
		strcpy(newPage->fpage.pagebuf, "");
		prev->nextpage = newPage;
		prev = newPage;
		fl->size += 1;
	}

	newPage->nextpage = NULL;
	return fl;
}

BFpage* fl_give_one(Freelist* fl){
	if(fl == NULL){exit(EXIT_FAILURE);}
	if(fl->size == 0){return NULL;}
	if(fl->head != NULL){
		BFpage* res = fl->head;
		fl->head = fl->head->nextpage;
		fl->size -= 1;
		return res;
	}
}

int fl_add(Freelist* fl, BFpage* bpage){
	if(fl == NULL || bpage == NULL){exit(EXIT_FAILURE);}
	if(fl->size >= fl->max_size){printf("\nFL ERROR : Max Size reached");exit(EXIT_FAILURE);}

	/*Cleaning the data into the BFPage*/
	bpage->dirty = FALSE;
	bpage->count = 0;
	bpage->pageNum = 0;
	bpage->fd = 0;
	bpage->prevpage = NULL;

	/*Adding it at the beginning of the freelist*/
	bpage->nextpage = fl->head;
	fl->head = bpage;
	fl->size += 1;
}

int fl_free(Freelist* fl){
	int i;
	if(fl == NULL){exit(EXIT_FAILURE);}
	if(fl->head != NULL){
		BFpage* el = fl->head;
		BFpage* temp;	
		for(i = 0; i < fl->size; i++){
			temp = el;
			el = el->nextpage;
			free(temp);
		}
		free(fl);
		return 0;
	}
	return 1; /*error*/
}

void fl_print(Freelist* fl){
	BFpage* ptr = fl->head;

	printf("\n------PRINTING LIST------");
	printf("\n This freelist has %d entries\n", fl->size);
	printf("\npagebuf data\tdirty\tcount\tpageNum\tfd ");
	while(ptr!=NULL){
		printf("\n[%s]", ptr->fpage.pagebuf);
		printf("\t\t[%d]", ptr->dirty);
		printf("\t[%d]", ptr->count);
		printf("\t[%d]", ptr->pageNum);
		printf("\t[%d]", ptr->fd);
		ptr = ptr->nextpage;
	}
	printf("\n------END PRINTING ------\n\n");
	return;
}

/****************************************************
*				HASHTABLE FUNCTIONS					*
*****************************************************/

int ht_hashcode(Hashtable* ht, int fd, int pageNum) {
	return (123 * fd * pageNum + 87) % 31 % ht->size;
}

Hashtable* ht_init(size_t size) {
	Hashtable* ht = malloc(sizeof(Hashtable));
	ht->size = size;
	ht->entries = malloc(sizeof(BFhash_entry*) * size); /* allocate memory for all entry pointers */
	size_t i;
	for (i = 0; i < size; i++) {
		ht->entries[i] = NULL;
	}
	return ht;
}

int ht_add(Hashtable* ht, BFpage* page) {
	int hc = ht_hashcode(ht, page->fd, page->pageNum);
	if (ht_get(ht, page->fd, page->pageNum) != NULL) {
		return NULL; /* entry already in hashtable */
	}
	else { /* entry needs to be created and inserted */
		BFhash_entry* newEntry = malloc(sizeof(BFhash_entry));
		newEntry->fd = page->fd;
		newEntry->pageNum = page->pageNum;
		newEntry->bpage = page;
		BFhash_entry* entry = ht->entries[hc];
		if (entry == NULL) { /* bucket is empty, insert it right here */
			ht->entries[hc] = newEntry;
			newEntry->preventry = NULL;
			newEntry->nextentry = NULL;
		}
		else { /* we have to iterate to last place of this bucket */
			while (entry->nextentry != NULL) {
				entry = entry->nextentry;
			}
			newEntry->preventry = entry;
			entry->nextentry = newEntry;
			newEntry->nextentry = NULL;
		}
	}
	return 0;
}

int ht_remove(Hashtable* ht, int fd, int pageNum) {
	int hc = ht_hashcode(ht, fd, pageNum);
	BFhash_entry* e = ht_get(ht, fd, pageNum);
	if (e == NULL) return -1; /* not found */
	BFhash_entry* prev = e->preventry;
	BFhash_entry* next = e->nextentry;
	if (prev != NULL && next != NULL) { /* between two entries */
		prev->nextentry = next;
		next->preventry = prev;
	} 
	else if (prev != NULL && next == NULL) { /* when last entry in bucket */
		prev->nextentry = NULL;
	}
	else if (prev == NULL && next != NULL) { /* when first entry in bucket */
		ht->entries[hc] = next;
	}
	else { /* when only entry in bucket */
		ht->entries[hc] = NULL;
	}
	free(e);
	return 0;
}

BFhash_entry* ht_get(Hashtable* ht, int fd, int pageNum) {
	int hc = ht_hashcode(ht, fd, pageNum);
	BFhash_entry* e = ht->entries[hc];
	if (e == NULL) {
		return NULL;
	}
	while (!(e->fd == fd && e->pageNum == pageNum)) {
		e = e->nextentry;
		if (e == NULL) return NULL; /* entry not found */
	}
	return e;
}

int ht_free(Hashtable* ht) {
	size_t i;
	for (i = 0; i < ht->size; i++) {
		BFhash_entry* e = ht->entries[i];
		BFhash_entry* tmp;
		while (e != NULL) {
			tmp = e;
			e = e->nextentry;
			free(tmp);
		}
		ht->entries[i] = NULL;
	}
	free(ht);
}

void ht_print(Hashtable* ht) {
	size_t i;
	for (i = 0; i < ht->size; i++) {
		BFhash_entry* e = ht->entries[i];
		while (e != NULL) {
			printf("bucket %d: fd %d, pagenum %d\n", i, e->fd, e->pageNum);
			e = e->nextentry;
		}
	}
}