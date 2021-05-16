#include "uv.h"
#include <iostream>

uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t write_req;

static char buffer[1024];
static uv_buf_t iov;

void on_read(uv_fs_t *req);

void on_write(uv_fs_t *req)
{
    if (req->result < 0)
    {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int)req->result));
    }
    else
    {
        // offset 设置为 -1，会使用当前文件描述符指向的 offset 并且更新它
        // https://man7.org/linux/man-pages/man2/preadv.2.html
        uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read);
    }
}

void on_read(uv_fs_t *req)
{
    if (req->result < 0)
    {
        fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
    }
    else if (req->result == 0) // 遇到 EOF，说明读到文件末尾了
    {
        uv_fs_t close_req;
        // synchronous
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
    }
    else if (req->result > 0) // the amount of data that was read
    {
        iov.len = req->result;
        // 文件描述符 1 对应着标椎输出
        uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);
    }
}

void on_open(uv_fs_t *req)
{
    // The request passed to the callback is the same as the one the call setup
    // function was passed.
    assert(req == &open_req);
    // 此时 req->result 返回的是打开的文件描述符
    if (req->result >= 0)
    {
        iov = uv_buf_init(buffer, sizeof(buffer));
        uv_fs_read(uv_default_loop(), &read_req, req->result,
                   &iov, 1, -1, on_read);
    }
    else
    {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
    }
}

int main(int argc, char **argv)
{
    uv_fs_open(uv_default_loop(), &open_req, argv[1], O_RDONLY, 0, on_open);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&read_req);
    uv_fs_req_cleanup(&write_req);
    return 0;
}