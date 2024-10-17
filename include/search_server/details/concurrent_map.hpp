#ifndef SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_
#define SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_

#include <map>
#include <mutex>
#include <vector>

namespace search_server::details {

template <typename Key, typename Value>
class ConcurrentMap {
private:
    static_assert(std::is_integral_v<Key>,
                  "search_server::details::ConcurrentMap supports only integer keys");

    using UnderlyingMap = std::map<Key, Value>;

    struct Bucket {
        std::mutex mutex;
        UnderlyingMap map;
    };

    std::vector<Bucket> mBuckets;

public:

    using key_type = Key;
    using mapped_type = Value;

    // TODO: make private?
    class Access {
        std::lock_guard<std::mutex> mGuard;
        Value& mRefToValue;

    public:
        Access(const Key& key, Bucket& bucket)
                : mGuard(bucket.mutex)
                , mRefToValue(bucket.map[key]) {}

        operator Value&() {
            return mRefToValue;
        }
    };

    explicit ConcurrentMap(size_t bucket_count) : mBuckets(bucket_count) {}

    Access operator[](const Key& key) {
        auto& bucket = getBucket(key);
        return {key, bucket};
    }

    void erase(const Key& key) {
        auto& bucket = getBucket(key);
        std::lock_guard guard(bucket.mutex);
        (void)bucket.map.erase(key);
    }

    UnderlyingMap buildOrdinaryMap() {
        UnderlyingMap result;
        for (auto& [mutex, map] : mBuckets) {
            std::lock_guard guard(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

private: // Bucket access

    Bucket& getBucket(const Key& key) {
        return mBuckets[static_cast<std::size_t>(key) % mBuckets.size()];
    }
};

} // namespace search_server::details

#endif // SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_
