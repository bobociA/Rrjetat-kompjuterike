#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <ctime>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

const std::string SENSOR_ID = "SENSOR_001";
const std::string MANAGEMENT_HOST = "192.168.56.102";
const int MANAGEMENT_PORT = 5000;
const int SENSOR_PORT = 5001;

json generate_sensor_data() {
    static std::random_device rd;
    static std::mt19937 engine(rd());
    std::uniform_real_distribution<double> temp(20.0, 40.0);
    std::uniform_int_distribution<int> hum(40, 80);

    std::time_t now = std::time(nullptr);
    char timestamp[64];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    json data;
    data["sensor_id"] = SENSOR_ID;
    data["temperature"] = temp(engine);
    data["humidity"] = hum(engine);
    data["timestamp"] = timestamp;

    return data;
}

void send_data_loop() {
    while (true) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "[SENSOR] Client socket error\n";
            std::this_thread::sleep_for(std::chrono::seconds(10));
            continue;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(MANAGEMENT_PORT);
        inet_pton(AF_INET, MANAGEMENT_HOST.c_str(), &addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            json data = generate_sensor_data();
            std::string msg = data.dump() + "\n";

            send(sock, msg.c_str(), msg.size(), 0);
            std::cout << "[SENSOR -> MGMT] Sent:\n" << msg;

            char buf[1024];
            ssize_t bytes = recv(sock, buf, sizeof(buf) - 1, 0);
            if (bytes > 0) {
                buf[bytes] = '\0';
                std::cout << "[MGMT -> SENSOR] Response: " << buf << std::endl;
            }
        } else {
            std::cerr << "[SENSOR] Connect failed\n";
        }

        close(sock);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void listen_for_responses() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[SENSOR] Server socket failed\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SENSOR_PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0 ||
        listen(server_fd, 5) < 0) {
        perror("[SENSOR]");
        close(server_fd);
        return;
    }

    std::cout << "[SENSOR] Listening on port " << SENSOR_PORT << std::endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd >= 0) {
            char buf[1024];
            ssize_t bytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
            if (bytes > 0) {
                buf[bytes] = '\0';
                std::cout << "[MGMT -> SENSOR] Received: " << buf << std::endl;
                const char* ack = "{\"status\":\"sensor_received\"}\n";
                send(client_fd, ack, strlen(ack), 0);
            }
            close(client_fd);
        }
    }
}

int main() {
    std::cout << "=== IoT SENSOR MODULE ===" << std::endl;

    std::thread sender(send_data_loop);
    std::thread listener(listen_for_responses);

    sender.join();
    listener.join();

    return 0;
}

