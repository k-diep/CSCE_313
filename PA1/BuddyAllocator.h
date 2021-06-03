#ifndef _BuddyAllocator_h_                   // include file only once
#define _BuddyAllocator_h_
#include <iostream>
#include <vector>
#include "math.h"
using namespace std;
typedef unsigned int uint;

/* declare types as you need */

class BlockHeader{
public:
	// think about what else should be included as member variables
	int block_size;  // size of the block
	BlockHeader* next; // pointer to the next block
	bool free; //check if the block is free
};

class LinkedList{
	// this is a special linked list that is made out of BlockHeader s. (single linked list)
public:
	BlockHeader* head;		// you need a head of the list
	uint size;				// size of the list
public:
	LinkedList(){
		head = nullptr;
		size = 0;
	}

	void insert (BlockHeader* b){	// adds a block to the list
		//insert at the beginning
		//when list is empty
		if (head == nullptr){
			head = b;
			size = size + 1;

		}
		//when list is not empty
		else{
			BlockHeader* oldHead = head;
			head = b;
			head->next = oldHead;

			size = size + 1;
		}
	}

	void remove (BlockHeader* b){  // removes a block from the list
		BlockHeader* current = head;
		if (current == b){
			head = current->next;
			current->next = nullptr;
		}
		else{
			while(current->next != b){
				current = current->next;
			}
			current->next = current->next->next;
		}
		size = size - 1;
	}

	BlockHeader* get_head(){
		return head;
	}

    uint get_size(){
        return size;
    }

    void print (){
    	BlockHeader* curr = head;

    	while (curr != nullptr) {
    		cout<< curr->block_size<< " ";
    		curr = curr->next;
    	}
    }

    ~LinkedList(){
    	BlockHeader* curr = head;
    	while (curr != nullptr) {
    		BlockHeader* deleteNode = curr;
    		curr = curr->next;
    		delete deleteNode;
    	}
    	head = nullptr;
    }

};


class BuddyAllocator{
public:
	/* declare more member variables as necessary */
	vector<LinkedList> FreeList;
	uint basic_block_size;
	uint total_memory_size;

	char* baseMem;
	uint FreeListLength;



public:
	/* private function you are required to implement
	 this will allow you and us to do unit test */
	
	BlockHeader* getbuddy (BlockHeader * addr); 
	// given a block address, this function returns the address of its buddy 
	
	bool arebuddies (BlockHeader* block1, BlockHeader* block2);
	// checks whether the two blocks are buddies are not
	// note that two adjacent blocks are not buddies when they are different sizes

	BlockHeader* merge (BlockHeader* block1, BlockHeader* block2);
	// this function merges the two blocks returns the beginning address of the merged block
	// note that either block1 can be to the left of block2, or the other way around

	BlockHeader* split (BlockHeader* block);
	// splits the given block by putting a new header halfway through the block
	// also, the original header needs to be corrected


public:
	BuddyAllocator (uint _basic_block_size, uint _total_memory_length);
	/* This initializes the memory allocator and makes a portion of 
	   ’_total_memory_length’ bytes available. The allocator uses a ’_basic_block_size’ as 
	   its minimal unit of allocation. The function returns the amount of 
	   memory made available to the allocator. 
	*/ 

	~BuddyAllocator(); 
	/* Destructor that returns any allocated memory back to the operating system. 
	   There should not be any memory leakage (i.e., memory staying allocated).
	*/ 

	char* alloc(int _length);
	/* Allocate _length number of bytes of free memory and returns the 
		address of the allocated portion. Returns 0 when out of memory. */ 

	void free(void* _a); 
	/* Frees the section of physical memory previously allocated 
	   using alloc(). */ 
   
	void printlist ();
	/* Mainly used for debugging purposes and running short test cases */
	/* This function prints how many free blocks of each size belong to the allocator
	at that point. It also prints the total amount of free memory available just by summing
	up all these blocks.
	Aassuming basic block size = 128 bytes):

	[0] (128): 5
	[1] (256): 0
	[2] (512): 3
	[3] (1024): 0
	....
	....
	 which means that at this point, the allocator has 5 128 byte blocks, 3 512 byte blocks and so on.*/
};

#endif 
