#ifndef IOX2_SNIPPETS_PUBLISH_SUBSCRIBE_DISTANCE_HPP
#define IOX2_SNIPPETS_PUBLISH_SUBSCRIBE_DISTANCE_HPP

#include <ostream>

// snippet:start payload
struct Distance {
    double distance_in_meters;
    float some_other_property;
};

inline auto operator<<(std::ostream& stream, const Distance& value) -> std::ostream& {
    stream << "Distance { distance_in_meters: " << value.distance_in_meters;
    stream << ", some_other_property: " << value.some_other_property << " }";
    return stream;
}
// snippet:end payload

#endif
