#include <windows.h>
#include  <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <winsock.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include "global.h"

using namespace std;

//Global Mutex
HANDLE vectorMutex;
HANDLE fileMutex;

//Global Variables
int usercount = 0;
vector <SOCKET *> clientSockets;

struct threadParams{

    SOCKET client_socket;
    HANDLE clientThread;

}StructObject;




// our thread for receiving commands
DWORD WINAPI receive_cmds(LPVOID lpParam)
{
    string userName;

    threadParams *params = (threadParams *) lpParam;
    // set our socket to the socket passed in as a parameter
    SOCKET current_client = params -> client_socket;

   cout <<"New thread created..." << endl;

    // buffer to hold our received data
    char rcvbuf[256];
    memset(&rcvbuf[0], '\0', sizeof(rcvbuf));

    // buffer to hold our sent data
    char sendData[256];
    memset(&sendData[0], '\0', sizeof(sendData));
    // for error checking
    int res;

    //Ask for user name
    strcpy(sendData,"Your name, please\n");
    cout << "Sent:\t Your name, please" << endl;
    send(current_client,sendData,strlen(sendData),0);
    memset(&sendData[0], '\0', sizeof(sendData));

    //Receive user name
     res = recv(current_client,rcvbuf,sizeof(rcvbuf),0);
     userName= charArrayToString(rcvbuf);
     cout << "User name is: " << userName << endl;
     memset(&sendData[0], '\0', sizeof(rcvbuf));

     //Send OK
    strcpy(sendData,"OK");
    send(current_client,sendData,strlen(sendData),0);
    cout << "Sent:\tOK" << endl;
    memset(&sendData[0], '\0', sizeof(sendData));

    // the message recv loop
    while(true)
    {
        res = recv(current_client,rcvbuf,sizeof(rcvbuf),0); // recv cmds
        //what did I receive
        cout << "Received:" << rcvbuf << "" << "\n";

        //Send OK
        strcpy(sendData,"OK");
        send(current_client,sendData,strlen(sendData),0);
        cout << "Sent:\tOK" << endl;
        memset(&sendData[0], '\0', sizeof(sendData));

        if (res != 0)
        {
            /* Start putting your stuff here */
            if ( strncmp( rcvbuf, "quit",4 )==0)
            {
                send(current_client,"quit",4,0);
                cout <<"Client disconnected" << endl;
                usercount -= 1;
                cout <<"Number of clients: " << usercount << endl;
                break;
            }
            else
            {

                //create message
                string message = userName + "," + charArrayToString(rcvbuf);

                //save to file

                /**Request Mutex Lock**/
                DWORD dwWaitResult;

                // Request ownership of mutex.
                    //Wait until the specified object is in the
                    //signaled state or the time-out interval elapses.
                    //see: http://msdn.microsoft.com/en-us/library/ms687032%28v=vs.85%29.aspx
                    dwWaitResult = WaitForSingleObject(
                    fileMutex,    // handle to mutex
                    INFINITE);  // no time-out interval

                    switch (dwWaitResult)
                    {
                        // The thread got ownership of the mutex
                        case WAIT_OBJECT_0:
                        {

                /********************/

                            ofstream fout;
                            fout.open("message_log.txt", ios::out | ios::app);

                            if(fout.is_open()==false){
                                cerr << "Unable to open file!" << endl;
                                return 3;
                            }
                        fout << message <<"\n";
                        fout.close();

                        //forward message to clients --DOES NOT EXECUTE!--
                        for (int i = 0 ; i < clientSockets.size();i++)
                        {
                            cout << "Attemping to forward message..." << endl;
                            int result;
                        result = send(*clientSockets.at(i),stringToCharArray(message), message.length(), 0);
                            if (result == SOCKET_ERROR)
                            {
                                cout << "Error Forwarding Message: " << WSAGetLastError() <<endl;
                            }
                        }


                /**Release Mutex Lock**/
                        ReleaseMutex(fileMutex);

                        //break;

                            // The thread got ownership of an abandoned mutex
                            // This is not good.
                        }
                        case WAIT_ABANDONED:
                            //return FALSE;
                            Sleep(1);
                    }
                /********************/

            }//end of else clause

            // clear buffers

            memset(&rcvbuf[0], '\0', sizeof(rcvbuf));
            memset(&sendData[0], '\0', sizeof(sendData));

    }//end of while loop
    }
}// end of function

int main()
{

    cout <<"Starting up multi-threaded TCP server" << endl;

    //Create File Mutex
        fileMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    if (fileMutex == NULL) //if create mutex failed
    {
        cout<<"CreateMutex error: " << GetLastError() << endl;
        return 1;
    }

    //Create Vector Mutex
        vectorMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex

    if (vectorMutex == NULL) //if create mutex failed
    {
        cout<<"CreateMutex error: " << GetLastError() << endl;
        return 1;
    }

    // our masterSocket(socket that listens for connections)
    SOCKET sock;

    // for our thread
//    DWORD thread;

    WSADATA wsaData;
    sockaddr_in server;

    // start winsock
    int ret = WSAStartup(0x101,&wsaData); // use highest version of winsock avalible

    if(ret != 0)
    {
        return 0;
    }

    // fill in winsock struct ...
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(31000); //convert the port to unsigned number

    // create our socket
    sock=socket(AF_INET,SOCK_STREAM,0);

    if(sock == INVALID_SOCKET)
    {
        return 0;
    }

    // bind our socket to a port(port 123)
    if( bind(sock,(sockaddr*)&server,sizeof(server)) !=0 )
    {
        return 0;
    }

    // listen for a connection
    if(listen(sock,5) != 0)
    {
        return 0;
    }

    // socket that we sendrecv data on
    //SOCKET client;

    sockaddr_in from;
    int fromlen = sizeof(from);

    // loop forever
    while(true)
    {
        // accept connections
        StructObject.client_socket = accept(sock,(struct sockaddr*)&from,&fromlen);
        /**is a mutex needed here?**/
        clientSockets.push_back(&StructObject.client_socket);
        cout << "Added " << *clientSockets.at(clientSockets.size()-1) << endl;
        cout << "Size of Sockets Vector: " << clientSockets.size() << endl;

        cout << "Client connected" << endl;
        usercount += 1;
        cout <<"Number of clients: " << usercount << endl;


        // create thread to run receive_cmds here
        //and send the client socket as a parameter
        //HANDLE clientThread;
        DWORD clientThreadID;

        StructObject.clientThread = CreateThread(
            NULL,       // default security attributes
            0,          // default stack size
            (LPTHREAD_START_ROUTINE) receive_cmds,
            &StructObject,       // no thread function threadParams
            0,          // default creation flags
            &clientThreadID); // receive thread identifier

        /**Request Mutex Lock**/
            DWORD dwWaitResult;

            // Request ownership of mutex.

                //Wait until the specified object is in the
                //signaled state or the time-out interval elapses.
                //see: http://msdn.microsoft.com/en-us/library/ms687032%28v=vs.85%29.aspx
                dwWaitResult = WaitForSingleObject(
                vectorMutex,    // handle to mutex
                INFINITE);  // no time-out interval

                switch (dwWaitResult)
                {
                    // The thread got ownership of the mutex
                    case WAIT_OBJECT_0:
                        {

                        /********************/
                        //Remove Socket from vector
                        for (int i = 0; i < clientSockets.size(); i++)
                        {
                            if(*clientSockets.at(i)==StructObject.client_socket)
                            {
                                clientSockets.erase(clientSockets.begin()+i);
                            }
                        }
                        /**Release Mutex Lock**/

                            ReleaseMutex(vectorMutex);
                        }
                        case WAIT_ABANDONED:
                        {
                            //return false;
                        }

            }//end of switch case

   }//end of infinite while loop


    // shutdown winsock
    closesocket(sock);
    WSACleanup();

 return 0;

}//end of main
