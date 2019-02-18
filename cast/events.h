#include <queue>

enum Event
{
    KEY_DOWN = 0,
    KEY_UP,
    KEY_LEFT,
    KEY_RIGHT
};

class Handler
{
public:
    virtual void handle_event(const Event e) = 0;
};

class EventQueue
{
public:
    EventQueue() : queue() {}
    ~EventQueue() {}

    void check_and_execute();
    void push(const Event e);
    void register_handler(Handler* handler);

private:
    std::queue<Event> queue;
    std::vector<Handler*> handlers;
};
