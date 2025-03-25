#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Constants from server
const uint8_t TAG_NIL = 0;
const uint8_t TAG_ERR = 1;
const uint8_t TAG_STR = 2;
const uint8_t TAG_INT = 3;
const uint8_t TAG_DBL = 4;
const uint8_t TAG_ARR = 5;

class RedisClient {
private:
    int sock_fd;
    
    void send_command(const std::vector<std::string>& cmd) {
        // Send message header (4 bytes)
        uint32_t total_size = 4; // 4 bytes for number of arguments
        for (const auto& arg : cmd) {
            total_size += 4 + arg.size(); // 4 bytes for length + string
        }
        
        uint32_t size = htonl(total_size);
        write(sock_fd, &size, 4);
        
        // Send number of arguments
        uint32_t nargs = htonl(cmd.size());
        write(sock_fd, &nargs, 4);
        
        // Send each argument
        for (const auto& arg : cmd) {
            uint32_t len = htonl(arg.size());
            write(sock_fd, &len, 4);
            write(sock_fd, arg.data(), arg.size());
        }
    }
    
    std::vector<uint8_t> read_response() {
        uint32_t size;
        read(sock_fd, &size, 4);
        size = ntohl(size);
        
        std::vector<uint8_t> response(size);
        read(sock_fd, response.data(), size);
        return response;
    }

public:
    RedisClient(const char* host, int port) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(host);
        
        if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            throw std::runtime_error("Failed to connect");
        }
    }
    
    ~RedisClient() {
        close(sock_fd);
    }
    
    // Basic string operations
    void set(const std::string& key, const std::string& value) {
        send_command({"set", key, value});
        read_response();
    }
    
    std::string get(const std::string& key) {
        send_command({"get", key});
        auto response = read_response();
        if (response[0] == TAG_NIL) return "";
        if (response[0] != TAG_STR) throw std::runtime_error("Unexpected response type");
        
        uint32_t len;
        memcpy(&len, &response[1], 4);
        len = ntohl(len);
        return std::string((char*)&response[5], len);
    }
    
    bool del(const std::string& key) {
        send_command({"del", key});
        auto response = read_response();
        if (response[0] != TAG_INT) throw std::runtime_error("Unexpected response type");
        int64_t value;
        memcpy(&value, &response[1], 8);
        return ntohll(value) == 1;
    }
    
    // TTL operations
    void pexpire(const std::string& key, int64_t ms) {
        send_command({"pexpire", key, std::to_string(ms)});
        read_response();
    }
    
    int64_t pttl(const std::string& key) {
        send_command({"pttl", key});
        auto response = read_response();
        if (response[0] != TAG_INT) throw std::runtime_error("Unexpected response type");
        int64_t value;
        memcpy(&value, &response[1], 8);
        return ntohll(value);
    }
    
    // Sorted set operations
    bool zadd(const std::string& key, double score, const std::string& member) {
        send_command({"zadd", key, std::to_string(score), member});
        auto response = read_response();
        if (response[0] != TAG_INT) throw std::runtime_error("Unexpected response type");
        int64_t value;
        memcpy(&value, &response[1], 8);
        return ntohll(value) == 1;
    }
    
    bool zrem(const std::string& key, const std::string& member) {
        send_command({"zrem", key, member});
        auto response = read_response();
        if (response[0] != TAG_INT) throw std::runtime_error("Unexpected response type");
        int64_t value;
        memcpy(&value, &response[1], 8);
        return ntohll(value) == 1;
    }
    
    double zscore(const std::string& key, const std::string& member) {
        send_command({"zscore", key, member});
        auto response = read_response();
        if (response[0] == TAG_NIL) return 0.0;
        if (response[0] != TAG_DBL) throw std::runtime_error("Unexpected response type");
        double value;
        memcpy(&value, &response[1], 8);
        return value;
    }
};

int main() {
    try {
        std::cout << "Connecting to Redis-like server..." << std::endl;
        RedisClient client("localhost", 1234);
        
        // Test basic string operations
        std::cout << "\nTesting string operations:" << std::endl;
        client.set("test_key", "Hello, World!");
        std::cout << "GET test_key: " << client.get("test_key") << std::endl;
        
        // Test TTL operations
        std::cout << "\nTesting TTL operations:" << std::endl;
        client.pexpire("test_key", 5000); // 5 seconds
        std::cout << "TTL of test_key: " << client.pttl("test_key") << "ms" << std::endl;
        
        // Test sorted set operations
        std::cout << "\nTesting sorted set operations:" << std::endl;
        client.zadd("scores", 100.0, "Alice");
        client.zadd("scores", 200.0, "Bob");
        client.zadd("scores", 150.0, "Charlie");
        
        std::cout << "Alice's score: " << client.zscore("scores", "Alice") << std::endl;
        std::cout << "Bob's score: " << client.zscore("scores", "Bob") << std::endl;
        std::cout << "Charlie's score: " << client.zscore("scores", "Charlie") << std::endl;
        
        // Test deletion
        std::cout << "\nTesting deletion:" << std::endl;
        client.del("test_key");
        std::cout << "After deletion, GET test_key: " << client.get("test_key") << std::endl;
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}