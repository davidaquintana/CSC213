#define _GNU_SOURCE
#include <openssl/md5.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_USERNAME_LENGTH 64
#define PASSWORD_LENGTH 6

/************************* Part A *************************/

/**
 * Find a six character lower-case alphabetic password that hashes
 * to the given hash value. Complete this function for part A of the lab.
 *
 * \param input_hash  An array of MD5_DIGEST_LENGTH bytes that holds the hash of a password
 * \param output      A pointer to memory with space for a six character password + '\0'
 * \returns           0 if the password was cracked. -1 otherwise.
 */
int crack_single_password(uint8_t* input_hash, char* output) {
  // Take our candidate password and hash it using MD5
  char candidate_passwd[] = "psswrd";  //< This variable holds the password we are trying

  while (1) {  // checks all possible passwords
    uint8_t
        candidate_hash[MD5_DIGEST_LENGTH];  //< This will hold the hash of the candidate password
    MD5((unsigned char*)candidate_passwd, strlen(candidate_passwd),
        candidate_hash);  //< Do the hash

    // Now check if the hash of the candidate password matches the input hash
    if (memcmp(input_hash, candidate_hash, MD5_DIGEST_LENGTH) == 0) {
      // Match! Copy the password to the output and return 0 (success)
      strncpy(output, candidate_passwd, PASSWORD_LENGTH + 1);
      return 0;
    } else {
      // No match. Return -1 (failure)
      return -1;
    }

    // if the first character is not z, go on to the next character
    if (candidate_passwd[0] != 'z') {
      candidate_passwd[0] += 1;  // incrementation to next character
    } else {
      for (int i = 0; i < PASSWORD_LENGTH; i++) {  // make sure to pre - increment
        if (candidate_passwd[i] != 'z') {
          candidate_passwd[i] += 1;  // next charavvter
          break;
        } else {
          candidate_passwd[i] = 'a';  // set to the first possible character
        }
      }
    }
  }
}

/********************* Parts B & C ************************/

/**
 * This struct is the root of the data structure that will hold users and hashed passwords.
 * This could be any type of data structure you choose: list, array, tree, hash table, etc.
 * Implement this data structure for part B of the lab.
 */

// struct that holds elements (username, pwd, and next element)
typedef struct element {
  char* username;
  uint8_t passwd[MD5_DIGEST_LENGTH];
  struct element* nextElement;
} element_t;

// struct to set pwd(first element and pwd length)
typedef struct password_set {
  element_t* firstElement;
  int leng;
} password_set_t;

// struct that holds the pwd threads
typedef struct thread {
  password_set_t* passwdSet;
  char init_passwd[7];
  char fin_passwd[7];
  int numCracked;
} thread_t;

/**
 * Initialize a password set.
 * Complete this implementation for part B of the lab.
 *
 * \param passwords  A pointer to allocated memory that will hold a password set
 */
void init_password_set(password_set_t* passwords) {
  // TODO: Initialize any fields you add to your password set structure
  passwords->firstElement = NULL;  // sets the elements to null, initilazing the pwd's elements
  passwords->leng = 0;             // initializes the length to 0
}

/**
 * Add a password to a password set
 * Complete this implementation for part B of the lab.
 *
 * \param passwords   A pointer to a password set initialized with the function above.
 * \param username    The name of the user being added. The memory that holds this string's
 *                    characters will be reused, so if you keep a copy you must duplicate the
 *                    string. I recommend calling strdup().
 * \param password_hash   An array of MD5_DIGEST_LENGTH bytes that holds the hash of this user's
 *                        password. The memory that holds this array will be reused, so you must
 *                        make a copy of this value if you retain it in your data structure.
 */
void add_password(password_set_t* passwords, char* username, uint8_t* password_hash) {
  // TODO: Add the provided user and password hash to your set of passwords
  element_t* newElement =
      (element_t*)malloc(sizeof(element_t));  // malloc to create a newElement node of element_t

  newElement->username =
      (char*)malloc(sizeof(char) * (strlen(username) + 1));  // malloc space for username element

  strcpy(newElement->username, username);  // copies data over

  memcpy(newElement->passwd, password_hash, MD5_DIGEST_LENGTH);  // copies data over

  element_t* prevElement =
      passwords->firstElement;  // sets a prevElement to what was the first element
  newElement->nextElement =
      prevElement;  // the next element of the new element would be the previous element
  passwords->firstElement = newElement;  // the first element is now the new element

  passwords->leng++;  // increment the pwd length
}

// frees all the  memory allocated for the given password set
void destroy_password(password_set_t* passwords) {
  // set a current value to loop through
  element_t* curElement = passwords->firstElement;
  // loops through all elements
  while (curElement != NULL) {
    // sets a temp element to the current element
    element_t* tempElement = curElement;
    // current element is now the next element
    curElement = curElement->nextElement;
    // free the temporary element
    free(tempElement);
  }

  // resets pwd length
  passwords->leng = 0;
}

// removes a single password from a list of passwords
void delete_item(password_set_t* passwords, element_t* elementPass) {
  // checks to see if they are the samwe
  if (strcmp(passwords->firstElement->username, elementPass->username) == 0) {
    // if the username is the same, set the pwd's elements
    passwords->firstElement = elementPass->nextElement;
    // frees the pwd elemement from the set
    free(elementPass);
  } else {  // if usernames are not the same
    // create a prev element to represent the the first element
    element_t* prevElement = passwords->firstElement;
    // loop through the whole set to find the element
    for (element_t* curElement = prevElement->nextElement; curElement != NULL;
         curElement = curElement->nextElement) {
      // once element is found
      if (strcmp(curElement->username, elementPass->username) == 0) {
        // set the element to be the next element
        prevElement->nextElement = curElement->nextElement;
        // free current element
        free(curElement);
        break;
      }
      // set the previous element to the current
      prevElement = curElement;
    }
  }
  passwords->leng--;
}

// cracks all of the passwords in the set. prints the username and cracked password for each user.
int pswd_helper(password_set_t* passwords, char* init_passwd, char* fin_passwd) {
  char candidate_passwd[7];
  strcpy(candidate_passwd, init_passwd);

  int totalCracked = 0;

  // all possible passwords are attempted while the set is not empty
  while (passwords->firstElement != NULL) {
    // holds the candidate password
    uint8_t candidate_hash[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)candidate_passwd, strlen(candidate_passwd), candidate_hash);

    // checks to see if there is a match with candidate password from data set
    element_t* curElement = passwords->firstElement;
    while (curElement != NULL) {
      // checks to see if pwd matches input
      if (memcmp(curElement->passwd, candidate_hash, MD5_DIGEST_LENGTH) == 0) {
        // prints pswd and updates total cracked
        printf("%s %s\n", curElement->username, candidate_passwd);
        ++totalCracked;

        // increments to the next password
        element_t* nextPass = curElement->nextElement;

        // deletes the current item
        delete_item(passwords, curElement);
        curElement = nextPass;
      } else {
        // tries the next element
        curElement = curElement->nextElement;
      }
    }

    // checks to see if we ran out of possible passwords
    if (strcmp(candidate_passwd, fin_passwd) == 0) {
      return totalCracked;
    }

    // checks to see if the first letter is z, if not go to next char
    if (candidate_passwd[0] != 'z') {
      candidate_passwd[0] += 1;
    } else {
      // used to increment to the next available letter
      for (int k = 0; k < PASSWORD_LENGTH; k++) {
        if (candidate_passwd[k] != 'z') {
          candidate_passwd[k] += 1;
          break;
        } else {
          candidate_passwd[k] = 'a';
        }
      }
    }
  }
  return totalCracked;  // returns the number of passwords that were cracked in the list
}

// uses the helper function using threads
void* thread_func(void* p) {
  thread_t* arg = (thread_t*)p;

  // using this password set, crack all the passwords
  arg->numCracked = pswd_helper(arg->passwdSet, arg->init_passwd, arg->fin_passwd);
  return NULL;
}

/**
 * Crack all of the passwords in a set of passwords. The function should print the username
 * and cracked password for each user listed in passwords, separated by a space character.
 * Complete this implementation for part B of the lab.
 *
 * \returns The number of passwords cracked in the list
 */
int crack_password_list(password_set_t* passwords) {
  int totalCracked = 0;

  // array of threads
  pthread_t threads[4];

  // array of corresponding thread arguemnts
  thread_t args[4];

  // variable to store the number of threads
  int numThreads;

  // finds the number of threads
  if (passwords->leng >= 4) {
    numThreads = 4;
  } else {
    numThreads = passwords->leng;
  }

  // divide the chunks of candiate passwords for threads
  char passwd1[] = "aaaaaa";
  char passwd2[] = "bbbbbb";
  char passwd3[] = "cccccc";
  char passwd4[] = "dddddd";
  char final[] = "zzzzzz";

  // setting start and end points for each thread based on the number of threads
  if (numThreads == 4) {
    strcpy(args[0].init_passwd, passwd1);
    strcpy(args[0].fin_passwd, passwd2);
    passwd2[0]++;

    strcpy(args[1].init_passwd, passwd2);
    strcpy(args[1].fin_passwd, passwd3);
    passwd3[0]++;

    strcpy(args[2].init_passwd, passwd3);
    strcpy(args[2].fin_passwd, passwd4);
    passwd4[0]++;

    strcpy(args[3].init_passwd, passwd4);
    strcpy(args[3].fin_passwd, final);
    // whith 3 one thread gets more work than the other two
  } else if (numThreads == 3) {
    strcpy(args[0].init_passwd, passwd1);
    strcpy(args[0].fin_passwd, passwd3);
    passwd3[0]++;

    strcpy(args[1].init_passwd, passwd3);
    strcpy(args[1].fin_passwd, passwd4);
    passwd4[0]++;

    strcpy(args[2].init_passwd, passwd4);
    strcpy(args[2].fin_passwd, final);
    // the work is evenly split amongst the 2 threads
  } else if (numThreads == 2) {
    strcpy(args[0].init_passwd, passwd1);
    strcpy(args[0].fin_passwd, passwd3);
    passwd3[0]++;

    strcpy(args[1].init_passwd, passwd3);
    strcpy(args[1].fin_passwd, final);
    // all of the work is placed on a single thread
  } else if (numThreads == 1) {
    strcpy(args[0].init_passwd, passwd1);
    strcpy(args[0].fin_passwd, final);
  }

  // create the threads
  for (int i = 0; i < numThreads; i++) {
    args[i].passwdSet = passwords;
    if (pthread_create(&threads[i], NULL, thread_func, &args[i])) {
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  // run the threads
  for (int i = 0; i < numThreads; i++) {
    if (pthread_join(threads[i], NULL)) {
      perror("pthread_join failed");
      exit(EXIT_FAILURE);
    }
  }

  // compute the total number of cracked passwords
  for (int i = 0; i < numThreads; i++) {
    totalCracked += args[i].numCracked;
  }

  // destroy the password before returning the number of cracked passwords
  destroy_password(passwords);
  return totalCracked;
}

/******************** Provided Code ***********************/

/**
 * Convert a string representation of an MD5 hash to a sequence
 * of bytes. The input md5_string must be 32 characters long, and
 * the output buffer bytes must have room for MD5_DIGEST_LENGTH
 * bytes.
 *
 * \param md5_string  The md5 string representation
 * \param bytes       The destination buffer for the converted md5 hash
 * \returns           0 on success, -1 otherwise
 */
int md5_string_to_bytes(const char* md5_string, uint8_t* bytes) {
  // Check for a valid MD5 string
  if (strlen(md5_string) != 2 * MD5_DIGEST_LENGTH) return -1;

  // Start our "cursor" at the start of the string
  const char* pos = md5_string;

  // Loop until we've read enough bytes
  for (size_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
    // Read one byte (two characters)
    int rc = sscanf(pos, "%2hhx", &bytes[i]);
    if (rc != 1) return -1;

    // Move the "cursor" to the next hexadecimal byte
    pos += 2;
  }

  return 0;
}

void print_usage(const char* exec_name) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s single <MD5 hash>\n", exec_name);
  fprintf(stderr, "  %s list <password file name>\n", exec_name);
}

int main(int argc, char** argv) {
  if (argc != 3) {
    print_usage(argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "single") == 0) {
    // The input MD5 hash is a string in hexadecimal. Convert it to bytes.
    uint8_t input_hash[MD5_DIGEST_LENGTH];
    if (md5_string_to_bytes(argv[2], input_hash)) {
      fprintf(stderr, "Input has value %s is not a valid MD5 hash.\n", argv[2]);
      exit(1);
    }

    // Now call the crack_single_password function
    char result[7];
    if (crack_single_password(input_hash, result)) {
      printf("No matching password found.\n");
    } else {
      printf("%s\n", result);
    }

  } else if (strcmp(argv[1], "list") == 0) {
    // Make and initialize a password set
    password_set_t passwords;
    init_password_set(&passwords);

    // Open the password file
    FILE* password_file = fopen(argv[2], "r");
    if (password_file == NULL) {
      perror("opening password file");
      exit(2);
    }

    int password_count = 0;

    // Read until we hit the end of the file
    while (!feof(password_file)) {
      // Make space to hold the username
      char username[MAX_USERNAME_LENGTH];

      // Make space to hold the MD5 string
      char md5_string[MD5_DIGEST_LENGTH * 2 + 1];

      // Make space to hold the MD5 bytes
      uint8_t password_hash[MD5_DIGEST_LENGTH];

      // Try to read. The space in the format string is required to eat the newline
      if (fscanf(password_file, "%s %s ", username, md5_string) != 2) {
        fprintf(stderr, "Error reading password file: malformed line\n");
        exit(2);
      }

      // Convert the MD5 string to MD5 bytes in our new node
      if (md5_string_to_bytes(md5_string, password_hash) != 0) {
        fprintf(stderr, "Error reading MD5\n");
        exit(2);
      }

      // Add the password to the password set
      add_password(&passwords, username, password_hash);
      password_count++;
    }

    // Now run the password list cracker
    int cracked = crack_password_list(&passwords);

    printf("Cracked %d of %d passwords.\n", cracked, password_count);

  } else {
    print_usage(argv[0]);
    exit(1);
  }

  return 0;
}
