#ifndef SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_
#define SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_

#include <mutex>
#include <unordered_map>
#include <vector>

namespace search_server::details {

// clang-format off
template
        < typename Key
        , typename Value
        , typename Hash = std::hash<Key>
        , typename KeyEqual = std::equal_to<Key>
        >
// clang-format on
class ConcurrentMap {
    using UnderlyingMap = std::unordered_map<Key, Value, Hash, KeyEqual>;

    struct Bucket {
        mutable std::mutex mutex;
        UnderlyingMap map;
    };

    class Access {
        std::unique_lock<std::mutex> mGuard;
        Value* mValuePtr;

    public:
        Access(const Key& key, Bucket& bucket)
                : mGuard(bucket.mutex)
                , mValuePtr(&bucket.map[key]) {}

        operator Value&() {
            return *mValuePtr;
        }
    };

    std::vector<Bucket> mBuckets;
    Hash mHasher;

public:

    using key_type = Key;
    using mapped_type = Value;
    using hasher = Hash;
    using key_equal = KeyEqual;

    explicit ConcurrentMap(size_t bucket_count, const Hash& hasher = Hash())
            : mBuckets(bucket_count)
            , mHasher(hasher) {}

    Access operator[](const Key& key) {
        auto& bucket = getBucket(key);
        return {key, bucket};
    }

    void erase(const Key& key) {
        auto& bucket = getBucket(key);
        std::unique_lock guard(bucket.mutex);
        (void)bucket.map.erase(key);
    }

    UnderlyingMap buildOrdinaryMap() const {
        UnderlyingMap result;
        for (auto& [mutex, map] : mBuckets) {
            std::unique_lock guard(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

private: // Bucket access

    Bucket& getBucket(const Key& key) {
        return mBuckets[mHasher(key) % mBuckets.size()];
    }
};

} // namespace search_server::details

#endif // SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_
