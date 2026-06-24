#include "Core/World/World.hpp"
#include "Vulkan/Device.hpp"
#include "Core/Blocks/Blocks.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace lve {

    World::World(Device& device, int chunkSize, int height)
        : device_(device), chunkSize_(chunkSize), height_(height) {
        genThread_ = std::thread([this]() { genThreadFunc(); });
    }

    World::~World() {
        genRunning_ = false;
        genCV_.notify_all();
        if (genThread_.joinable()) genThread_.join();
        {
            std::lock_guard<std::mutex> lock(genMutex_);
            pendingGen_.clear();
            completedGen_.clear();
            pendingRequests_.clear();
        }
        for (auto& bucket : pendingCleanup_) {
            bucket.clear();
        }
    }

    uint64_t World::packKey(int gx, int gz) {
        return (static_cast<uint64_t>(static_cast<int64_t>(gx)) << 32) |
                (static_cast<uint64_t>(static_cast<int64_t>(gz)) & 0xFFFFFFFF);
    }

    int World::worldToGrid(float worldCoord, int chunkSize) {
        return static_cast<int>(std::floor(worldCoord / (chunkSize - 1)));
    }

    bool World::isChunkLoaded(int gridX, int gridZ) const {
        return chunks_.find(packKey(gridX, gridZ)) != chunks_.end();
    }

    void World::loadChunkSync(int gridX, int gridZ) {
        if (isChunkLoaded(gridX, gridZ)) return;

        int N = chunkSize_;
        int h = height_;
        uint8_t grassId = static_cast<uint8_t>(Blocks::GRASS_BLOCK.getId());

        std::vector<uint8_t> blockIds(static_cast<size_t>(N) * h * N, 0);

        float originX = static_cast<float>(gridX) * (N - 1);
        float originZ = static_cast<float>(gridZ) * (N - 1);

        for (int z = 0; z < N; ++z) {
            for (int x = 0; x < N; ++x) {
                float wx = originX + x;
                float wz = originZ + z;
                float surface = 8.0f + noise_.getHeight(wx, wz);
                int top = std::max(0, std::min(h - 1, static_cast<int>(surface)));

                blockIds[static_cast<size_t>(top) * N * N + z * N + x] = grassId;
            }
        }

        auto chunk = std::make_unique<Chunk>(device_, glm::ivec2(gridX, gridZ), N, 1.0f);
        ChunkMesher::generate(*chunk, blockIds, h);
        chunk->upload();

        chunks_[packKey(gridX, gridZ)] = std::move(chunk);
        chunkCacheDirty_ = true;
    }

    void World::unloadChunk(int gridX, int gridZ) {
        auto it = chunks_.find(packKey(gridX, gridZ));
        if (it == chunks_.end()) return;
        int idx = static_cast<int>(frameCount_ % pendingCleanup_.size());
        pendingCleanup_[idx].push_back(std::move(it->second));
        chunks_.erase(it);
        chunkCacheDirty_ = true;
    }

    void World::flushPendingCleanup() {
        if (frameCount_ >= CLEANUP_DELAY) {
            int clearIdx = static_cast<int>((frameCount_ - CLEANUP_DELAY) % pendingCleanup_.size());
            pendingCleanup_[clearIdx].clear();
        }
        ++frameCount_;
    }

    void World::genThreadFunc() {
        TerrainGenerator noise(1337);
        uint8_t grassId = static_cast<uint8_t>(Blocks::GRASS_BLOCK.getId());

        while (genRunning_) {
            std::pair<int, int> task;
            {
                std::unique_lock<std::mutex> lock(genMutex_);
                if (!genCV_.wait_for(lock, std::chrono::milliseconds(50),
                    [this]() { return !pendingGen_.empty() || !genRunning_; })) {
                    continue;
                }
                if (!genRunning_) break;
                task = pendingGen_.front();
                pendingGen_.pop_front();
            }

            int gx = task.first;
            int gz = task.second;
            int N = chunkSize_;
            int h = height_;

            std::vector<uint8_t> blockIds(static_cast<size_t>(N) * h * N, 0);
            float originX = static_cast<float>(gx) * (N - 1);
            float originZ = static_cast<float>(gz) * (N - 1);

            for (int z = 0; z < N; ++z) {
                for (int x = 0; x < N; ++x) {
                    float wx = originX + x;
                    float wz = originZ + z;
                    float surface = 8.0f + noise.getHeight(wx, wz);
                    int top = std::max(0, std::min(h - 1, static_cast<int>(surface)));
                    blockIds[static_cast<size_t>(top) * N * N + z * N + x] = grassId;
                }
            }

            Chunk tempChunk(device_, glm::ivec2(gx, gz), N, 1.0f);
            ChunkMesher::generate(tempChunk, blockIds, h);

            ChunkGenResult result;
            result.gx = gx;
            result.gz = gz;
            result.vertices = std::move(tempChunk.vertices());
            result.indices = std::move(tempChunk.indices());

            {
                std::lock_guard<std::mutex> lock(genMutex_);
                completedGen_.push_back(std::move(result));
            }
        }
    }

    void World::processCompletedChunks() {
        std::deque<ChunkGenResult> results;
        {
            std::lock_guard<std::mutex> lock(genMutex_);
            results.swap(completedGen_);
        }

        for (auto& r : results) {
            uint64_t key = packKey(r.gx, r.gz);
            {
                std::lock_guard<std::mutex> lock(genMutex_);
                pendingRequests_.erase(key);
            }

            if (chunks_.count(key)) continue;

            auto chunk = std::make_unique<Chunk>(device_, glm::ivec2(r.gx, r.gz), chunkSize_, 1.0f);
            chunk->vertices() = std::move(r.vertices);
            chunk->indices() = std::move(r.indices);
            chunk->upload();

            chunks_[key] = std::move(chunk);
            chunkCacheDirty_ = true;
        }
    }

    void World::update(float cameraX, float cameraZ, int renderDistance) {
        int centerX = worldToGrid(cameraX, chunkSize_);
        int centerZ = worldToGrid(cameraZ, chunkSize_);

        // Unload chunks outside render distance
        std::vector<uint64_t> toUnload;
        for (auto& [key, chunk] : chunks_) {
            int gx = static_cast<int>(key >> 32);
            int gz = static_cast<int>(key & 0xFFFFFFFF);
            if (std::abs(gx - centerX) > renderDistance ||
                std::abs(gz - centerZ) > renderDistance) {
                toUnload.push_back(key);
            }
        }
        {
            std::lock_guard<std::mutex> lock(genMutex_);
            for (uint64_t key : toUnload) {
                pendingRequests_.erase(key);
            }
        }
        for (uint64_t key : toUnload) {
            int gx = static_cast<int>(key >> 32);
            int gz = static_cast<int>(key & 0xFFFFFFFF);
            unloadChunk(gx, gz);
        }

        // Load center 3x3 chunks synchronously on first update
        if (chunks_.empty()) {
            for (int gz = centerZ - HIGH_PRIO_RADIUS; gz <= centerZ + HIGH_PRIO_RADIUS; ++gz) {
                for (int gx = centerX - HIGH_PRIO_RADIUS; gx <= centerX + HIGH_PRIO_RADIUS; ++gx) {
                    loadChunkSync(gx, gz);
                }
            }
        }

        // Queue async loads in expanding rings (closest to player first)
        int queued = 0;
        {
            std::lock_guard<std::mutex> lock(genMutex_);
            for (int r = HIGH_PRIO_RADIUS + 1; r <= renderDistance; ++r) {
                auto tryQueue = [&](int gx, int gz) {
                    uint64_t key = packKey(gx, gz);
                    if (chunks_.count(key)) return;
                    if (pendingRequests_.count(key)) return;
                    if (queued >= MAX_REQUESTS_PER_FRAME) return;

                    pendingGen_.push_back({gx, gz});
                    pendingRequests_.insert(key);
                    ++queued;
                };

                // top edge: (centerZ - r)
                for (int gx = centerX - r; gx <= centerX + r; ++gx)
                    tryQueue(gx, centerZ - r);

                // bottom edge: (centerZ + r)
                for (int gx = centerX - r; gx <= centerX + r; ++gx)
                    tryQueue(gx, centerZ + r);

                // left edge: (centerX - r), skip corners
                for (int gz = centerZ - r + 1; gz <= centerZ + r - 1; ++gz)
                    tryQueue(centerX - r, gz);

                // right edge: (centerX + r), skip corners
                for (int gz = centerZ - r + 1; gz <= centerZ + r - 1; ++gz)
                    tryQueue(centerX + r, gz);

                if (queued >= MAX_REQUESTS_PER_FRAME) break;
            }
        }
        if (queued > 0) genCV_.notify_one();
    }

    std::vector<Chunk*> World::getLoadedChunks() const {
        if (chunkCacheDirty_) {
            chunkCache_.clear();
            chunkCache_.reserve(chunks_.size());
            for (auto& [key, chunk] : chunks_) {
                chunkCache_.push_back(chunk.get());
            }
            chunkCacheDirty_ = false;
        }
        return chunkCache_;
    }

} // namespace lve
