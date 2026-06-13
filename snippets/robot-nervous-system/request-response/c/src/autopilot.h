#ifndef IOX2_SNIPPETS_REQUEST_RESPONSE_AUTOPILOT_H
#define IOX2_SNIPPETS_REQUEST_RESPONSE_AUTOPILOT_H

// snippet:start request-type
struct Position {
    float position[2];
};
// snippet:end request-type

// snippet:start response-type
struct State {
    float position[2];
    float speed[2];
};
// snippet:end response-type

#endif
