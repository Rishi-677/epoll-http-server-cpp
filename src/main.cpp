#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <string.h>
#include "http_parser.h"
#include "http_response.h"
#include "router.h"
#include "routes.h"
#include <signal.h>
#include <atomic>
#include "middleware.h"
#include "middleware_setup.h"

using namespace std;

atomic<bool> running(true);

#define PORT 8080
#define MAX_EVENTS 1000
#define THREAD_COUNT 4

Router router;
unordered_map<int, string> client_ip_map;

void handleSignal(int)
{
    cout << "\nGraceful shutdown initiated..." << std::endl;
    running = false;
}

void make_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

struct Task
{
    int client_fd;
    string request;
    string client_ip;
};

queue<Task> taskQueue;
mutex queueMutex;
condition_variable cv;

void worker()
{
    while (true)
    {
        Task task;

        {
            unique_lock<mutex> lock(queueMutex);
            cv.wait(lock, []
                    { return !taskQueue.empty() || !running; });

            if (!running && taskQueue.empty())
                return; // exit thread cleanly

            task = taskQueue.front();
            taskQueue.pop();
        }

        // Parse request
        HttpRequest req = HttpParser::parse(task.request);
        req.client_ip = task.client_ip;

        cout << "\n-----------Client Request-----------" << endl;

        // Build response
        HttpResponse res;

        runMiddlewares(req, res);

        if (!res.body.empty())
        {
            string responseStr = res.toString();
            write(task.client_fd, responseStr.c_str(), responseStr.size());
            close(task.client_fd);
            client_ip_map.erase(task.client_fd);
            continue;
        }

        bool found = router.route(req, res);

        auto end = std::chrono::high_resolution_clock::now();
        auto end_us = std::chrono::duration_cast<std::chrono::microseconds>(
                          end.time_since_epoch())
                          .count();

        if (req.headers.count("__start_time"))
        {
            long start_us = stol(req.headers["__start_time"]);
            cout << "[DONE] " << req.method << " "
                 << req.path << " in "
                 << (end_us - start_us) << " µs\n";
        }

        if (!found)
        {
            res.status = 404;
            res.statusText = "Not Found";
            res.headers["Content-Type"] = "text/plain";
            res.body = "404 Route Not Found\n";
        }

        // Send response
        string responseStr = res.toString();
        write(task.client_fd, responseStr.c_str(), responseStr.size());
        close(task.client_fd);
        client_ip_map.erase(task.client_fd);
    }
}

int main()
{
    signal(SIGINT, handleSignal);  // Ctrl+C
    signal(SIGTERM, handleSignal); // kill

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (sockaddr *)&address, sizeof(address));
    listen(server_fd, 128);
    make_non_blocking(server_fd);

    // epoll setup
    int epoll_fd = epoll_create1(0);
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    epoll_event events[MAX_EVENTS];

    // Thread pool
    vector<thread> workers;
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        workers.emplace_back(worker);
    }

    cout << "Server running on http://localhost:8080\n";

    registerRoutes(router); // Registering the routes
    registerMiddlewares();  // Registering the middlewares

    // Event loop
    while (running)
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;

            // New connection
            if (fd == server_fd)
            {
                while (true)
                {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);

                    int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);

                    if (client_fd < 0)
                        break;

                    make_non_blocking(client_fd);

                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
                    client_ip_map[client_fd] = ip_str;

                    epoll_event ev;
                    ev.events = EPOLLIN;
                    ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                }
            }
            // Client data
            else
            {
                char buffer[3000];
                int bytes = read(fd, buffer, sizeof(buffer));

                if (bytes <= 0)
                {
                    close(fd);
                    client_ip_map.erase(fd);
                }
                else
                {
                    Task task;
                    task.client_fd = fd;
                    task.request = string(buffer, bytes);
                    task.client_ip = client_ip_map[fd];

                    {
                        lock_guard<mutex> lock(queueMutex);
                        taskQueue.push(task);
                    }
                    cv.notify_one();
                }
            }
        }
    }

    running = false; // closing the Server
    cv.notify_all();

    for (auto &t : workers)
    {
        if (t.joinable())
            t.join();
    }

    cout << "Shutting down server..." << endl;

    close(server_fd);
    close(epoll_fd);
}