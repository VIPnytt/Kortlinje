import { mdiDockLeft, mdiClockDigital, mdiHome, mdiMenu, mdiWindowClose } from "@mdi/js";
import { type Component, createEffect, createSignal, type JSX, Match, onCleanup, onMount, Switch } from "solid-js";

import { Canvas } from "../components/Canvas";
import { Icon } from "../components/Icon";
import { ToastProvider } from "../components/Toast";
import { Tooltip } from "../components/Tooltip";
import { HOSTNAME, NAME } from "../config/constants";
import { Device } from "../config/devices";
import { EXTENSION_OTA } from "../config/modules";
import { VERSION } from "../config/version";
import {
    name as ConnectivityName,
    SidebarSecondaryComponent as ConnectivitySecondarySidebarComponent,
    MainThird as ConnectivityThird,
    SidebarThird as ConnectivityThirdSidebar,
} from "../services/Connectivity";
import { SidebarSecondaryComponent as DeviceSecondarySidebarComponent, DeviceVersion } from "../services/Device";
import {
    Main as DisplayMain,
    DisplayPower,
    Sidebar as DisplaySidebar,
} from "../services/Display";
import {
    Footer as ExtensionsFooter,
    name as ExtensionsName,
    SidebarSecondary as ExtensionsSecondarySidebar,
    MainThird as ExtensionsThird,
    SidebarThird as ExtensionsThirdSidebar,
} from "../services/Extensions";
import {
    Main as ModesMain,
    name as ModesName,
    SidebarSecondary as ModesSecondarySidebar,
    Sidebar as ModesSidebar,
    MainThird as ModesThird,
    SidebarThird as ModesThirdSidebar,
} from "../services/Modes";
import { MainComponent as ExtensionOtaComponent } from "./Ota";
import { WebSocketmessages, WebSocketState, WebSocketWS } from "./WebSocket";

export const name = "Web UI";

const [getFrame, setFrame] = createSignal<number[]>();
const [getHash, setHash] = createSignal<string>(location.hash);
const [getSidebar, setSidebar] = createSignal<boolean>(true);

export const WebUISidebar = getSidebar;
export const WebUISidebarSet = setSidebar;

const onChange = () => setHash(location.hash);

createEffect(() => {
    if (!DisplayPower() && !getSidebar()) {
        setSidebar(true);
    }
});

export const receiver = (json: { frame?: number[] }) => {
    json?.frame !== undefined && setFrame(json.frame);
};

onMount(() => {
    addEventListener("hashchange", onChange, false);
});

onCleanup(() => {
    removeEventListener("hashchange", onChange, false);
});

export const WebUIPath = () => getHash().replace(/^#/, "") || "/";

export const WebUI: Component = () => (
    <WebSocketmessages>
        <ToastProvider>
            <Switch
                fallback={
                    <>
                        {WebUIPath() === "/" ? <Primary /> : <Secondary />}
                        {(DisplayPower() || !getSidebar()) && <Toggle />}
                    </>
                }
            >
                <Match when={WebSocketState() !== WebSocketWS.OPEN}>
                    <MainSecondary />
                </Match>
                <Match when={VERSION !== DeviceVersion()}>
                    <Third />
                </Match>
            </Switch>
        </ToastProvider>
    </WebSocketmessages>
);

const Primary: Component = () => (
    <Layout
        main={
            <Switch fallback={<ModesMain />}>
                <Match when={!DisplayPower()}>
                    <DisplayMain />
                </Match>
            </Switch>
        }
        sidebar={
            <>
                <DisplaySidebar />
                <ModesSidebar />
                <Home />
            </>
        }
    />
);

const Secondary: Component = () => (
    <Layout
        main={
            <Switch fallback={<MainSecondary />}>
                <Match when={WebUIPath().startsWith(`/${ConnectivityName.toLowerCase()}`)}>
                    <ConnectivityThird />
                </Match>
                <Match when={WebUIPath().startsWith(`/${ExtensionsName.toLowerCase()}`)}>
                    <ExtensionsThird />
                </Match>
                <Match when={WebUIPath().startsWith(`/${ModesName.toLowerCase()}`)}>
                    <ModesThird />
                </Match>
            </Switch>
        }
        sidebar={
            <>
                <Switch
                    fallback={
                        <>
                            <SidebarSectionSecondary title={Device.MODEL}>
                                <div class="space-y-3">
                                    <ConnectivitySecondarySidebarComponent />
                                    <DeviceSecondarySidebarComponent />
                                </div>
                            </SidebarSectionSecondary>
                            <ExtensionsSecondarySidebar />
                            <ModesSecondarySidebar />
                        </>
                    }
                >
                    <Match when={WebUIPath().startsWith(`/${ConnectivityName.toLowerCase()}`)}>
                        <ConnectivityThirdSidebar />
                    </Match>
                    <Match when={WebUIPath().startsWith(`/${ExtensionsName.toLowerCase()}`)}>
                        <ExtensionsThirdSidebar />
                    </Match>
                    <Match when={WebUIPath().startsWith(`/${ModesName.toLowerCase()}`)}>
                        <ModesThirdSidebar />
                    </Match>
                </Switch>
                <Home />
            </>
        }
    />
);

export const Third: Component = () => (
    <div class="main">
        <div class="space-y-3 p-5">
            <h2>Kortlinje</h2>
            <div class="box">
                <div class="space-y-3">
                    <h3>Version conflict</h3>
                    <div class="text-sm">
                        Firmware: {DeviceVersion()}
                        <br />
                        Filesystem: {VERSION}
                    </div>
                    <div class="text-sm">Flash or upload matching versions to restore compatibility.</div>
                    <div class="hr" />
                    <div class="hover:underline text-sm tracking-wide">
                        <a
                            href="https://github.com/VIPnytt/Kortlinje/releases/latest"
                            rel="noopener"
                            target="_blank"
                        >
                            Download latest stable release
                        </a>
                    </div>
                </div>
            </div>
        </div>
        {EXTENSION_OTA && <ExtensionOtaComponent />}
    </div>
);

export const Main: Component = () => (
    <div class="bg-contrast-light dark:bg-contrast-dark main">
        <div class="shrink-0 snap-center">
            <Canvas
                disabled={true}
                pixels={getFrame() || new Array(Device.GRID_COLUMNS * Device.GRID_ROWS).fill(0)}
            />
        </div>
    </div>
);

export const MainSecondary: Component = () => (
    <div class="main">
        <h1>
            <Icon path={mdiClockDigital} />
        </h1>
        <div class="mode-title">{NAME}</div>
    </div>
);

const Home: Component = () => (
    <>
        {WebUIPath() === "/" ? <ExtensionsFooter /> : <Footer />}
        <div class="hr mt-3 mx-0 pt-3">
            <a
                class="link"
                href={`#/${WebUIPath() === "/" ? "menu" : ""}`}
            >
                <Icon
                    class="mr-2"
                    path={WebUIPath() === "/" ? mdiMenu : mdiHome}
                />
                {WebUIPath() === "/" ? "Menu" : "Home"}
            </a>
        </div>
    </>
);

const Footer: Component = () => (
    <>
        <div class="hr mb-3 mt-auto" />
        <div class="link">
            <a
                class="flex-1"
                href="https://github.com/VIPnytt/Kortlinje/wiki"
                rel="noopener"
                target="_blank"
            >
                <Icon
                    class="mr-2"
                    path={mdiClockDigital}
                />
                Kortlinje
            </a>
            <a
                class="text-content-alt-light dark:text-content-alt-dark flex-none"
                href="https://github.com/VIPnytt/Kortlinje/releases"
                rel="noopener"
                target="_blank"
            >
                <Tooltip text="Release notes">{VERSION}</Tooltip>
            </a>
        </div>
    </>
);

const Toggle: Component = () => (
    <div class={`absolute ${getSidebar() ? "left-[calc(--spacing(68)-1px)] top-0" : "left-6 top-6"}`}>
        <button
            class={`text-interactive-light dark:text-content-dark ${getSidebar() ? "border-0 bg-menu-light dark:bg-menu-dark hover:bg-negative-alt-light dark:hover:bg-negative-alt-dark" : "bg-neutral-light dark:bg-neutral-dark hover:bg-positive-alt-light dark:hover:bg-positive-alt-dark"}`}
            onclick={() => setSidebar(!getSidebar())}
            type="button"
        >
            <Icon path={getSidebar() ? mdiWindowClose : mdiDockLeft} />
        </button>
    </div>
);

export const SidebarSection: Component<{
    title?: string;
    children: JSX.Element;
}> = (props) => (
    <>
        <div class="space-y-3">
            <h3 class="ml-3">{props.title || "Options"}</h3>
            <div class="space-y-3">{props.children}</div>
        </div>
        <div class="hr my-6" />
    </>
);

export const SidebarSectionSecondary: Component<{
    title: string;
    children: JSX.Element;
}> = (props) => (
    <>
        <div class="space-y-3">
            <h3 class="ml-3">{props.title}</h3>
            <div class="space-y-3">{props.children}</div>
        </div>
        <div class="hr my-6" />
    </>
);

const Layout: Component<{
    main: JSX.Element;
    sidebar: JSX.Element;
}> = (props) => (
    <div class={`h-full ${getSidebar() ? `grid grid-cols-[--spacing(80)_1fr]` : ""}`}>
        {getSidebar() && (
            <aside class="bg-menu-light dark:bg-menu-dark border-r flex flex-col h-full p-2 pt-4 rounded-none">
                {props.sidebar}
            </aside>
        )}
        <main class="h-full overflow-auto rounded-none">{props.main}</main>
    </div>
);

export const Canonical = () => {
    import.meta.env.PROD &&
        `${HOSTNAME}.local` !== location.hostname &&
        fetch(`http://${HOSTNAME}.local`, {
            method: "HEAD",
        }).then((response) => {
            if (response.status === 200) {
                WebSocketWS.close();
                location.href = `http://${HOSTNAME}.local`;
            }
        });
};
