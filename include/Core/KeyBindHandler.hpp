#pragma once

#include <GLFW/glfw3.h>

#include <array>
#include <functional>
#include <vector>
#include <algorithm>

namespace lve {

    class KeyBindHandler {
    public:
        using Callback = std::function<void()>;
        using Chord = std::vector<int>;
        using NoesisKeyCallback = std::function<void(int key, int action)>;
        using NoesisCharCallback = std::function<void(unsigned int codepoint)>;

        void setWindow(GLFWwindow* window) { window_ = window; }

        void onKeyEvent(int key, int scancode, int action, int mods) {
            if (key < 0 || key > GLFW_KEY_LAST) return;
            currKeys_[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
            if (noesisKeyCallback_)
                noesisKeyCallback_(key, action);
        }

        void onChar(unsigned int codepoint) {
            if (noesisCharCallback_)
                noesisCharCallback_(codepoint);
        }

        void update() {
            for (size_t i = 0; i < bindings_.size(); i++) {
                bool now = isSatisfied(i) && !isSuppressed(i);
                bool was = prevSatisfied_[i];

                if (now && !was && bindings_[i].onPress)
                    bindings_[i].onPress();
                if (!now && was && bindings_[i].onRelease)
                    bindings_[i].onRelease();

                prevSatisfied_[i] = now;
            }

            prevKeys_ = currKeys_;
        }

        bool isDown(int key) const { return currKeys_[key]; }
        bool isPressed(int key) const { return currKeys_[key] && !prevKeys_[key]; }
        bool isReleased(int key) const { return !currKeys_[key] && prevKeys_[key]; }

        void onPress(Chord chord, Callback cb) {
            bindings_.push_back({std::move(chord), std::move(cb), {}});
            prevSatisfied_.push_back(false);
        }

        void onRelease(Chord chord, Callback cb) {
            bindings_.push_back({std::move(chord), {}, std::move(cb)});
            prevSatisfied_.push_back(false);
        }

        void setNoesisKeyCallback(NoesisKeyCallback cb) { noesisKeyCallback_ = std::move(cb); }
        void setNoesisCharCallback(NoesisCharCallback cb) { noesisCharCallback_ = std::move(cb); }

        void clear() { bindings_.clear(); prevSatisfied_.clear(); }

    private:
        struct Binding {
            Chord chord;
            Callback onPress;
            Callback onRelease;
        };

        bool isSatisfied(size_t idx) const {
            for (int k : bindings_[idx].chord)
                if (!currKeys_[k]) return false;
            return true;
        }

        bool isSuppressed(size_t idx) const {
            const auto& chord = bindings_[idx].chord;
            for (size_t j = 0; j < bindings_.size(); j++) {
                if (j == idx) continue;
                const auto& other = bindings_[j].chord;
                if (other.size() <= chord.size()) continue;

                bool otherSatisfied = true;
                for (int k : other)
                    if (!currKeys_[k]) { otherSatisfied = false; break; }
                if (!otherSatisfied) continue;

                bool isSubset = true;
                for (int k : chord)
                    if (std::find(other.begin(), other.end(), k) == other.end())
                        { isSubset = false; break; }
                if (isSubset) return true;
            }
            return false;
        }

        GLFWwindow* window_ = nullptr;
        std::array<bool, GLFW_KEY_LAST + 1> prevKeys_{};
        std::array<bool, GLFW_KEY_LAST + 1> currKeys_{};
        std::vector<Binding> bindings_;
        std::vector<bool> prevSatisfied_;
        NoesisKeyCallback noesisKeyCallback_;
        NoesisCharCallback noesisCharCallback_;
    };

} // namespace lve
