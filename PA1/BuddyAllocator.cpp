#include "BuddyAllocator.h"
#include <iostream>
using namespace std;

BuddyAllocator::BuddyAllocator (uint _basic_block_size, uint _total_memory_length){
	uint new_total_size;
	uint new_basic_size;

	new_basic_size = ceil(log2(_basic_block_size));
	new_basic_size = pow(2, new_basic_size);

	new_total_size = ceil(log2(_total_memory_length));
	new_total_size = pow(2, new_total_size);

	basic_block_size  = new_basic_size;
	total_memory_size = new_total_size;

	//allocate a chunk of memory of new_total_size
	baseMem = new char[new_total_size];
	BlockHeader* baseMemStart = (BlockHeader*) baseMem;
	baseMemStart->free = true;
	baseMemStart->next = nullptr;
	baseMemStart->block_size = new_total_size;

	FreeListLength = int(log2(new_total_size/new_basic_size)) + 1;

	//insert into the last element of the vector of linked list (this holds the largest possible size)doyu
	FreeList.resize(FreeListLength);
	FreeList[FreeListLength-1].insert(baseMemStart);
}

BuddyAllocator::~BuddyAllocator (){
	delete[] baseMem;
}

BlockHeader* BuddyAllocator::getbuddy (BlockHeader * addr){
	char* block = (char*) addr;
	char* buddy1 = ((block - baseMem) ^ addr->block_size) + baseMem;
	BlockHeader* buddy2 = (BlockHeader*) buddy1;
	return buddy2;
}

bool BuddyAllocator::arebuddies(BlockHeader* block1, BlockHeader* block2){
	BlockHeader* buddy1 = nullptr;
	BlockHeader* buddy2 = nullptr;
    buddy1 = getbuddy(block1);
    buddy2 = getbuddy(block2);
    //checking to see if both blocks'ss buddy are each other
    bool areBuddies = 0;
    areBuddies = (buddy1 == block2) && (buddy2 == block1);
    areBuddies = areBuddies && buddy2->free;
    return areBuddies;
}

BlockHeader* BuddyAllocator::merge (BlockHeader* block1, BlockHeader* block2){
	//merge assumes they are buddies(handle test in alloc/free)
	BlockHeader* leftBlock  = nullptr;
	BlockHeader* rightBlock = nullptr;
	if( block1 < block2 ) {
		leftBlock  = block1;
		rightBlock = block2;
	}
	else{
		leftBlock  = block2;
		rightBlock = block1;
	}

	uint oldSize = leftBlock->block_size;
	uint newSize = 2*oldSize;
	uint FreeListIndex = log2(leftBlock->block_size / basic_block_size);

	//removing/inserting in/out of the linkedlist

	FreeList[FreeListIndex].remove( leftBlock);
	FreeList[FreeListIndex].remove(rightBlock);
	FreeList[FreeListIndex+1].insert(leftBlock);
	leftBlock->block_size = newSize;

	return leftBlock;
}

BlockHeader* BuddyAllocator::split (BlockHeader* block){
	BlockHeader* leftChild  = nullptr;
	BlockHeader* rightChild = nullptr;
	leftChild  = block;


	uint oldSize = leftChild->block_size;
	uint newSize = oldSize/2;
	uint FreeListIndex = log2(leftChild->block_size / basic_block_size);
	FreeList[FreeListIndex].remove(leftChild);
	leftChild->block_size = newSize;

	rightChild = (BlockHeader*)((char*)leftChild + newSize);
	rightChild->block_size = newSize;
	leftChild->free = true;
	rightChild->free = true;
	FreeList[FreeListIndex-1].insert(leftChild);
	FreeList[FreeListIndex-1].insert(rightChild);
	return leftChild;
}

char* BuddyAllocator::alloc(int length) {
	/* This preliminary implementation simply hands the call over the
   	   the C standard library!
       Of course this needs to be replaced by your implementation.
   */
	//return malloc (length);
	uint totalLength;
	uint index = 0;
	char* allocBlock;
	BlockHeader* block;

	//find and round up the required size
	totalLength = length + sizeof(BlockHeader);
	totalLength = ceil(log2(totalLength));
	totalLength = pow(2, totalLength);
	if(totalLength <= basic_block_size){
		totalLength = basic_block_size;
	}
	if(totalLength > total_memory_size){
		cout<<"Length requested is too big to allocate, try allocating bigger memory size. "<<endl;
		return nullptr;
	}
	//finding the index of FreeList
	index = log2(totalLength/basic_block_size);

	//if FreeList contains a free block at the index
	if (FreeList[index].get_size() > 0){
		block = FreeList[index].get_head();
		block->free = false;
		//block->next = nullptr;
		FreeList[index].remove(block);
		allocBlock = (char*) block;
		allocBlock = allocBlock + sizeof(BlockHeader);
		//cout<<"Alloc Block: "<<(void*)allocBlock<<endl;
		return allocBlock;
	}
	//if FreeList does not contain a free block at the index
	else{
		uint currentSize = 0;
		BlockHeader* splitBlock = nullptr;
		//finding the index of next possible block
		while(FreeList[index].get_size() == 0){
			//find a free block to split;
			index = index + 1;
			if(index >= FreeListLength){
				cout<<"Not enough memory to allocate "<< length <<" bytes. Need to allocate more memory." << endl;
				return nullptr;
			}
			
		}
		currentSize = FreeList[index].get_head()->block_size;
		while(totalLength != currentSize){
			block = FreeList[index].get_head();
			splitBlock = this->split(block);
			block = splitBlock;
			currentSize = currentSize/2;
			index = index - 1;
		}
		FreeList[index].remove(block);
		block->free = false;
		block->next = nullptr;
		allocBlock = (char*) block;
		allocBlock = allocBlock + sizeof(BlockHeader);
		return allocBlock;
	}
}

void BuddyAllocator::free(void* a) {
	//this assumes that at a holds a BlockHeader*
	char* startAddr = nullptr;
	BlockHeader* buddy = nullptr; //hold the address of buddy
	BlockHeader* freeBlock = nullptr; //hold the address of the block being free
	uint freeSize = 0;
	uint index = 0;
	bool merge = true;
	bool areBuddies;
	if(a == nullptr){
		cout <<  "Cannot free this address is nullptr." << endl;
		return;
	}
	startAddr = (char*) a - sizeof(BlockHeader);
	freeBlock = (BlockHeader*) startAddr;
	if (freeBlock->free == 1){
		cout << "This block is already freed.";
		return;
	}
	freeSize = freeBlock->block_size;
	freeBlock->free = true;
	index = log2(freeSize/basic_block_size);
	FreeList[index].insert(freeBlock);


	while(merge){
		buddy = this->getbuddy(freeBlock);
		areBuddies = arebuddies(freeBlock, buddy);
		if(areBuddies && buddy->free == true){
			freeBlock = this->merge(freeBlock, buddy);
		}
		else{
			merge = false;
		}
	}



}

void BuddyAllocator::printlist (){
	cout << "Printing the Freelist in the format \"[index] (block size) : # of blocks\"" << endl;
	int64_t total_free_memory = 0;
	for (int i=0; i<FreeList.size(); i++){
		int blocksize = ((1<<i) * basic_block_size); // all blocks at this level are this size
		cout << "[" << i <<"] (" << blocksize << ") : ";  // block size at index should always be 2^i * bbs
		int count = 0;
		BlockHeader* b = FreeList [i].head;
		// go through the list from head to tail and count
			while (b){
				total_free_memory += blocksize;
				count ++;
				// block size at index should always be 2^i * bbs
				// checking to make sure that the block is not out of place

					if (b->block_size != blocksize){
						cerr << "ERROR:: Block is in a wrong list" << endl;
						exit (-1);
					}

					b = b->next;
			}
			cout << count << endl;
			cout << "Amount of available free memory: " << total_free_memory << " bytes" << endl;
	}
}

