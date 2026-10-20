import {
    mdiPower,
    mdiSleep,
} from "@mdi/js";
import { type Component, createSignal } from "solid-js";

import { Icon } from "../components/Icon";
import { Tooltip } from "../components/Tooltip";
import { SidebarSection } from "../extensions/WebUI";
import { WebSocketWS } from "../extensions/WebSocket";

export const name = "Display";

const [getBrightness, setBrightness] = createSignal<number>(0);
const [getPower, setPower] = createSignal<boolean>(false);

export const DisplayPower = getPower;
export const DisplayPowerSet = setPower;

export const receiver = (json: { brightness?: number; power?: boolean }) => {
    json?.brightness !== undefined && setBrightness(json.brightness);
    json?.power !== undefined && setPower(json.power);
};

export const Main: Component = () => (
    <div class="main">
        <div class="text-center">
            <h1>
                <Icon path={mdiSleep} />
            </h1>
            <div class="mode-title">Sleeping</div>
        </div>
    </div>
);

export const Sidebar: Component = () => {
    const handleBrightness = (value: number, send: boolean = true) => {
        setBrightness(value);
        if (send) {
            WebSocketWS.send(
                JSON.stringify({
                    [name]: {
                        brightness: getBrightness(),
                    },
                }),
            );
        }
    };

    const handlePower = () => {
        setPower(!getPower());
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    power: getPower(),
                },
            }),
        );
    };

    return (
        <SidebarSection title={name}>
            <div class="space-y-3">
                <div class="action grid-cols-[--spacing(12)_1fr]">
                    <button
                        class={`text-interactive-light dark:text-content-dark ${getPower() ? "bg-positive-light dark:bg-positive-dark hover:bg-negative-alt-light dark:hover:bg-negative-alt-dark" : "bg-negative-light dark:bg-negative-dark hover:bg-positive-alt-light dark:hover:bg-positive-alt-dark"}`}
                        onclick={handlePower}
                        type="button"
                    >
                        <Icon path={mdiPower} />
                    </button>
                    <Tooltip text={`Brightness ${Math.floor((([1, 2, 4, 10, 11, 12, 13, 14][getBrightness() - 1] - 1) / 13) * 99) + 1} %`}>
                        <input
                            class="w-full"
                            max="8"
                            min="1"
                            onInput={(e) => handleBrightness(e.currentTarget.valueAsNumber, false)}
                            onKeyUp={() => handleBrightness(getBrightness())}
                            onPointerUp={() => handleBrightness(getBrightness())}
                            type="range"
                            value={getBrightness()}
                        />
                    </Tooltip>
                </div>
            </div>
        </SidebarSection>
    );
};
