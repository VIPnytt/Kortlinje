import { mdiMirrorRectangle, mdiPlay, mdiStop } from "@mdi/js";
import { type Component, createSignal } from "solid-js";

import { Icon } from "../components/Icon";
import { Tooltip } from "../components/Tooltip";
import { name as StreamName } from "../modes/Stream";
import { name as ExtensionsName } from "../services/Extensions";
import { ModesMode } from "../services/Modes";
import { WebSocketWS } from "./WebSocket";

export const name = "Mirror";

const [getActive, setActive] = createSignal<boolean>(false);
const [getPort, setPort] = createSignal<number>(4048);

export const receiver = (json: { active?: boolean }) => {
    json?.active !== undefined && setActive(json.active);
};

const handleActive = () => {
    setActive(!getActive());
    WebSocketWS.send(
        JSON.stringify({
            [name]: {
                active: getActive(),
            },
        }),
    );
};

export const Actions: Component = () => (
    <div class="action">
        <a href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}>
            <Icon
                class="mr-2"
                path={mdiMirrorRectangle}
            />
            {name}
        </a>
        <Tooltip text={`${getActive() ? "Stop" : "Start"} ${name.toLowerCase()}ing`}>
            <button
                class={`w-full ${getActive() ? "action-activated" : "action-deactivated"}`}
                disabled={!getActive() && ModesMode() === StreamName}
                onclick={handleActive}
                type="button"
            >
                <Icon path={getActive() ? mdiStop : mdiPlay} />
            </button>
        </Tooltip>
    </div>
);

export const Link: Component = () => (
    <a
        class="link"
        href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}
    >
        <Icon
            class="mr-2"
            path={mdiMirrorRectangle}
        />
        {name}
    </a>
);

export const MainThird: Component = () => {
    const handlePort = (port: number) => {
        setPort(port);
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    port: getPort(),
                },
            }),
        );
    };
    return (
        <div class="main">
            <div class="space-y-3 p-5">
                <h2>{name}</h2>
                <div class="box">
                    <div class="space-y-3">
                        <div class="text-sm">Duplicate the display to other devices.</div>
                        <h3>Protocol</h3>
                        <select
                            class="w-full"
                            value={getPort()}
                            onchange={(e) => handlePort(parseInt(e.currentTarget.value, 10))}
                        >
                            <option value="6454">Art-Net</option>
                            <option value="4048">Distributed Display Protocol</option>
                            <option value="5568">E1.31</option>
                        </select>
                        <button
                            class={`mt-3 w-full ${getActive() ? "action-activated" : "action-deactivated"}`}
                            disabled={!getActive() && ModesMode() === StreamName}
                            onclick={handleActive}
                            type="button"
                        >
                            <Icon
                                class="mr-2"
                                path={getActive() ? mdiStop : mdiPlay}
                            />
                            {getActive() ? `${name.toLowerCase()}ing` : name.toLowerCase()}
                        </button>
                    </div>
                </div>
            </div>
        </div>
    );
};
