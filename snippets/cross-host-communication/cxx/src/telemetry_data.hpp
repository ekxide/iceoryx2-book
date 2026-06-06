#ifndef IOX2_SNIPPETS_TUNNEL_TELEMETRY_DATA_HPP
#define IOX2_SNIPPETS_TUNNEL_TELEMETRY_DATA_HPP

// snippet:start
struct BatteryState {
    // shared type name; must match across languages
    static constexpr const char* IOX2_TYPE_NAME = "BatteryState";
    float charge_percent;
};

struct Position {
    // shared type name; must match across languages
    static constexpr const char* IOX2_TYPE_NAME = "Position";
    float x;
    float y;
};
// snippet:end

#endif
