#include <thread>
#include "Threads/Engine.hpp"
#include "Threads/IO.hpp"
#include "Threads/Logger.hpp"


std::thread logging;
std::thread io;
std::thread engine;

int main() {
    lve::IO::Init();
    lve::Logger::Init();
    lve::Engine::Init();

    io = std::thread([]() {
            lve::IO::Get().start();
    });

    logging = std::thread([]() {
        lve::Logger::Get().start();
    });

    engine = std::thread([]() {
        lve::Engine::Get().run();
    });

    std::this_thread::sleep_for(std::chrono::seconds(5));
    lve::Engine::Shutdown();
    lve::Logger::Shutdown();
    lve::IO::Shutdown();


    if (engine.joinable()) {
        engine.join();
    }

    if (logging.joinable()) {
        logging.join();
    }
    if (io.joinable()) {
        io.join();
    }

    return 0;
}
