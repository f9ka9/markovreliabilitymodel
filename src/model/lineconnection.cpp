#include "lineconnection.h"

Node* LineConnection::getSource() const {return source;}
void LineConnection::setSource(Node* node) {source = node;}

Node* LineConnection::getTarget() const {return target;}
void LineConnection::setTarget(Node* node) {target = node;}
