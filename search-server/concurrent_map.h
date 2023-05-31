#pragma once

#include <map>
#include <vector>
#include <mutex>

template<typename Key, typename Value>
class ConcurrentMap {
private:
    using Map = std::map<Key, Value>;

    struct Bucket {
        std::mutex mutex;
        Map map;
    };

public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;

        Access(const Key& key, Bucket& bucket)
                : guard(bucket.mutex)
                , ref_to_value(bucket.map[key]) {
        }
    };

    explicit ConcurrentMap(size_t bucket_count)
            : buckets_(bucket_count) {
    }

    Access operator[](const Key& key) {
        auto& bucket = buckets_[static_cast<std::size_t>(key) % buckets_.size()];
        return {key, bucket};
    }

    Map BuildOrdinaryMap() {
        Map result;
        for (auto& [mutex, map] : buckets_) {
            std::lock_guard guard(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

private:
    std::vector<Bucket> buckets_;
};