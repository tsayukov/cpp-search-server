#ifndef SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_
#define SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_

#include <map>
#include <mutex>
#include <vector>

namespace search_server::details::concurrent {

template <typename Key, typename Value>
class Map {
private:
    static_assert(std::is_integral_v<Key>,
                  "search_server::details::concurrent::Map supports only integer keys");

    using UnderlyingMap = std::map<Key, Value>;

    struct Bucket {
        std::mutex mutex;
        UnderlyingMap map;
    };

    std::vector<Bucket> buckets_;

public:

    using key_type = Key;
    using mapped_type = Value;

    // TODO: make private?
    class Access {
        std::lock_guard<std::mutex> guard_;
        Value& ref_to_value_;

    public:
        Access(const Key& key, Bucket& bucket)
                : guard_(bucket.mutex)
                , ref_to_value_(bucket.map[key]) {
        }

        operator Value&() {
            return ref_to_value_;
        }
    };

    explicit Map(size_t bucket_count)
            : buckets_(bucket_count) {}

    Access operator[](const Key& key) {
        auto& bucket = GetBucket(key);
        return {key, bucket};
    }

    void erase(const Key& key) {
        auto& bucket = GetBucket(key);
        std::lock_guard guard(bucket.mutex);
        (void) bucket.map.erase(key);
    }

    // TODO: replace to cast
    UnderlyingMap BuildOrdinaryMap() {
        UnderlyingMap result;
        for (auto& [mutex, map] : buckets_) {
            std::lock_guard guard(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

private: // Bucket access

    Bucket& GetBucket(const Key& key) {
        return buckets_[static_cast<std::size_t>(key) % buckets_.size()];
    }
};

} // namespace search_server::details::concurrent

#endif // SEARCH_SERVER_DETAILS_CONCURRENT_MAP_HPP_
