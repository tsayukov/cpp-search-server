#pragma once

inline constexpr auto default_inserter = [](auto& receiver, const auto& value) {
    receiver.insert(receiver.end(), value);
};

template<typename Receiver, typename Source,
         typename Predicate, typename Inserter = decltype(default_inserter)>
Receiver Filter(const Source& source,
                Predicate predicate,
                Inserter inserter = default_inserter,
                Receiver default_ctor = Receiver()) {
    Receiver receiver = default_ctor;
    for (const auto& element : source) {
        if (predicate(element)) {
            inserter(receiver, element);
        }
    }
    return receiver;
}