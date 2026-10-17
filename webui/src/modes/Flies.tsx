import Cookies from "js-cookie";
import { type Component, createSignal } from "solid-js";

import { Canvas } from "../components/Canvas";
import { Device } from "../config/devices";
import { WebSocketWS } from "../extensions/WebSocket";

export const name = "Flies";

const [getFrame, setFrame] = createSignal<boolean[]>();

export const Main: Component = () => {
    const _id: number = parseInt(Cookies.get(`${name}.id`) ?? Math.round(Math.random() * (2 ** 8 - 1)).toString(), 10);
    Cookies.set(`${name}.id`, _id.toString(), {
        expires: 1,
    });

    const handleFrame = (data: boolean[]) => {
        setFrame([
            ...data,
        ]);
    };

    const handlePixel = (_x: number, _y: number, _lit: boolean) => {
        setFrame(new Array(Device.GRID_COLUMNS * Device.GRID_ROWS).fill(0));
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    id: _id,
                    x: _x,
                    y: _y,
                },
            }),
        );
    };

    return (
        <div class="bg-contrast-light dark:bg-contrast-dark main">
            <div>
                <Canvas
                    onFrame={handleFrame}
                    onPixel={handlePixel}
                    pixels={getFrame() || new Array(Device.GRID_COLUMNS * Device.GRID_ROWS).fill(0)}
                />
            </div>
        </div>
    );
};
