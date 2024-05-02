#include "PriorityQueue.h"

PriorityQueue::PriorityQueue() : head(nullptr) {}

PriorityQueue::~PriorityQueue() {
    Node* current = head;
    while (current != nullptr) {
        Node* temp = current;
        current = current->next;
        delete temp;
    }
}

bool PriorityQueue::contains(const String& data) const {
    Node* current = head;
    while (current != nullptr) {
        if (current->data == data) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void PriorityQueue::push(String data, int priority) {
    if (contains(data)) {
        std::cout << "Item already exists, not added: " << data << std::endl;
        return;
    }

    Node* newNode = new Node(data, priority);
    if (head == nullptr || priority > head->priority) {
        newNode->next = head;
        head = newNode;
    } else {
        Node* current = head;
        while (current->next != nullptr && current->next->priority >= priority) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

void PriorityQueue::push(String data) {
    push(data, 100);
}

String PriorityQueue::pop() {
    if (head == nullptr) return "";

    Node* temp = head;
    String result = head->data;
    head = head->next;
    delete temp;
    return result;
}

String PriorityQueue::top() const {
    if (head == nullptr) return "";
    return head->data;
}

String PriorityQueue::last() const {
    if (head == nullptr) return "";

    Node* current = head;
    while (current->next != nullptr) {
        current = current->next;
    }
    return current->data;
}
