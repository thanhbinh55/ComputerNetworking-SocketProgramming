#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>
#include <windows.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

std::mutex cout_mutex; // khoa de in ra console an toan giua cac luong

// Ham tra ve thoi gian hien tai (dang chuoi)
std::string timestamp() {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[32];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

// Ham xu ly mot phien WebSocket
void do_session(tcp::socket socket) {
    try {
        auto remote_ep = socket.remote_endpoint();
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Client ket noi tu: " << remote_ep.address().to_string()
                      << ":" << remote_ep.port() << std::endl;
        }

        websocket::stream<tcp::socket> ws{std::move(socket)};
        ws.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
            res.set(http::field::server, "Boost.Beast C++ WebSocket Server");
        }));

        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(ws.next_layer(), buffer, req);
        ws.accept(req);

        std::cout << "[" << timestamp() << "] Handshake thanh cong." << std::endl;

        while (true) {
            beast::flat_buffer msg_buffer;
            ws.read(msg_buffer);
            std::string msg = beast::buffers_to_string(msg_buffer.data());

            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "[" << timestamp() << "] Nhan: " << msg << std::endl;
            }

            std::string reply = "{\"time\": \"" + timestamp() +
                                "\", \"status\": \"OK\", \"echo\": \"" + msg + "\"}";
            ws.text(true);
            ws.write(net::buffer(reply));

            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "[" << timestamp() << "] Gui lai: " << reply << std::endl;
            }
        }
    } catch (const beast::system_error& se) {
        if (se.code() != websocket::error::closed)
            std::cerr << "Loi Beast: " << se.code().message() << std::endl;
        else
            std::cout << "Ket noi bi dong." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Loi: " << e.what() << std::endl;
    }
}

int main() {
    try {
        SetConsoleOutputCP(CP_UTF8);
        unsigned short port = 9001;
        auto address = net::ip::make_address("127.0.0.1");
        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, {address, port}};
        std::cout << "Server dang lang nghe tai ws://127.0.0.1:" << port << std::endl;

        // Xu ly Ctrl+C de tat server an toan
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            std::cout << "\nServer dang tat..." << std::endl;
            ioc.stop();
        });

        while (true) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);
            std::thread(&do_session, std::move(socket)).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Loi server: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
