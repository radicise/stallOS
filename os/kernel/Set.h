#ifndef __SET_H__
#define __SET_H__1
#include "types.h"
#include "kmemman.h"
#define SET_BS 16
#define FAILMASK_SET 0x00080000
struct Set {
	struct Set_block* start;
	long amnt;
};
struct Set_block {
	uintptr used;
	uintptr next;
	uintptr data[SET_BS - 2];
};
struct Set* Set_create(void) {
	struct Set* la = alloc(sizeof(struct Set));
	la->start = NULL;
	la->amnt = 0;
	return la;
}
void Set_destroy(struct Set* la) {
	struct Set_block* lb = la->start;
	while (1) {
		if (lb == NULL) {
			dealloc(la, sizeof(struct Set));
			return;
		}
		struct Set_block* lc = (struct Set_block*) lb->next;
		dealloc(lb, sizeof(struct Set_block));
		lb = lc;
	}
}
int Set_remove(uintptr dat, struct Set* ll) {// Success: 0; Not found: -1
	struct Set_block* lb = ll->start;
	struct Set_block* last = NULL;
	while (1) {
		if (lb == NULL) {
			return (-1);
		}
		uintptr amnt = lb->used;
		uintptr* data = lb->data;
		for (int i = 0; i < amnt; i++) {
			if (data[i] == dat) {
				lb->used--;
				for (int j = i + 1; j < amnt; j++) {
					data[j - 1] = data[j];
				}
				if (lb->used == 0) {
					struct Set_block* next = (struct Set_block*) lb->next;
					dealloc(lb, sizeof(struct Set_block));
					if (last == NULL) {
						ll->start = next;
					}
					else {
						last->next = (uintptr) next;
					}
				}
				ll->amnt--;
				return 0;
			}
		}
		last = lb;
		lb = (struct Set_block*) lb->next;
	}
}
int Set_contains(uintptr dat, struct Set* ll) {// Contains given element: 1; Does not contain given element: 0
	struct Set_block* lb = ll->start;
	while (1) {
		if (lb == NULL) {
			return 0;
		}
		uintptr amnt = lb->used;
		uintptr* data = lb->data;
		for (int i = 0; i < amnt; i++) {
			if (data[i] == dat) {
				return 1;
			}
		}
		lb = (struct Set_block*) lb->next;
	}
}
int Set_add(uintptr dat, struct Set* ll) {// Success: 0; Already exists: -1
	if (Set_contains(dat, ll)) {
		return (-1);
	}
	struct Set_block* lb = ll->start;
	struct Set_block* last = NULL;
	while (1) {
		if (lb == NULL) {
			lb = alloc(sizeof(struct Set_block));
			lb->used = 1;
			lb->next = (uintptr) NULL;
			if (last == NULL) {
				ll->start = lb;
			}
			else {
				last->next = (uintptr) lb;
			}
			lb->data[0] = dat;
			ll->amnt++;
			return 0;
		}
		uintptr amnt = lb->used;
		if (amnt != (SET_BS - 2)) {
			lb->data[amnt] = dat;
			lb->used++;
			ll->amnt++;
			return 0;
		}
		last = lb;
		lb = (struct Set_block*) lb->next;
	}
}
uintptr Set_containsByCompare(uintptr dat, int (*comparator)(uintptr, uintptr), struct Set* ll) {// Contains a match: The match; Does not contain a match: (uintptr) -1
	struct Set_block* lb = ll->start;
	while (1) {
		if (lb == NULL) {
			return (-1);
		}
		uintptr amnt = lb->used;
		uintptr* data = lb->data;
		for (int i = 0; i < amnt; i++) {// comparison is guaranteed to be in the order `comparator(<suspected match>, <provided value>)'
			if (!(comparator(data[i], dat))) {// comparator(A, B) == 0: A matches with B according to the comparator; comparator(A, B) != 0: A does not match with B according to the comparator
				return data[i];
			}
		}
		lb = (struct Set_block*) lb->next;
	}
}
#endif