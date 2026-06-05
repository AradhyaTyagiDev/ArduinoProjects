enum class RobotState
{
    Initializing,
    MovingForward,
    ObstacleDetected,
    Reversing,
    SearchingPath,
    VerifyPath,
    TurningLeftMove
    TurningRightMove,
    StuckRecovery
};

enum class MotionProfile
{
    FullSpeed,
    MediumSpeed,
    SlowSpeed,
    Crawl,
    Search,
    Emergency
};

struct ScheduledAction
{
    uint32_t startTime = 0;

    uint32_t durationMs = 0;

    bool active = false;
};



struct SearchStep
{
    bool left;

    uint8_t angle;
};

constexpr SearchStep SEARCH_PATTERN[] =
{
    { true,   5 },
    { false, 10 },
    { true,  15 },
    { false, 25 },
    { true,  40 },
    { false, 55 }
};