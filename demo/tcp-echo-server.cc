#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <iostream>

#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;

typedef struct
{
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req)
{
    write_req_t *wr = (write_req_t *)req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void on_close(uv_handle_t *handle)
{
    free(handle);
}

void echo_write(uv_write_t *req, int status)
{
    if (status)
    {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread > 0)
    {
        write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t *)req, client, &req->buf, 1, echo_write);
        return;
    }
    if (nread < 0)
    {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)client, on_close);
    }

    free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status)
{
    std::cout << server->io_watcher.fd << std::endl;
    if (status < 0)
    {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    // uv_accept 主要干的事情，就是将 server.accepted_fd 传递给 client->io_watcher.fd
    if (uv_accept(server, (uv_stream_t *)client) == 0)
    {
        uv_read_start((uv_stream_t *)client, alloc_buffer, echo_read);
    }
    else
    {
        uv_close((uv_handle_t *)client, on_close);
    }
}

int main()
{
    loop = uv_default_loop();

    /**
     * handle，表示一个 long-lived objects
     */
    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

    // 这一步会新建 socket，并绑定端口，server.io_watcher.fd 会被赋值
    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);

    // uv_listen 会将 server.io_watcher.cb 从初始的 uv__stream_io 替换为 uv__server_io
    int r = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, on_new_connection);
    if (r)
    {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
