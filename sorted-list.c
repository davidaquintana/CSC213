/*

David Quintana
Assignment 2
Sept. 11, 2023

Acknowledgements:
CSC161: scheme-lists.c, listQueue.c, namelist.c
  -C functions that I made during CSC161(Spring 22)
  -I used these old labs/assignments to see how my struct should be used
  -I used these old labs/assignments for examples of count, print, insert and initialize functions
CSC213: stack-list.c, stack-array.c
  -C functions demos made in class
  -I used these demos to see how my struct should be used
  -I used these demos for examples of initialize and destroy functions

Mentor:
  -Fixed Code:
    int_node_t* current = lst->head;
    while(current->next != NULL && current->next->data < value)
    {
      current = current->next;
    }
    newNode->next = current->next;
    current->next = newNode;

  -Previous Code:
    int_node_t* current = lst->head;
    while(current->next != NULL && current->next->data < value)
    {
      current = current->next;
      newNode->next = current->next;
      current->next = newNode;
    }

  ChatGPT:
    - I was having a memory leak within my sorted_list_insert function, so I asked what was wrong with my code and GPT stated that I was not
      leaving my function that checks if lst->head == NULL or if the data is smaller than value. After this knowledge I inserted a return
    
*/
#include "sorted-list.h"
#include <stdio.h>
#include <stdlib.h>


/**
 * Initialize a sorted list.
 *
 * \param lst This is a pointer to space that should be initialized as a sorted list. The caller of
 * this function retains ownership of the memory that lst points to (meaning the caller must free it
 * if the pointer was returned from malloc)
 */

void sorted_list_init(sorted_list_t* lst) {
  //initializes the sorted-list by setting lst->head to NULL
  lst->head = NULL;
}

/**
 * Destroy a sorted list. Free any memory allocated to store the list, but not the list itself.
 *
 * \param lst This is a pointer to the space that holds the list being destroyred. This function
 * should free any memory used to represent the list, but the caller retains ownership of the lst
 * pointer itself.
 */
void sorted_list_destroy(sorted_list_t* lst) {
  //makes sure that there is something in lst
  if (lst->head != NULL) {
    //runs through lst until lst->head is null (lst is empty)
    while (lst->head != NULL) {
      //lst->head will be the node deleted
      int_node_t* deletedNode = lst->head;
      //lst->head is now equal to the next element in lst
      lst->head = lst->head->next;      
      //frees memory block of deletedNode
      free(deletedNode);
    }
  }
}

/**
 * Add an element to a sorted list, maintaining the lowest-to-highest sorted order.
 *
 * \param lst   The list that is being inserted to
 * \param value The value being inserted
 */
void sorted_list_insert(sorted_list_t* lst, int value) {
  //creates a newNode that will be inserted to lst
  int_node_t* newNode = malloc(sizeof(int_node_t));
  //data in newNode is now the value inputed
  newNode->data = value;

  //if lst is empty or lst->head->data is less than or equal to value
  if(lst->head == NULL || lst->head->data >= value)
  {
    //next node in newNode is now the head of lst
    newNode->next = lst->head;
    //lst->head is now the value inserted, everything next is the next
    lst->head = newNode;
    return;
  }

  //current node to reitirate through the node
  int_node_t* current = lst->head;

  //while lst has a next and the data in next is smaller than value
  while(current->next != NULL && current->next->data < value)
  {
    //new current is the next
    current = current->next;
  }
  //newNode->next is now current->next
  newNode->next = current->next;
  //current->next is now newNode
  current->next = newNode;
}

/**
 * Count how many times a value appears in a sorted list.
 *
 * \param lst The list being searched
 * \param value The value being counted
 * \returns the number of times value appears in lst
 */
size_t sorted_list_count(sorted_list_t* lst, int value) {
  //count variable
  int count = 0;
  //current node to reitirate through the node
  int_node_t* current = lst->head;

  //while current is not empty and current->data is smaller or equal to value
  while (current != NULL && current->data <= value) {
    //if data is the value
    if (current->data == value) {
      //increments count
      count++;
    }
      //if data is not in current, move to current->next
      current = current->next;

    //if current is not empty and data in current is larger than value
    if(current != NULL && current->data > value)
    {
      //after you move beyond where the value is, the while loop is broken bc the value should not be anywhere else in the lst
      break;
    }
  }
  //returns the counter variable
  return count;
}

/**
 * Print the values in a sorted list in ascending order, separated by spaces and followed by a
 * newline.
 *
 * \param lst The list to print
 */
void sorted_list_print(sorted_list_t* lst) {
  // TODO: implement me

  //current node to reitirate through the node
  int_node_t *current = lst->head;

  //while current is not empty
    while (current != NULL) {
      
      //prints out current->data
      printf("%d ", current->data);

      //moves to next element of lst
      current = current->next;
    }
    //prints newline
    printf("\n");
}
