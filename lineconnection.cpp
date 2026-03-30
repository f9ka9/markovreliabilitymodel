#include "lineconnection.h"

Node* LineConnection::getFirst() const {return first;}
void LineConnection::setFirst(Node* node){first = node;}
Node* LineConnection::getSecond() const {return second;}
void LineConnection::setSecond(Node* node){second = node;}