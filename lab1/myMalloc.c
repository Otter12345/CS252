#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "myMalloc.h"
#include "printing.h"

/* Due to the way assert() prints error messges we use out own assert function
 * for deteminism when testing assertions
 */
#ifdef TEST_ASSERT
  inline static void assert(int e) {
    if (!e) {
      fprintf(stderr, "Assertion Failed!\n");
      exit(1);
    }
  }
#else 
  #include <assert.h>
#endif

/*
 * Mutex to ensure thread safety for the freelist
 */
static pthread_mutex_t mutex;

/*
 * Array of sentinel nodes for the freelists
 */
header freelistSentinels[N_LISTS];

/*
 * Pointer to the second fencepost in the most recently allocated chunk from
 * the OS. Used for coalescing chunks
 */
header * lastFencePost;

/*
 * Pointer to maintian the base of the heap to allow printing based on the
 * distance from the base of the heap
 */ 
void * base;

/*
 * List of chunks allocated by  the OS for printing boundary tags
 */
header * osChunkList [MAX_OS_CHUNKS];
size_t numOsChunks = 0;

/*
 * direct the compiler to run the init function before running main
 * this allows initialization of required globals
 */
static void init (void) __attribute__ ((constructor));

// Helper functions for manipulating pointers to headers
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off);
static inline header * get_left_header(header * h);
static inline header * ptr_to_header(void * p);

// Helper functions for allocating more memory from the OS
static inline void initialize_fencepost(header * fp, size_t left_size);
static inline void insert_os_chunk(header * hdr);
static inline void insert_fenceposts(void * raw_mem, size_t size);
static header * allocate_chunk(size_t size);

// Helper functions for freeing a block
static inline void deallocate_object(void * p);
static inline void updateChunkPosition(size_t size, header * chunkPosition);
static inline void insertFreelistTail(size_t size, header * chunkPosition);

// Helper functions for allocating a block
static inline header * allocate_object(size_t raw_size);

// Helper functions for verifying that the data structures are structurally 
// valid
static inline header * detect_cycles();
static inline header * verify_pointers();
static inline bool verify_freelist();
static inline header * verify_chunk(header * chunk);
static inline bool verify_tags();

// Library initialization
static void init();

/**
 * @brief Helper function to retrieve a header pointer from a pointer and an 
 *        offset
 *
 * @param ptr base pointer
 * @param off number of bytes from base pointer where header is located
 *
 * @return a pointer to a header offset bytes from pointer
 */
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off) {
  return (header *)((char *) ptr + off);
}

/**
 * @brief Helper function to get the header to the right of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 */
header * get_right_header(header * h) {
  return get_header_from_offset(h, h->size);
}

/**
 * @brief Helper function to get the header to the left of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 */
inline static header * get_left_header(header * h) {
  return get_header_from_offset(h, -h->left_size);
}

/**
 * @brief Fenceposts are marked as always allocated and may need to have
 * a left object size to ensure coalescing happens properly
 *
 * @param fp a pointer to the header being used as a fencepost
 * @param left_size the size of the object to the left of the fencepost
 */
inline static void initialize_fencepost(header * fp, size_t left_size) {
  fp->allocated = FENCEPOST;
  fp->size = ALLOC_HEADER_SIZE;
  fp->left_size = left_size;
}

/**
 * @brief Helper function to maintain list of chunks from the OS for debugging
 *
 * @param hdr the first fencepost in the chunk allocated by the OS
 */
inline static void insert_os_chunk(header * hdr) {
  if (numOsChunks < MAX_OS_CHUNKS) {
    osChunkList[numOsChunks++] = hdr;
  }
}

/**
 * @brief given a chunk of memory insert fenceposts at the left and 
 * right boundaries of the block to prevent coalescing outside of the
 * block
 *
 * @param raw_mem a void pointer to the memory chunk to initialize
 * @param size the size of the allocated chunk
 */
inline static void insert_fenceposts(void * raw_mem, size_t size) {
  // Convert to char * before performing operations
  char * mem = (char *) raw_mem;

  // Insert a fencepost at the left edge of the block
  header * leftFencePost = (header *) mem;
  initialize_fencepost(leftFencePost, ALLOC_HEADER_SIZE);

  // Insert a fencepost at the right edge of the block
  header * rightFencePost = get_header_from_offset(mem, size - ALLOC_HEADER_SIZE);
  initialize_fencepost(rightFencePost, size - 2 * ALLOC_HEADER_SIZE);
}

/**
 * @brief Allocate another chunk from the OS and prepare to insert it
 * into the free list
 *
 * @param size The size to allocate from the OS
 *
 * @return A pointer to the allocable block in the chunk (just after the 
 * first fencpost)
 */
static header * allocate_chunk(size_t size) {
  void * mem = sbrk(size);
  
  insert_fenceposts(mem, size);
  header * hdr = (header *) ((char *)mem + ALLOC_HEADER_SIZE);
  hdr->allocated = UNALLOCATED;
  hdr->size = size - 2 * ALLOC_HEADER_SIZE;
  hdr->left_size = ALLOC_HEADER_SIZE;
  return hdr;
}

/**
 * @brief Helper allocate an object given a raw request size from the user
 *
 * @param raw_size number of bytes the user needs
 *
 * @return A block satisfying the user's request
 */
static inline header * allocate_object(size_t raw_size) {
  // TODO implement allocation
  //calculate the required block size
  if(raw_size == 0){
	return NULL;
  }
 else{
   raw_size +=ALLOC_HEADER_SIZE;

    if(raw_size % 8!=0){
	raw_size = (raw_size / 8 + 1)*8; 
   }


	if(raw_size < sizeof(header))
		raw_size=sizeof(header);

  // find the appropriate free list to allocate
  size_t index;

  index = (raw_size-ALLOC_HEADER_SIZE) / 8 - 1;
  if(index > N_LISTS-1){
  	  index = N_LISTS-1;
  }
  
  while(freelistSentinels[index].next == &freelistSentinels[index] && index<(N_LISTS-1)){
	index++;
  } 

	if(index<(N_LISTS-1)){
		header * block = freelistSentinels[index].next;
		
		if(raw_size == block->size){
			block->allocated = ALLOCATED;
			block->next->prev = block->prev;
			block->prev->next = block->next;
			return (header*)(&block->data[0]);
		} else {
			header * newH = get_header_from_offset(block, (block->size - raw_size));
			block->size -= raw_size;
			newH->size=raw_size;
			newH->allocated=ALLOCATED;
			newH->left_size=block->size;
			header *rightHeader = get_right_header(newH);
			rightHeader->left_size = raw_size; 

			if(block->size < (index*8 + 1) || block->size > ((index+1)*8)){
				//if position updated
				updateChunkPosition(block->size, block);
			}
					
			return (header*)(&newH->data[0]);
		}
		
	}

	//LAST element in freelist
	else {
	  header * block = freelistSentinels[index].next;

	//check all blocks
	  while (block->size<raw_size && block != &freelistSentinels[N_LISTS-1]){
		block=block->next;
	  }

	 //create more chunk
	  if(block == &freelistSentinels[N_LISTS-1] ) {
		//create another chunk
		header * chunk = allocate_chunk(ARENA_SIZE);
		if (numOsChunks < MAX_OS_CHUNKS) {
    			osChunkList[numOsChunks++] = (header *)((char*)chunk - ALLOC_HEADER_SIZE);
  		} else {
			fprintf(stderr, "Max Chunks limit reached.\n");			
			return NULL;		
		}

		//check if needs coalescing
		header * prevChunk = lastFencePost;
		lastFencePost = get_header_from_offset(chunk, chunk->size);

		if(osChunkList[numOsChunks-1] - prevChunk == 0){
		  //coalescing
			//check if left block is UNALLOCATED
			header * leftBlock = get_left_header(prevChunk);
			header * newH;
			
			if(leftBlock->allocated == UNALLOCATED){
				leftBlock->prev->next = leftBlock->next;
				leftBlock->next->prev = leftBlock->prev;
				leftBlock->size += 2*ALLOC_HEADER_SIZE + chunk->size; 
				newH = leftBlock;
				
			} else{
				newH = get_header_from_offset(chunk, -2*ALLOC_HEADER_SIZE);
				newH->size +=ALLOC_HEADER_SIZE + chunk->size;
				newH->allocated=UNALLOCATED;
			}
				
			    	osChunkList[numOsChunks-1] = NULL;
				//insert the new block into freelist
				freelistSentinels[N_LISTS-1].prev->next = newH;
				newH->prev = freelistSentinels[N_LISTS-1].prev;
				freelistSentinels[N_LISTS-1].prev = newH;
				newH->next = &freelistSentinels[N_LISTS-1];

				lastFencePost->left_size = newH->size;
			//	chunk = NULL;
				numOsChunks--;
			
		} 
		else {
		//no coalesing
		freelistSentinels[N_LISTS-1].prev->next = chunk;
		chunk->prev = freelistSentinels[N_LISTS-1].prev;
		freelistSentinels[N_LISTS-1].prev = chunk;
		chunk->next = &freelistSentinels[N_LISTS-1];
		}
	
        return allocate_object(raw_size - ALLOC_HEADER_SIZE);
		
	  }
	  
	  //no new chunk needed
	  else{
		if(raw_size == block->size){
			block->allocated=ALLOCATED;
			block->next->prev = block->prev;
			block->prev->next = block->next;
			return (header*)(&block->data[0]);
		} else {
	  //create new header for allocated block
			header * newH = get_header_from_offset(block, (block->size - raw_size));
			block->size -= raw_size;
			newH->size=raw_size;
			newH->left_size=block->size;
			header *rightHeader = get_right_header(newH);
			rightHeader->left_size = raw_size; 
			newH->allocated=ALLOCATED;

			if(block->size < ((N_LISTS-1)*8 + 1))
				updateChunkPosition(block->size, block);
			
			return (header*)(&newH->data[0]);
			}	
	  }
	}
 }
 //Hello 
	return NULL;

}

/**
 * @brief Helper to get the header from a pointer allocated with malloc
 *
 * @param p pointer to the data region of the block
 *
 * @return A pointer to the header of the block
 */
static inline header * ptr_to_header(void * p) {
  return (header *)((char *) p - ALLOC_HEADER_SIZE); //sizeof(header));
}

/**
 * @brief Helper to manage deallocation of a pointer returned by the user
 *
 * @param p The pointer returned to the user by a call to malloc
 */
static inline void deallocate_object(void * p) {
  // TODO implement deallocation
  header * oldHeader = ptr_to_header(p);

  if (p == NULL){
  	  return;
  }

  if(oldHeader->allocated == UNALLOCATED || oldHeader == NULL){
	  fprintf(stderr,"Double Free Detected\n");
  	  assert(false);
  }

  //neighbour header
  header * left_H = get_left_header(oldHeader);
  header * right_H = get_right_header(oldHeader);

  //check if neighbour header is unallocated
  size_t newSize, index, oldIndex;
  oldHeader->allocated = UNALLOCATED;

  //L & R UNALLOCATED
  if (left_H->allocated == UNALLOCATED && right_H->allocated == UNALLOCATED){
  //cut right side of block out from freelist
	right_H->prev->next = right_H->next;
	right_H->next->prev = right_H->prev;

	//calculate new size
	 newSize = left_H->size + right_H->size + oldHeader->size;
	 oldIndex = (left_H->size-ALLOC_HEADER_SIZE)/8 -1;
	// left_H->size = newSize;

	// oldHeader->allocated = UNALLOCATED;
	// oldHeader = left_H;
	 //right_H = left_H;
	 if(oldIndex!=(newSize-ALLOC_HEADER_SIZE)/8 -1)
		 updateChunkPosition(newSize,left_H);
	
  } 


  // L UNALLOCATED
  else if(left_H->allocated == UNALLOCATED){
	newSize = oldHeader->size + left_H->size;
	//oldHeader->allocated = UNALLOCATED;
	//oldHeader = left_H;
	oldIndex = (left_H->size- ALLOC_HEADER_SIZE)/8 - 1;
	index = (newSize - ALLOC_HEADER_SIZE)/8 - 1;

	if(index != oldIndex){
		updateChunkPosition(newSize, left_H);
  }
	}

  // R UNALLOCATED
  else if(right_H->allocated == UNALLOCATED){
	newSize = oldHeader->size + right_H->size;
	index = (newSize - ALLOC_HEADER_SIZE)/8 - 1;
	oldIndex = (right_H->size- ALLOC_HEADER_SIZE)/8 - 1;
	
	if(index != oldIndex){
		if( index > N_LISTS-1){
			index = N_LISTS-1;
		}
		
		//cut the block out
		right_H->prev->next = right_H->next;
		right_H->next->prev = right_H->prev;
		//assign to new position in freelist
		freelistSentinels[index].next->prev = oldHeader;
		oldHeader->prev = &freelistSentinels[index];
		oldHeader->next = freelistSentinels[index].next;
		freelistSentinels[index].next = oldHeader;
	}
		 //update size and left size
		 oldHeader->size = newSize;
		 header * nextR = get_right_header(right_H);
		 nextR->left_size = newSize;
		 oldHeader->left_size = left_H->size;
		 oldHeader->allocated = UNALLOCATED;
		// right_H = oldHeader;
  }

  else {
	index = (oldHeader->size-ALLOC_HEADER_SIZE)/8 - 1;

	if(index > (N_LISTS-1)){
		index = N_LISTS-1;
	}

	freelistSentinels[index].next->prev = oldHeader;
	oldHeader->next = freelistSentinels[index].next;
	freelistSentinels[index].next = oldHeader;
	oldHeader->prev = &freelistSentinels[index];
	oldHeader->allocated = UNALLOCATED;
	
  }
  
}

/**
 * @brief Helper to calculate updated block position in the freelist
 *
 * @return void
 */

static inline void updateChunkPosition(size_t size, header * chunkPosition){
	size_t newIndex;

	//calculate new index
	newIndex = (size-ALLOC_HEADER_SIZE)/8 - 1;

	if(newIndex>(N_LISTS-1)){
		insertFreelistTail(size, chunkPosition);
	}
	else{
			
	//cut the block out
	chunkPosition->prev->next = chunkPosition->next;
	chunkPosition->next->prev = chunkPosition->prev;
	
	//assign to new position in freelist
	freelistSentinels[newIndex].next->prev = chunkPosition;
	chunkPosition->prev = &freelistSentinels[newIndex];
	chunkPosition->next = freelistSentinels[newIndex].next;
	freelistSentinels[newIndex].next = chunkPosition;
	
	//update left_size
	chunkPosition->size = size;
	header * rightHeader = get_right_header(chunkPosition);
	rightHeader->left_size = size;

	}
}

/**
 * @brief Helper to update block position in the freelist
 *
 * @return void
 */

static inline void insertFreelistTail(size_t size, header * chunkPosition){
	size_t index = N_LISTS-1;
		
	//cut the block out
	chunkPosition->prev->next = chunkPosition->next;
	chunkPosition->next->prev = chunkPosition->prev;
	
	//assign to new position in freelist
	freelistSentinels[index].prev->next = chunkPosition;
	chunkPosition->prev = freelistSentinels[index].prev;
	chunkPosition->next = &freelistSentinels[index];
	freelistSentinels[index].prev = chunkPosition;
	
	//update left_size
	chunkPosition->size = size;
	header * rightHeader = get_right_header(chunkPosition);
	rightHeader->left_size = size;

}

/**
 * @brief Helper to detect cycles in the free list
 * https://en.wikipedia.org/wiki/Cycle_detection#Floyd's_Tortoise_and_Hare
 *
 * @return One of the nodes in the cycle or NULL if no cycle is present
 */
static inline header * detect_cycles() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * slow = freelist->next, * fast = freelist->next->next; 
         fast != freelist; 
         slow = slow->next, fast = fast->next->next) {
      if (slow == fast) {
        return slow;
      }
    }
  }
  return NULL;
}

/**
 * @brief Helper to verify that there are no unlinked previous or next pointers
 *        in the free list
 *
 * @return A node whose previous and next pointers are incorrect or NULL if no
 *         such node exists
 */
static inline header * verify_pointers() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * cur = freelist->next; cur != freelist; cur = cur->next) {
      if (cur->next->prev != cur || cur->prev->next != cur) {
        return cur;
      }
    }
  }
  return NULL;
}

/**
 * @brief Verify the structure of the free list is correct by checkin for 
 *        cycles and misdirected pointers
 *
 * @return true if the list is valid
 */
static inline bool verify_freelist() {
  header * cycle = detect_cycles();
  if (cycle != NULL) {
    fprintf(stderr, "Cycle Detected\n");
    print_sublist(print_object, cycle->next, cycle);
    return false;
  }

  header * invalid = verify_pointers();
  if (invalid != NULL) {
    fprintf(stderr, "Invalid pointers\n");
    print_object(invalid);
    return false;
  }

  return true;
}

/**
 * @brief Helper to verify that the sizes in a chunk from the OS are correct
 *        and that allocated node's canary values are correct
 *
 * @param chunk AREA_SIZE chunk allocated from the OS
 *
 * @return a pointer to an invalid header or NULL if all header's are valid
 */
static inline header * verify_chunk(header * chunk) {
  if (chunk->allocated != FENCEPOST) {
    fprintf(stderr, "Invalid fencepost\n");
    print_object(chunk);
    return chunk;
  }

  for (; chunk->allocated != FENCEPOST; chunk = get_right_header(chunk)) {
    if (chunk->size  != get_right_header(chunk)->left_size) {
      fprintf(stderr, "Invalid sizes\n");
      print_object(chunk);
      return chunk;
    }
  }

  return NULL;
}

/**
 * @brief For each chunk allocated by the OS verify that the boundary tags
 *        are consistent
 *
 * @return true if the boundary tags are valid
 */
static inline bool verify_tags() {
  for (size_t i = 0; i < numOsChunks; i++) {
    header * invalid = verify_chunk(osChunkList[i]);
    if (invalid != NULL) {
      return invalid;
    }
  }

  return NULL;
}

/**
 * @brief Initialize mutex lock and prepare an initial chunk of memory for allocation
 */
static void init() {
  // Initialize mutex for thread safety
  pthread_mutex_init(&mutex, NULL);

#ifdef DEBUG
  // Manually set printf buffer so it won't call malloc when debugging the allocator
  setvbuf(stdout, NULL, _IONBF, 0);
#endif // DEBUG

  // Allocate the first chunk from the OS
  header * block = allocate_chunk(ARENA_SIZE);

  header * prevFencePost = get_header_from_offset(block, -ALLOC_HEADER_SIZE);
  insert_os_chunk(prevFencePost);

  lastFencePost = get_header_from_offset(block, block->size);

  // Set the base pointer to the beginning of the first fencepost in the first
  // chunk from the OS
  base = ((char *) block) - ALLOC_HEADER_SIZE; //sizeof(header);

  // Initialize freelist sentinels
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    freelist->next = freelist;
    freelist->prev = freelist;
  }

  // Insert first chunk into the free list
  header * freelist = &freelistSentinels[N_LISTS - 1];
  freelist->next = block;
  freelist->prev = block;
  block->next = freelist;
  block->prev = freelist;
}

/* 
 * External interface
 */
void * my_malloc(size_t size) {
  pthread_mutex_lock(&mutex);
  header * hdr = allocate_object(size); 
  pthread_mutex_unlock(&mutex);
  return hdr;
}

void * my_calloc(size_t nmemb, size_t size) {
  return memset(my_malloc(size * nmemb), 0, size * nmemb);
}

void * my_realloc(void * ptr, size_t size) {
  void * mem = my_malloc(size);
  memcpy(mem, ptr, size);
  my_free(ptr);
  return mem; 
}

void my_free(void * p) {
  pthread_mutex_lock(&mutex);
  deallocate_object(p);
  pthread_mutex_unlock(&mutex);
}

bool verify() {
  return verify_freelist() && verify_tags();
}
