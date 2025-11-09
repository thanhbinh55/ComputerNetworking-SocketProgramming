#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;         // Thư viện cơ sở
namespace http = beast::http;           // Hỗ trợ HTTP (cho Handshake)
namespace websocket = beast::websocket; // Hỗ trợ WebSocket
namespace net = boost::asio;            // Hỗ trợ mạng (IO)
using tcp = boost::asio::ip::tcp;       // TCP sockets

// Hàm xử lý phiên WebSocket
void do_session(tcp::socket& socket) {
    try {
        // Khởi tạo luồng WebSocket
        websocket::stream<tcp::socket> ws{std::move(socket)};

        // Thiết lập các tùy chọn (tùy chọn)
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res) {
                res.set(http::field::server, "Boost.Beast WebSocket Server");
            }));

        // ********** Nâng cấp Kết nối (Handshake) **********
        // Đọc yêu cầu HTTP đầu tiên từ client
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(ws.next_layer(), buffer, req);

        // Chấp nhận (Accept) yêu cầu nâng cấp kết nối thành WebSocket
        ws.accept(req);
        std::cout << "✅ WebSocket Handshake thành công. Bắt đầu giao tiếp.\n";

        // ********** Luồng Giao tiếp WebSocket **********
        while (true) {
            beast::flat_buffer buffer_in;
            
            // Đọc tin nhắn từ client
            ws.read(buffer_in);
            
            // Chuyển đổi dữ liệu sang dạng chuỗi (string_view)
            std::string received_msg = beast::buffers_to_string(buffer_in.data());
            std::cout << "✉️ Nhận từ Client: " << received_msg << "\n";
            
            // Xây dựng tin nhắn phản hồi
            std::string reply_msg = "Server đã nhận: " + received_msg;
            reply_msg = "{\"status\": \"OK\", \"message\": \"" + reply_msg + "\"}";

            // Gửi tin nhắn phản hồi lại client
            ws.write(net::buffer(reply_msg));
            std::cout << "➡️ Phản hồi tới Client: " << reply_msg << "\n";
        }
    } catch(beast::system_error const& se) {
        // Xử lý lỗi hệ thống (ví dụ: kết nối bị đóng)
        if(se.code() != websocket::error::closed) {
            std::cerr << "Lỗi Beast: " << se.code().message() << std::endl;
        } else {
            std::cout << "❌ Kết nối WebSocket đã đóng.\n";
        }
    } catch(std::exception const& e) {
        std::cerr << "Lỗi: " << e.what() << std::endl;
    }
}

int main() {
    auto const address = net::ip::make_address("127.0.0.1"); // Lắng nghe trên localhost
    unsigned short const port = 8080;

    try {
        net::io_context ioc{1}; // Đối tượng IO Context

        // Bộ chấp nhận kết nối (Acceptor)
        tcp::acceptor acceptor{ioc, {address, port}};
        std::cout << "🚀 Server C++ đang lắng nghe tại ws://127.0.0.1:" << port << "\n";

        while (true) {
            tcp::socket socket{ioc};
            // Chờ và chấp nhận kết nối mới
            acceptor.accept(socket);

            // Bắt đầu một phiên mới trong một luồng riêng (hoặc đơn giản là đồng bộ cho ví dụ này)
            do_session(socket);
        }
    } catch (const std::exception& e) {
        std::cerr << "Lỗi Server: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
