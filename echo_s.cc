// **************************************************************************************
// * Echo Strings (echo_s.cc)
// * -- Accepts TCP connections and then echos back each string sent.
// **************************************************************************************
#include "echo_s.h"


// **************************************************************************************
// * processConnection()
// * - Handles reading the line from the network and sending it back to the client.
// * - Returns 1 if the client sends "QUIT" command, 0 if the client sends "CLOSE".
// **************************************************************************************
int processConnection(int sockFd) {

  int quitProgram = 0;
  int keepGoing = 1;    
  // char filename[] = "echoFile.txt";
  // int fd = open(filename, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
  // if (fd == -1){
  //   std::cout << "open failed" << strerror(errno) << std::endl;
  //   exit(-1);
  // }
  while (keepGoing) {

    //
    // Call read() call to get a buffer/line from the client.
    // Hint - don't forget to zero out the buffer each time you use it.
    //

    char buffer[1024] = {0};
    int bytesRead = read(sockFd,buffer,1024);
    if (bytesRead < 0 ) {
      std::cout << "read failed" << strerror(errno) << std::endl;
      exit(-1);
    }
    char message[bytesRead] = {0};
    
    for(int i = 0; i < bytesRead-1; i++){
      message[i] = buffer[i];
    }
    DEBUG << "Calling read(" << sockFd << message << ")"<< ENDL;
    DEBUG << "Recieved " << bytesRead << ", containing the string " << buffer << ENDL;
    //
    // Check for one of the commands
    //
    // If CLOSE, close the connection and start waiting for another connection.
    // If QUIT, close the connection and the listening socket and exit your program.
    if(message == "QUIT") {
      close(sockFd);
      DEBUG << "Data included QUIT" << ENDL;
      keepGoing = false;
      quitProgram = false;
    }
    else if(message == "CLOSE") {
      close(sockFd);
      DEBUG << "Data included CLOSE" << ENDL;
      quitProgram = 1; 
      keepGoing = false;
    }
    else {
      //
      // Call write() to send line back to the client.
      //
      int bytesWritten = 0;
      if ((bytesWritten = write(sockFd,buffer,bytesRead)) < 0) {
        std::cout << "write failed" <<strerror(errno) << std::endl;
        exit(-1);
      }
      DEBUG << "Calling write(" << sockFd <<message << ")"<< ENDL;
    }
  }

  return quitProgram;
}
    


// **************************************************************************************
// * main()
// * - Sets up the sockets and accepts new connection until processConnection() returns 1
// **************************************************************************************

int main (int argc, char *argv[]) {

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  boost::log::add_console_log(std::cout, boost::log::keywords::format = "%Message%");
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  int opt = 0;
  while ((opt = getopt(argc,argv,"v")) != -1) {
    
    switch (opt) {
    case 'v':
      boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
      break;
    case ':':
    case '?':
    default:
      std::cout << "useage: " << argv[0] << " -v" << std::endl;
      exit(-1);
    }
  }

  // *******************************************************************
  // * Creating the inital socket is the same as in a client.
  // ********************************************************************
  int     listenFd = -1;
       // Call socket() to create the socket you will use for lisening.
  if ((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cout << "Failed to create listening socket " << strerror(errno) << std::endl;
    exit(-1);
  }     
  DEBUG << "Calling Socket() assigned file descriptor " << listenFd << ENDL;
  

  
  // ********************************************************************
  // * The bind() and calls take a structure that specifies the
  // * address to be used for the connection. On the cient it contains
  // * the address of the server to connect to. On the server it specifies
  // * which IP address and port to lisen for connections.
  // ********************************************************************
  struct sockaddr_in servaddr;
  srand(time(NULL));
  int port = (rand() % 10000) + 1024;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = PF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);


  // ********************************************************************
  // * Binding configures the socket with the parameters we have
  // * specified in the servaddr structure.  This step is implicit in
  // * the connect() call, but must be explicitly listed for servers.
  // ********************************************************************
  DEBUG << "Calling bind(" << listenFd << "," << &servaddr << "," << sizeof(servaddr) << ")" << ENDL;
  int bindSuccesful = 0;
  while (!bindSuccesful) {
    // set and assign the port (if it fails it will choos e a new pot)
    port = (rand() % 10000) + 1024;
    servaddr.sin_port = htons(port);

    if (bind(listenFd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
      std::cout << "bind() failed: " << strerror(errno) << std::endl;
      exit(-1);
    }
    bindSuccesful = true;
  }
  std::cout << "Using port " << port << std::endl;


  // ********************************************************************
  // * Setting the socket to the listening state is the second step
  // * needed to being accepting connections.  This creates a queue for
  // * connections and starts the kernel listening for connections.
  // ********************************************************************
  int listenQueueLength = 1;
  DEBUG << "Calling listen(" << listenFd << "," << listenQueueLength << ")" << ENDL;
  if (listen(listenFd, listenQueueLength) < 0) {
    std::cout << "listen() failed: " << strerror(errno) << std::endl;
    exit(-1);
  }

  // ********************************************************************
  // * The accept call will sleep, waiting for a connection.  When 
  // * a connection request comes in the accept() call creates a NEW
  // * socket with a new fd that will be used for the communication.
  // ********************************************************************
  int quitProgram = 0;
  while (!quitProgram) {
    int connFd = 0;

    DEBUG << "Calling accept(" << listenFd << "NULL,NULL)." << ENDL;
    if ((connFd = accept(listenFd, (sockaddr *) NULL, NULL)) < 0) {
      std::cout << "accept() failed: " << strerror(errno) << std::endl;
      exit(-1);
    }
  //  createThreadAndProcess(connFd);
    // The accept() call checks the listening queue for connection requests.
    // If a client has already tried to connect accept() will complete the
    // connection and return a file descriptor that you can read from and
    // write to. If there is no connection waiting accept() will block and
    // not return until there is a connection.
    
    DEBUG << "We have recieved a connection on " << connFd << ENDL;

    
    quitProgram = processConnection(connFd);
   
    close(connFd);
  }

  close(listenFd);

}
