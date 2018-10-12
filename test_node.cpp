#include <stdlib.h>
#include <iostream>

struct cache_block {
	bool valid = false;
	bool dirty_bit = false;
	int tag;
	
};

struct node {
	int data;
	node* next;
};

int main(void) {
	node* n;
	node* t;
	node* h;

	n = new node;
	n->data = 1;
	t = n;
	h = n;

	n = new node;
	n->data = 2;
	t->next = n;
	t = t->next; // or, t = n;

	n = new node;
	n->data = 3;
	t->next = n;
	n->next = NULL;
	t = t->next;

	std::cout << "head address : " << h << std::endl;
	std::cout << "head data : " << h->data << std::endl;
	std::cout << "head next : " << h->next << std::endl;
	std::cout << "node address : " << h->next << std::endl;
	std::cout << "node data : " << h->next->data << std::endl;
	std::cout << "node next : " << h->next->next << std::endl;
	std::cout << "tail address : " << t << std::endl;
	std::cout << "tail data : " << t->data << std::endl;
	std::cout << "tail next : " << t->next << std::endl;


	return 0;
}