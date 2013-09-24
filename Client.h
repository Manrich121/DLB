#ifndef CLIENT_H
#define CLIENT_H

#include "Point.h"
class Client {

public:
    Client();   // constructor
    Client(Point p, int max); //contructor with init pos
    ~Client();  //Destructer
    void move();

    Point loc;

protected:
    int vx;
    int vy;
    int edge;
};

#endif // CLIENT_H
