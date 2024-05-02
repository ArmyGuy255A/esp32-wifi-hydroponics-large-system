#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H


#include <iostream>
#include <Arduino.h>

struct Node {
    String data;
    int priority;
    Node* next;

    Node(String d, int p) : data(d), priority(p), next(nullptr) {}
};

class PriorityQueue {
public:
    PriorityQueue();
    ~PriorityQueue();
    void push(String data, int priority);
    void push(String data);
    String pop();
    String top() const;
    String last() const;

private:
    Node* head;
    bool contains(const String& data) const;
};

#endif
