import { mdiWeb } from "@mdi/js";
import { type Component, createSignal, For } from "solid-js";

import { Icon } from "../components/Icon";
import { SidebarSection } from "../extensions/WebUI";
import { WebSocketWS } from "../extensions/WebSocket";

export const name = "Weather";

const [getProvider, setProvider] = createSignal<string>("");
const [getProviders, setProviders] = createSignal<string[]>([]);

export const receiver = (json: { provider?: string; providers?: string[]; temperature?: number }) => {
    json?.provider !== undefined && setProvider(json.provider);
    json?.providers !== undefined && setProviders(json.providers);
};

export const Sidebar: Component = () => {
    const handleProvider = (provider: string) => {
        setProvider(provider);
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    provider: getProvider(),
                },
            }),
        );
    };

    return (
        <SidebarSection title="Provider">
            <div class="action grid-cols-[--spacing(4)_1fr]">
                <Icon path={mdiWeb} />
                <select
                    class="w-full"
                    value={getProvider()}
                    onchange={(e) => handleProvider(e.currentTarget.value)}
                >
                    <For each={getProviders()}>{(provider) => <option>{provider}</option>}</For>
                </select>
            </div>
        </SidebarSection>
    );
};
