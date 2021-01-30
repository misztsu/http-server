#ifndef SHAREDMAP_H
#define SHAREDMAP_H

#include <unordered_map>
#include <mutex>
#include <shared_mutex>

template<typename K, typename V, typename MapType = std::unordered_map<K, V>>
class SharedMap
{
public:
    SharedMap() = default;

    SharedMap(SharedMap& other)
    {
        std::shared_lock lock(other.mutex);
        map = other.map;
    }

    SharedMap(SharedMap&& other) : map(std::move(other.map)) {}

    std::pair<typename MapType::iterator, bool> insert(const std::pair<K, V> &val)
    {
        std::unique_lock lock(mutex);
        return map.insert(val);
    }

    template<typename... T>
    std::pair<typename MapType::iterator, bool> emplace(T&&... args)
    {
        std::unique_lock lock(mutex);
        return map.emplace(std::forward<T>(args)...);
    }

    bool hasKey(const K &key) const
    {
        std::shared_lock lock(mutex);
        return map.find(key) != map.end();
    }

    V get(const K key)
    {
        std::shared_lock lock(mutex);
        return map.at(key);
    }
    V &at(const K &key)
    {
        std::shared_lock lock(mutex);
        return map.at(key);
    }
    const V &at(const K &key) const
    {
        std::shared_lock lock(mutex);
        return map.at(key);
    }

    bool erase(const K &key)
    {
        std::unique_lock lock(mutex);
        return map.erase(key) > 0;
    }

    void clear()
    {
        std::unique_lock lock(mutex);
        map.clear();
    }

    template<typename I>
    class iterator
    {
    public:
        iterator() = delete;
        iterator(const iterator&) = default;
        iterator(iterator&&) = default;
        const I &operator->() { return i; } 
        bool operator!=(const iterator<I> &other) { return i != other.i; }
        iterator<I> &operator++() { ++i; return *this; }
        auto &operator*() { return *i; }
    private:
        using Lock = std::unique_lock<std::shared_mutex>;
        I i;
        friend class SharedMap;
        iterator(I&& i, std::shared_mutex& mutex) :
                i(std::move(i)),
                lock(new Lock(mutex)) {}
        iterator(I&& i) : i(std::move(i)) {}
        std::shared_ptr<Lock> lock;
    };

    iterator<typename MapType::iterator> begin()
    {
        return iterator<typename MapType::iterator>(map.begin(), mutex);
    }
    iterator<typename MapType::iterator> beginNoLock()
    {
        return iterator<typename MapType::iterator>(map.begin());
    }
    iterator<typename MapType::const_iterator> begin() const
    {
        return iterator<typename MapType::const_iterator>(map.begin(), mutex);
    }
    iterator<typename MapType::const_iterator> beginNoLock() const
    {
        return iterator<typename MapType::const_iterator>(map.begin());
    }

    iterator<typename MapType::iterator> end()
    {
        return iterator<typename MapType::iterator>(map.end());
    }
    iterator<typename MapType::iterator> endLock()
    {
        return iterator<typename MapType::iterator>(map.begin(), new std::unique_lock(mutex));
    }
    iterator<typename MapType::const_iterator> end() const
    {
        return iterator<typename MapType::const_iterator>(map.end());
    }
    iterator<typename MapType::const_iterator> endLock() const
    {
        return iterator<typename MapType::const_iterator>(map.end(), new std::unique_lock(mutex));
    }

private:
    MapType map;
    mutable std::shared_mutex mutex;
};


#endif /* SHAREDMAP_H */