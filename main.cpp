#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/crc.hpp>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::system::error_code;

awaitable<void> read_data(tcp::socket socket)
{
        boost::asio::streambuf buffer;
        uint64_t count_message = 0;
        while (socket.is_open()) {
            count_message++;
            error_code ec;

            auto start_time = std::chrono::steady_clock::now();

            const size_t n = co_await boost::asio::async_read_until(socket, buffer, "\n", use_awaitable);

            boost::asio::streambuf::const_buffers_type bufs = buffer.data();
            std::string str(boost::asio::buffers_begin(bufs),
                            boost::asio::buffers_begin(bufs) + n);

            std::thread crc_thread([&str]() {
                boost::crc_32_type crc;
                crc.process_bytes(str.c_str(), str.size());
                std::cout << "CRC: " << crc.checksum() << std::endl;

            });

            crc_thread.detach();


            // Get the end time
            auto end_time = std::chrono::steady_clock::now();
            auto duration_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            if (ec == boost::asio::error::eof) {
//                std::cout << "End of File: "<<std::endl;
            }
            std::cout << "Received : "<< std::dec << count_message << " : " << n << " bytes from " << socket.remote_endpoint() << " time : "<< duration_time << std::endl;
            std::string data(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + n);
            std::cout << "Data: " << data << std::endl;
            buffer.consume(n);



        }
}

awaitable<void> start_server(boost::asio::io_context& io_context)
{
    tcp::acceptor acceptor(io_context, {tcp::v4(), 8081});

    if(!acceptor.is_open()) {
        std::cout << "Closed socket\n";
    }

    for (;;) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(io_context, read_data(std::move(socket)), detached);
    }
}

int main()
{
    try {
        boost::asio::io_context io_context(1);
        co_spawn(io_context, start_server(io_context), detached);
        io_context.run();
    } catch (const std::exception& ex) {
        std::cerr << "Exception caught in main: " << ex.what() << std::endl;
    }
    return 0;
}
