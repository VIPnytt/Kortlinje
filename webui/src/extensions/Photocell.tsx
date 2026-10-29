import { mdiBrightnessAuto, mdiBrightness6 } from "@mdi/js";
import { type Component, createSignal } from "solid-js";

import { Icon } from "../components/Icon";
import { Tooltip } from "../components/Tooltip";
import { name as ExtensionsName } from "../services/Extensions";
import { WebSocketWS } from "./WebSocket";

export const name = "Photocell";

const [getEnabled, setEnabled] = createSignal<boolean>(false);

export const receiver = (json: { enabled?: boolean }) => {
    json?.enabled !== undefined && setEnabled(json.enabled);
};

const handleEnabled = () => {
    setEnabled(!getEnabled());
    WebSocketWS.send(
        JSON.stringify({
            [name]: {
                enabled: getEnabled(),
            },
        }),
    );
};

export const Actions: Component = () => (
    <div class="action">
        <a href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}>
            <Icon
                class="mr-2"
                path={mdiBrightnessAuto}
            />
            {name}
        </a>
        <Tooltip text={`${getEnabled() ? "Disable" : "Enable"} ${name.toLowerCase()}`}>
            <button
                class={`w-full ${getEnabled() ? "action-activated" : "action-deactivated"}`}
                onclick={handleEnabled}
                type="button"
            >
                <Icon path={getEnabled() ? mdiBrightnessAuto : mdiBrightness6} />
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
            path={mdiBrightnessAuto}
        />
        {name}
    </a>
);

export const MainThird: Component = () => (
    <div class="main">
        <div class="space-y-3 p-5">
            <h2>{name}</h2>
            <div class="box">
                <div class="space-y-3">
                    <h3>Ambient light detection</h3>
                    <div class="text-sm">Automatically switches on/off the display.</div>
                    <button
                        class={`mt-3 w-full ${getEnabled() ? "action-activated" : "action-deactivated"}`}
                        onclick={handleEnabled}
                        type="button"
                    >
                        <Icon
                            class="mr-2"
                            path={getEnabled() ? mdiBrightnessAuto : mdiBrightness6}
                        />
                        {getEnabled() ? "enabled" : "disabled"}
                    </button>
                </div>
            </div>
        </div>
    </div>
);
