type Geometry = "circular" | "rectangular";

interface Parameters {
    GRID_COLUMNS: number;
    GRID_ROWS: number;
    LED_BASE_BRIGHTNESS: number;
    LED_BASE_TONE: number;
    LED_GEOMETRY: Geometry;
    LED_SIZE_HORIZONTAL: number;
    LED_SIZE_VERTICAL: number;
    MANUFACTURER: string;
    MODEL: string;
    PITCH_HORIZONTAL: number;
    PITCH_VERTICAL: number;
}

const IkeaKortlinjeParameters: Parameters = {
    GRID_COLUMNS: 42, // px
    GRID_ROWS: 9, // px
    LED_BASE_BRIGHTNESS: 2 ** 6,
    LED_BASE_TONE: 2 ** 4,
    LED_GEOMETRY: "rectangular",
    LED_SIZE_HORIZONTAL: 3, // mm
    LED_SIZE_VERTICAL: 3, // mm
    MANUFACTURER: "IKEA",
    MODEL: "Kortlinje",
    PITCH_HORIZONTAL: 4, // mm
    PITCH_VERTICAL: 4, // mm
};

export const Device: Parameters = IkeaKortlinjeParameters
