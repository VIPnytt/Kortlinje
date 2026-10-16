import {
    mdiCheckboxBlankCircleOutline,
    mdiCheckboxMarkedCircleOutline,
    mdiFormatFont,
    mdiProgressClock,
} from "@mdi/js";
import { type Component, createSignal, For } from "solid-js";

import { Icon } from "../components/Icon";
import { SidebarSection } from "../extensions/WebUI";
import { WebSocketWS } from "../extensions/WebSocket";

export const name = "Clock";

const [getFont, setFont] = createSignal<string>("");
const [getFonts, setFonts] = createSignal<string[]>([]);
const [getTicking, setTicking] = createSignal<boolean>(false);

export const receiver = (json: { font?: string; fonts?: string[]; ticking?: boolean }) => {
    json?.font !== undefined && setFont(json.font);
    json?.fonts !== undefined && setFonts(json.fonts);
    json?.ticking !== undefined && setTicking(json.ticking);
};

export const Sidebar: Component = () => {
    const handleFont = (font: string) => {
        setFont(font);
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    font: getFont(),
                },
            }),
        );
    };

    const handleTicking = () => {
        setTicking(!getTicking());
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    ticking: getTicking(),
                },
            }),
        );
    };

    return (
        <SidebarSection>
            <div class="action grid-cols-[--spacing(4)_1fr]">
                <Icon path={mdiFormatFont} />
                <select
                    class="w-full"
                    onchange={(e) => handleFont(e.currentTarget.value)}
                    value={getFont()}
                >
                    <For each={getFonts()}>{(font) => <option>{font}</option>}</For>
                </select>
            </div>
            <div class="action grid-cols-[--spacing(4)_1fr_--spacing(12)]">
                <Icon path={mdiProgressClock} />
                Second indicator
                <button
                    class={`w-full ${getTicking() ? "action-negative" : "action-deactivated"}`}
                    onclick={handleTicking}
                    type="button"
                >
                    <Icon path={getTicking() ? mdiCheckboxMarkedCircleOutline : mdiCheckboxBlankCircleOutline} />
                </button>
            </div>
        </SidebarSection>
    );
};
