interface Parameters {
    GRID_COLUMNS: number;
    GRID_ROWS: number;
    LED_BASE_TONE: number;
    LED_SIZE_HORIZONTAL: number;
    LED_SIZE_VERTICAL: number;
    PITCH_HORIZONTAL: number;
    PITCH_VERTICAL: number;
}

const IkeaKortlinjeParameters: Parameters = {
    GRID_COLUMNS: 42, // px
    GRID_ROWS: 9, // px
    LED_BASE_TONE: 2 ** 3,
    LED_SIZE_HORIZONTAL: 2.54, // mm
    LED_SIZE_VERTICAL: 2.54, // mm
    PITCH_HORIZONTAL: 3.3, // mm
    PITCH_VERTICAL: 3.3, // mm
};

export const Device: Parameters = IkeaKortlinjeParameters
