#include "events.h"

void EventQueue::push(const Event e)
{
    queue.push(e);
}

void EventQueue::check_and_execute()
{
    if (!queue.empty() && !handlers.empty())
    {
        for (auto handler : handlers)
        {
            handler->handle_event(queue.front());
        }

        queue.pop();
    }
}

void EventQueue::register_handler(Handler* handler)
{
    handlers.push_back(handler);
}
