/**
 * @file circular_buffer.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2023-09-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __CIRCULAR_BUFFER_H_
#define __CIRCULAR_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    volatile uint32_t produce_count;
    volatile uint32_t consume_count;
    uint16_t max_size;
    volatile char *data;
} Circular_Buffer ;


//*****************************************************************************
// Initializes a circular buffer.
//
//      1. Initialize the produce_count
//      2. Initialize the consume_count
//      3. Initialize the max_size
//      4. malloc enough memory from the heap for the data array
//
// Parameters
//    buffer  :   The address of the circular buffer.
//    max_size:   Number of entries in the circular buffer.
//*****************************************************************************
Circular_Buffer * circular_buffer_init(uint16_t max_size);

//*****************************************************************************
// Returns a circular buffer to the heap
//
// Parameters
//    buffer  :   The address of the circular buffer.
//*****************************************************************************
void circular_buffer_delete(Circular_Buffer * buffer);

//*****************************************************************************
// Adds a character to the circular buffer.
//
// Parameters
//    buffer  :   The address of the circular buffer.
//    c       :   Character to add.
//*******************************************************************************
bool circular_buffer_add(Circular_Buffer *buffer, char c);

//*****************************************************************************
// Removes the oldest character from the circular buffer.
//
// Parameters
//    buffer  :   The address of the circular buffer.
//*****************************************************************************
char circular_buffer_remove(Circular_Buffer *buffer);

//*****************************************************************************
// Returns true if the circular buffer is empty.  Returns false if it is not.
//
// Parameters
//    buffer  :   The address of the circular buffer.
//*****************************************************************************
bool circular_buffer_empty(Circular_Buffer *buffer);

//*****************************************************************************
// Returns true if the circular buffer is full.  Returns false if it is not.
//
// Parameters
//    buffer  :   The address of the circular buffer.
//*****************************************************************************
bool circular_buffer_full(Circular_Buffer *buffer);

#endif /* circular_buffer_H_ */
