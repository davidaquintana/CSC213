
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "message.h"
#include "socket.h"
#include "ui.h"

void *thread_receive(void *arg);

void distribute(char* message, int excluded_fd);

void *thread_listen(void *arg);

void add_connection(int new_fd);

void remove_connection(int remove_fd);

// typedef struct connections_node
// {
//   int fd;

//   struct connections_node *next;

// } connections_node_t;

int *connections;
int max_size = 20;
int num_clients = 0;

// Keep the username in a global so we can access it from the callback
const char *username;

// This function is run whenever the user hits enter after typing a message
void input_callback(const char *message)
{
  //check for quitting message
  if (strcmp(message, "quit") == 0 || strcmp(message, "q") == 0)
  {
    //Send a message noting a quit to all other users
    char quit_message[strlen(username) + 15];
    sprintf(quit_message, "INFO:%s has left", username);
    distribute(quit_message, -1);
    //run through each connection and close the port
    for(int i = 0; i < num_clients; i++){
      close(connections[i]);
    }
    //then free the array of connections
    free(connections);
    ui_exit();
  }
  else
  {
    //need to send a message to peers including the username and the message
    //so add 2 chars for colon and terminating char
    int length = strlen(message) + strlen(username) + 2;
    char* new_message = malloc(length * sizeof(char));

    //fill in the modified message
    snprintf(new_message, length, "%s:%s", username, message);
    new_message[length - 1] = '\0';

    //send out the message to all connections. 
    //-1 indicates this message is from the user, not another connection
    distribute(new_message, -1);

    //display the message and free the malloc'd space
    ui_display(username, message);
    free(new_message);
  }
}

int main(int argc, char **argv)
{
  // Make sure the arguments include a username
  if (argc != 2 && argc != 4)
  {
    fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
    exit(1);
  }

  //create the global struct of initial size
  connections = malloc(sizeof(int) * max_size);

  // Save the username in a global
  username = argv[1];

  // Set up a server socket to accept incoming connections
  unsigned short port = 0;
  // get the fd number
  int server_fd = server_socket_open(&port);
  if (server_fd == -1)
  {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }

  // Did the user specify a peer we should connect to?
  if (argc == 4)
  {
    // Unpack arguments
    char *peer_hostname = argv[2];
    unsigned short peer_port = atoi(argv[3]);

    // Connect to another peer in the chat network
    int peer_fd = socket_connect(peer_hostname, peer_port);
    if (peer_fd == -1)
    {
      perror("Failed to connect");
      exit(EXIT_FAILURE);
    }

    // add the new connection into the list, which will open a thread to listen on that fd
    add_connection(peer_fd);
  }

  //create the thread that will act as this nodes server
  pthread_t main_server_thread;

  //will run the listen function, which initiates a call to listen on this nodes fd
  int rc = pthread_create(&main_server_thread, NULL, &thread_listen, &server_fd);
  if(rc != 0){
    perror("Could not create server thread");
    exit(EXIT_FAILURE);
  }

  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  // display the port number for this node
  char temp[22];
  sprintf(temp, "Connect on port %d", port);
  ui_display("INFO", temp);

  //Send a message noting a quit to all other users
  if(argc == 4){
    char join_local[31];
    sprintf(join_local, "Successfully joined port %d", atoi(argv[3]));
    ui_display("INFO", join_local);

    char join_message[strlen(username) + 17];
    sprintf(join_message, "INFO:%s has joined", username);
    distribute(join_message, -1);
  }

  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();

  return 0;
}

/**
 * thread function that receives messages
 * 
 * @param arg the fd of the client which this thread is receiving messages from
 */
void *thread_receive(void *arg)
{
  int client_fd = *(int*)arg;

  // run a while loop that is constantly listening on some port number
  while (1)
  {
    char* message = receive_message(client_fd);
    //check for faulty message
    if(message == NULL){
      //remove the client if it gets a faulty message
      remove_connection(client_fd);
      return NULL;
    }

    //once we have a valid message, duplicate message to decompose
    char* message_only = message;

    //send to other peers except whoever sent the message
    distribute(message, client_fd);

    //now decode the message and get the user that sent it
    //must be done following sending in order to avoid altering the encoding
    char* client_username = strsep(&message_only, ":");

    //finally print it to our UI and free the original message
    ui_display(client_username, message_only);
    free(message);
  }

  return NULL;
}

/**
 * thread function that sends messages to all relevant connections
 * 
 * @param message the message we want to send out
 * @param excluded_fd the fd of the node that sent the message
 */
void distribute(char* message, int excluded_fd){
  
  //increment through all connections
  for(int i = 0; i < num_clients; i++){
    //send the message to all connections that did not send it to uuuus
    if(connections[i] != excluded_fd){
      send_message(connections[i], message);
    }
  }
}

/**
 * thread function that listens for new connections
 * 
 * @param arg the fd of the server that we want to listen for other connections on
 */
void *thread_listen(void *arg){

  //unpack thread argument
  int server_fd = *(int*)arg;

  //listen for new connections to this thread
  if (listen(server_fd, 1)) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  // run a while loop that is constantly looking for new connections
  while (1)
  {
    //accept a new connection
    int client_fd = server_socket_accept(server_fd);
    // check that the connection worked
    if(client_fd == -1){
      perror("Could not accept new connection");
      exit(EXIT_FAILURE);
    }

    //ensure we have space to add it, if not then add more space
    if(num_clients >= max_size){
      max_size *= 2;
      connections = realloc(connections, sizeof(int) * max_size);
    }

    //add the new connection and open a new thread for it
    add_connection(client_fd);
  }

  return NULL;
}

/**
 * function that adds a connection to the global array and initiates a new struct to listen on that fd
 * 
 * @param new_fd the fd of the connection that has been accepted by our server
 * 
*/
void add_connection(int new_fd)
{
  //add the fd and increment the number of clients on our server
  connections[num_clients] = new_fd;
  num_clients++;

  //now that the connection is added, we need to create a new thread to handle listening for this client
  pthread_t new_client_thread;
  //have the thread begin receiving messages from the connection we just added
  int rc = pthread_create(&new_client_thread, NULL, thread_receive, &(connections[num_clients - 1]));
  if(rc != 0){
    perror("could not create new thread");
    exit(EXIT_FAILURE);
  }
}

/**
 * function that removes a bad connection
 * 
 * @param remove_fd the fd of the connection that we want to remove
 * This would occur when we have quit some connection and there is no longer a valid message being sent from it
 * 
*/
void remove_connection(int remove_fd){
  
  //increment through the connections
  for(int i = 0; i < num_clients; i++){
    //if we hit the one we want to remove, then move the last connection to the faulty connection's index
    //then shorten the array of connections
    if(connections[i] == remove_fd){
      connections[i] = connections[num_clients - 1];
      num_clients--;
      //end the loop
      break;
    }
  }
}
