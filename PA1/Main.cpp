#include "Ackerman.h"
#include "BuddyAllocator.h"
#include "math.h"
#include <getopt.h>

void easytest(BuddyAllocator* ba){
  // be creative here
  // know what to expect after every allocation/deallocation cycle

  // here are a few examples
  ba->printlist();
  // allocating a byte
  char * mem = (char *) ba->alloc (1);
  // now print again, how should the list look now
  ba->printlist ();

  ba->free (mem); // give back the memory you just allocated
  ba->printlist(); // shouldn't the list now look like as in the beginning

}

int main(int argc, char ** argv) {

  int basic_block_size = 128, memory_length = 512 * 1024;
  int c;

  while((c = getopt(argc, argv, "b: s:")) != -1){
  	switch(c){
		case 'b':
			basic_block_size = atoi(optarg);
			break;
		case 's':
			memory_length = atoi(optarg);
			break;
	}

  }
  if(basic_block_size < 128){
	basic_block_size = 128;
  }
  cout << "Basic Block: " << basic_block_size;
  cout << "Memory Block: " << memory_length;
  // create memory manager
  BuddyAllocator * allocator = new BuddyAllocator(basic_block_size, memory_length);

  // the following won't print anything until you start using FreeList and replace the "new" with your own implementation
  easytest (allocator);

  
  // stress-test the memory manager, do this only after you are done with small test cases
  Ackerman* am = new Ackerman ();
  am->test(allocator); // this is the full-fledged test. 
  
  // destroy memory manager
  delete allocator;



//testing
	/*
	int basic_block_size = 129, memory_length = 512 * 1024;
	BuddyAllocator *b = new BuddyAllocator(basic_block_size, memory_length);
	cout<<b<<endl;
	char* mem = (char *) b->alloc (1);
	//cout << (void*) mem <<endl;
	//b->alloc (1);
	b->printlist();
	//mem = b->alloc (1);
	//b->printlist();
	b->free(mem);
	b->printlist();
	cout << b->FreeList[11].get_head()->free;
	*/




}
