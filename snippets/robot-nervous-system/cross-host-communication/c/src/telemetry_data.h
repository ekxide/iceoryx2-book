#ifndef IOX2_SNIPPETS_TUNNEL_TELEMETRY_DATA_H
#define IOX2_SNIPPETS_TUNNEL_TELEMETRY_DATA_H

// snippet:start
// shared type name; must match across languages
#define BATTERY_STATE_TYPE_NAME "BatteryState"
struct BatteryState {
    float charge_percent;
};

// shared type name; must match across languages
#define POSITION_TYPE_NAME "Position"
struct Position {
    float x;
    float y;
};
// snippet:end

#endif
