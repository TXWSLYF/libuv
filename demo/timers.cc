#include "uv.h"
#include <iostream>

int count = 0;

void callback(uv_timer_t *handle)
{
    std::cout << ++count << std::endl;
    if (count >= 10)
    {
        uv_timer_set_repeat(handle, 1000);
    }

    if (count == 20)
    {
        uv_timer_stop(handle);
    }
}

int main()
{
    uv_loop_t *loop = new uv_loop_t();
    uv_loop_init(loop);

    uv_timer_t timer_req;

    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, callback, 0, 100);

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}