
// main.cpp - WebSocket Client Application
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/context.hpp>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

// Class to handle WebSocket 

communication
class WebSocketClient {
private:
    net::io_context ioc_;
    ssl::context ctx_{ssl::context::tlsv12_client};
    std::unique_ptr<websocket::stream<beast::ssl_stream<tcp::socket>>> ws_;
    std::atomic<bool> is_connected_{false};
    std::thread read_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_ = false;

public:
    WebSocketClient() {
        // Set up SSL context
        ctx_.set_default_verify_paths();
        ctx_.set_verify_mode(ssl::verify_peer);
    }


    ~WebSocketClient() {
        disconnect();
        if (read_thread_.joinable()) {
            read_thread_.join();
        }
    }

    bool connect(const std::string& host, const std::string& port, const std::string& target) {
        try {
            // Look up the domain name
            tcp::resolver resolver{ioc_};
            auto const results = resolver.resolve(host, port);
            
            // Create the WebSocket with SSL support
            ws_ = std::make_unique<websocket::stream<bea

st::ssl_stream<tcp::socket>>>(ioc_, ctx_);
            
            // Set SNI hostname (required for SSL)
            if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host.c_str())) {
                throw beast::system_error(
                    beast::error_code(static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                    "Failed to set SNI hostname");
            }
            
            // Connect to the TCP endpoint
            net::connect(ws_->next_layer().next_layer(), results.begin(), results.end());
            
            // Perform SSL handshake

            ws_->next_layer().handshake(ssl::stream_base::client);
            
            // Perform WebSocket handshake
            ws_->handshake(host, target);
            
            is_connected_ = true;
            
            // Start a thread to read incoming messages
            read_thread_ = std::thread([this]() {
                read_messages();
            });
            
            std::cout << "Connected to WebSocket server: " << host << target << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error connecting to 

WebSocket server: " << e.what() << std::endl;
            return false;
        }
    }

    void disconnect() {
        if (is_connected_) {
            try {
                // Signal the read thread to stop
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    shutdown_ = true;
                }
                cv_.notify_all();
                
                // Close the WebSocket connection
                ws_->close(websocket::close_code::normal);

                is_connected_ = false;
                std::cout << "Disconnected from WebSocket server" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error during disconnect: " << e.what() << std::endl;
            }
        }
    }

    bool is_connected() const {
        return is_connected_;
    }

    bool send_message(const std::string& message) {
        if (!is_connected_) {
            std::cerr << "Not connected to WebSocket server" << std::endl;
            return false;
        }


        try {
            ws_->write(net::buffer(message));
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error sending message: " << e.what() << std::endl;
            is_connected_ = false;
            return false;
        }
    }

private:
    void read_messages() {
        try {
            beast::flat_buffer buffer;
            
            while (is_connected_) {
                // Check if we should shutdown
                {
                    std::unique_lock<std::mutex> 

lock(mutex_);
                    if (cv_.wait_for(lock, std::chrono::milliseconds(100), 
                                    [this]{ return shutdown_; })) {
                        break;
                    }
                }
                
                // Check if there's a message to read
                if (ws_->is_open() && ws_->got_binary() || ws_->got_text()) {
                    ws_->read(buffer);
                    std::cout << "\nReceived: " << beast::make_printable(buffer.data()) << std::endl;
                    std::cout << "> ";
                    std::cout.flush();
                    buffer.consume(buffer.size());
                }

                
                // Sleep a bit to avoid busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        } catch (const beast::system_error& se) {
            if (se.code() != websocket::error::closed) {
                std::cerr << "WebSocket read error: " << se.what() << std::endl;
            }
            is_connected_ = false;
        } catch (const std::exception& e) {
            std::cerr << "Exception in read thread: " << e.what() << std::endl;
            is_connected_ = false;
        }
    }
};


// Parse URL into components
bool parse_url(const std::string& url, std::string& protocol, std::string& host, 
               std::string& port, std::string& target) {
    
    // Find protocol delimiter
    auto proto_end = url.find("://");
    if (proto_end == std::string::npos) {
        std::cerr << "Invalid URL format (no protocol)" << std::endl;
        return false;
    }
    
    protocol = url.substr(0, proto_end);
    
    // Find host and target
    auto host_start = proto_end + 3;
    auto path_start = url.find("/", host_start);
    

    if (path_start == std::string::npos) {
        host = url.substr(host_start);
        target = "/";
    } else {
        host = url.substr(host_start, path_start - host_start);
        target = url.substr(path_start);
    }
    
    // Check for port specification
    auto port_start = host.find(":");
    if (port_start != std::string::npos) {
        port = host.substr(port_start + 1);
        host = host.substr(0, port_start);
    } else {
        // Default ports
        if (protocol == "ws" || protocol == "http") {
            port = "80";
        } else if (protocol == "wss" || protocol == "https") {

            port = "443";
        } else {
            std::cerr << "Unknown protocol: " << protocol << std::endl;
            return false;
        }
    }
    
    return true;
}

int main() {
    WebSocketClient client;
    std::string server_url = "wss://echo.websocket.events/.ws";
    std::string input;
    std::string protocol, host, port, target;
    
    std::cout << "WebSocket Client" << std::endl;
    std::cout << "----------------" << std::endl;

    
    // Parse the default URL
    if (!parse_url(server_url, protocol, host, port, target)) {
        std::cerr << "Failed to parse default URL" << std::endl;
        return 1;
    }
    
    bool running = true;
    while (running) {
        if (!client.is_connected()) {
            std::cout << "Enter WebSocket server URL (default: " << server_url << "): ";
            std::getline(std::cin, input);
            
            if (input.empty()) {
                input = server_url;
            }
            
            if (!parse_url(input, protocol, host, 

port, target)) {
                std::cerr << "Invalid URL format" << std::endl;
                continue;
            }
            
            std::cout << "Connecting to " << host << ":" << port << target << "..." << std::endl;
            if (!client.connect(host, port, target)) {
                std::cout << "Connection failed. Try again? (y/n): ";
                std::getline(std::cin, input);
                if (input != "y" && input != "Y") {
                    running = false;
                }
                continue;
            }
        }
        
        std::cout << "\nCommands:" << 

std::endl;
        std::cout << "  /quit - Exit the application" << std::endl;
        std::cout << "  /disconnect - Disconnect from the server" << std::endl;
        std::cout << "  Any other input will be sent as a message" << std::endl;
        
        while (client.is_connected()) {
            std::cout << "> ";
            std::getline(std::cin, input);
            
            if (input == "/quit") {
                running = false;
                break;
            } else if (input == "/disconnect") {
                client.disconnect();
                break;
            } else if (!input.empty()) {
                if (!client.send_message(input)) {
                    std::cout << "Failed to send 

message. Connection lost." << std::endl;
                    break;
                }
            }
        }
    }
    
    std::cout << "Exiting application" << std::endl;
    return 0;
}
