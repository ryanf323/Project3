#include <string>
#include <cstdlib>
#include <vector>
using namespace std;

extern vector <SOCKET *> clientSockets;

/********************
* Name: stringToCharArray
* Purpose: Converts a string in a character array to send
via the socket.
* Arguments: String to be converted
* Returns: New Cstring.
********************/
char* stringToCharArray(string oldStr){

    char *newCStr = new char[oldStr.size()+1];
    strcpy(newCStr, oldStr.c_str());

    return newCStr;
}

/********************
* Name: charArrayToString
* Purpose: Converts a character array to
* a string object
* Arguments: C-String to be converted
* Returns: New string.
********************/
string charArrayToString(char* oldStr){

    string temp(oldStr);

    return temp;
}
