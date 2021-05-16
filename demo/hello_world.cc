#include "uv.h"
#include <iostream>

int main()
{
    uv_loop_t *loop = new uv_loop_t();
    uv_loop_init(loop);

    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);

    delete loop;

    return 0;
}