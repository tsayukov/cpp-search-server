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

    using key_type = Key;
    using mapped_type = Value;

    struct Access {
        Access(const Key& key, Bucket& bucket)
                : guard(bucket.mutex)
                , ref_to_value(bucket.map[key]) {
        }

        operator Value&() {
            return ref_to_value;
        }

    private:
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count)
            : buckets_(bucket_count) {
    }

    Access operator[](const Key& key) {
        auto& bucket = GetBucket(key);
        return {key, bucket};
    }

    void erase(const Key& key) {
        auto& bucket = GetBucket(key);
        std::lock_guard guard(bucket.mutex);
        (void) bucket.map.erase(key);
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

    Bucket& GetBucket(const Key& key) {
        return buckets_[static_cast<std::size_t>(key) % buckets_.size()];
    }
};