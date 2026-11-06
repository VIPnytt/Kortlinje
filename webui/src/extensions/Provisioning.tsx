import { mdiWifi, mdiWifiOff } from "@mdi/js";
import { type Component, createSignal, Show } from "solid-js";

import { Icon } from "../components/Icon";
import { Toast } from "../components/Toast";
import { Tooltip } from "../components/Tooltip";
import { name as ExtensionsName } from "../services/Extensions";
import { WebSocketWS } from "./WebSocket";

export const name = "Provisioning";

const [getConfigured, setConfigured] = createSignal<boolean>(false);
const [getSsid, setSsid] = createSignal<string>("");
const [getClearing, setClearing] = createSignal<boolean>(false);

export const receiver = (json: { configured?: boolean; ssid?: string }) => {
    json?.configured !== undefined && setConfigured(json.configured);
    json?.ssid !== undefined && setSsid(json.ssid);
    if (json?.configured === false) {
        setClearing(false);
    }
};

const handleClear = () => {
    if (getClearing()) {
        return;
    }
    if (
        !confirm(
            getConfigured()
                ? `Forget WiFi “${getSsid() || "saved network"}”? The clock will reboot into setup mode.`
                : "No saved WiFi. Reboot into setup mode anyway?",
        )
    ) {
        return;
    }
    setClearing(true);
    Toast().toast("Clearing WiFi… rebooting");
    WebSocketWS.send(
        JSON.stringify({
            [name]: {
                clear: true,
            },
        }),
    );
};

export const Link: Component = () => (
    <Tooltip text="Saved WiFi network">
        <a
            class="link"
            href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}
        >
            <Icon
                class="mr-2"
                path={mdiWifi}
            />
            WiFi
        </a>
    </Tooltip>
);

export const MainThird: Component = () => (
    <div class="main">
        <div class="space-y-3 p-5">
            <h2>WiFi</h2>
            <div class="box">
                <div class="space-y-3">
                    <h3>Saved network</h3>
                    <div class="text-sm">
                        <Show
                            when={getConfigured()}
                            fallback="No WiFi credentials stored. Hold SET ~3s or reboot with empty creds for SoftAP setup."
                        >
                            Connected / stored as <span class="font-mono">{getSsid()}</span>
                        </Show>
                    </div>
                    <button
                        class="action-deactivated mt-3 w-full"
                        disabled={getClearing()}
                        onclick={handleClear}
                        type="button"
                    >
                        <Icon
                            class="mr-2"
                            path={mdiWifiOff}
                        />
                        {getClearing() ? "rebooting…" : "Forget WiFi"}
                    </button>
                </div>
            </div>
        </div>
    </div>
);
