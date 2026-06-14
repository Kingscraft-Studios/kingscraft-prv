#include "Bus/MessageBus.hpp"
#include "Threads/Engine.hpp"

int main() {
    MessageBus::Init();
    lve::Engine::Init();
    lve::Engine::Get().run();
    lve::Engine::Shutdown();
    MessageBus::Shutdown();
    return 0;
}
