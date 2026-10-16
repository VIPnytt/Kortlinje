import { mdiCheckboxBlankCircleOutline, mdiCheckboxMarkedCircleOutline } from "@mdi/js";
import { type Component, createSignal } from "solid-js";

import { ClockIcon } from "../components/Clock";
import { Icon } from "../components/Icon";
import { SidebarSection } from "../extensions/WebUI";
import { WebSocketWS } from "../extensions/WebSocket";

export const name = "Snake";

const [getClock, setClock] = createSignal<boolean>(false);

export const receiver = (json: { clock?: boolean }) => {
    json?.clock !== undefined && setClock(json.clock);
};

export const Sidebar: Component = () => {
    const handleClock = () => {
        setClock(!getClock());
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    clock: getClock(),
                },
            }),
        );
    };

    return (
        <SidebarSection>
            <div class="action grid-cols-[--spacing(4)_1fr_--spacing(12)]">
                <Icon path={ClockIcon()} />
                Clock
                <button
                    class={`w-full ${getClock() ? "action-negative" : "action-deactivated"}`}
                    onclick={handleClock}
                    type="button"
                >
                    <Icon path={getClock() ? mdiCheckboxMarkedCircleOutline : mdiCheckboxBlankCircleOutline} />
                </button>
            </div>
        </SidebarSection>
    );
};
