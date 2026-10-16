import { mdiClockDigital, mdiImageFrame } from "@mdi/js";
import { type Component, createSignal, For, Match, Switch } from "solid-js";

import { Icon } from "../components/Icon";
import {
    MODE_DRAW,
} from "../config/modules";
import {
    Main as ExtensionWebUIMain,
    SidebarSection,
    SidebarSectionSecondary,
    MainSecondary as WebUIMain,
} from "../extensions/WebUI";
import { WebSocketWS } from "../extensions/WebSocket";
import { Main as ModeDrawMain, name as ModeDrawName, Sidebar as ModeDrawSidebar } from "../modes/Draw";
import { name as ModeTickerName, Sidebar as ModeTickerSidebar } from "../modes/Ticker";
import { DisplayPowerSet } from "./Display";

export const name = "Modes";

const [getMode, setMode] = createSignal<string>("");
const [getModes, setModes] = createSignal<string[]>([]);

export const ModesList = getModes;
export const ModesMode = getMode;

export const receiver = (json: { list?: string[]; mode?: string }) => {
    json?.list !== undefined && setModes(json.list);
    json?.mode !== undefined && setMode(json.mode);
};

export const Main: Component = () => (
    <Switch fallback={<ExtensionWebUIMain />}>
        {MODE_DRAW && (
            <Match when={getMode() === ModeDrawName}>
                <ModeDrawMain />
            </Match>
        )}
    </Switch>
);

export const Sidebar: Component = () => {
    const handleMode = (mode: string) => {
        setMode(mode);
        WebSocketWS.send(
            JSON.stringify({
                [name]: {
                    mode: mode,
                },
            }),
        );
        DisplayPowerSet(true);
    };

    return (
        <>
            <SidebarSection title="Mode">
                <div class="action grid-cols-[--spacing(4)_1fr]">
                    <Icon path={mdiClockDigital} />
                    <select
                        class="w-full"
                        name={name}
                        onchange={(e) => handleMode(e.currentTarget.value)}
                        value={getMode()}
                    >
                        <For each={getModes()}>{(mode) => <option>{mode}</option>}</For>
                    </select>
                </div>
            </SidebarSection>
            <Switch>
                {MODE_DRAW && (
                    <Match when={getMode() === ModeDrawName}>
                        <ModeDrawSidebar />
                    </Match>
                )}
                <Match when={getMode() === ModeTickerName}>
                    <ModeTickerSidebar />
                </Match>
            </Switch>
        </>
    );
};

export const SidebarSecondary: Component = () => (
    <SidebarSectionSecondary title={name}>
        <div class="space-y-3">
            <a
                class="link"
                href={`#/${name.toLowerCase()}`}
            >
                <Icon
                    class="mr-2"
                    path={mdiImageFrame}
                />
                More {name.toLowerCase()}...
            </a>
        </div>
    </SidebarSectionSecondary>
);

export const MainThird: Component = () => (
    <WebUIMain />
);

export const SidebarThird: Component = () => (
    <SidebarSectionSecondary title={name}><></>
    </SidebarSectionSecondary>
);

export const DescriptionComponent: Component<{
    icon?: string;
    internal?: boolean;
    text?: string;
}> = (props) => (
    <div class="main">
        <h1>
            <Icon path={props.icon || mdiClockDigital} />
        </h1>
        <a
            class="mode-title"
            href={`${props.internal ? `#/${name.toLowerCase()}/` : `https://github.com/VIPnytt/Kortlinje/wiki/${name}#-`}${getMode().toLowerCase().replace(/\s+/g, "-")}`}
            target={props.internal ? "_self" : "_blank"}
        >
            {getMode()}
        </a>
        {props.text && <div class="mode-subtitle">{props.text}</div>}
    </div>
);

export const MainComponent: Component<{
    icon?: string;
    internal?: boolean;
    text?: string;
}> = (props) => (
    <div class="main">
        <h1>
            <Icon path={props.icon || mdiClockDigital} />
        </h1>
        <a
            class="mode-title"
            href={`${props.internal ? `#/${name.toLowerCase()}/` : `https://github.com/VIPnytt/Kortlinje/wiki/${name}#-`}${getMode().toLowerCase().replace(/\s+/g, "-")}`}
            target={props.internal ? "_self" : "_blank"}
        >
            {getMode()}
        </a>
        {props.text && <div class="mode-subtitle">{props.text}</div>}
    </div>
);
