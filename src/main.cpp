#include "Bus/MessageBus.hpp"
#include "Threads/Engine.hpp"

int main() {
    lve::MessageBus::Init();
    lve::Engine::Init();
    lve::Engine::get().run();
    lve::Engine::Shutdown();
    lve::MessageBus::Shutdown();
    return 0;
}
