import { mdiClockDigital, mdiImageFrame } from "@mdi/js";
import { type Component, createSignal, For, Match, Switch } from "solid-js";

import { Icon } from "../components/Icon";
import { WebSocketWS } from "../extensions/WebSocket";
import {
  Main as ExtensionWebUIMain,
  SidebarSection,
  SidebarSectionSecondary,
  MainSecondary as WebUIMain,
  WebUIPath,
} from "../extensions/WebUI";
import {
  Main as ModeAnimationMain,
  name as ModeAnimationName,
  Sidebar as ModeAnimationSidebar,
} from "../modes/Animation";
import {
  Actions as ModeCountdownActions,
  Link as ModeCountdownLink,
  Main as ModeCountdownMain,
  name as ModeCountdownName,
  Sidebar as ModeCountdownSidebar,
} from "../modes/Countdown";
import {
  Main as ModeDrawMain,
  name as ModeDrawName,
  Sidebar as ModeDrawSidebar,
} from "../modes/Draw";
import {
  Link as ModeTickerLink,
  Main as ModeTickerMain,
  name as ModeTickerName,
  Sidebar as ModeTickerSidebar,
} from "../modes/Ticker";
import { name as ModeWeatherName, Sidebar as ModeWeatherSidebar } from "../modes/Weather";
import { DisplayPowerSet } from "./Display";

export const name = "Modes";

const [getMode, setMode] = createSignal<string>("");
const [getModes, setModes] = createSignal<string[]>([]);

export const ModesCollection = getModes;
export const ModesMode = getMode;

export const receiver = (json: { collection?: string[]; mode?: string }) => {
  json?.collection !== undefined && setModes(json.collection);
  json?.mode !== undefined && setMode(json.mode);
};

export const Main: Component = () => (
  <Switch fallback={<ExtensionWebUIMain />}>
    <Match when={getMode() === ModeAnimationName}>
      <ModeAnimationMain />
    </Match>
    <Match when={getMode() === ModeDrawName}>
      <ModeDrawMain />
    </Match>
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
            onChange={(e) => handleMode(e.currentTarget.value)}
            value={getMode()}
          >
            <For each={getModes()}>{(mode) => <option>{mode}</option>}</For>
          </select>
        </div>
      </SidebarSection>
      <Switch>
        <Match when={getMode() === ModeAnimationName}>
          <ModeAnimationSidebar />
        </Match>
        <Match when={getMode() === ModeCountdownName}>
          <ModeCountdownSidebar />
        </Match>
        <Match when={getMode() === ModeDrawName}>
          <ModeDrawSidebar />
        </Match>
        <Match when={getMode() === ModeTickerName}>
          <ModeTickerSidebar />
        </Match>
        <Match when={getMode() === ModeWeatherName}>
          <ModeWeatherSidebar />
        </Match>
      </Switch>
    </>
  );
};

export const SidebarSecondary: Component = () => (
  <SidebarSectionSecondary title={name}>
    <div class="space-y-3">
      <ModeCountdownActions />
      <a class="link" href={`#/${name.toLowerCase()}`}>
        <Icon class="mr-2" path={mdiImageFrame} />
        More {name.toLowerCase()}...
      </a>
    </div>
  </SidebarSectionSecondary>
);

export const MainThird: Component = () => (
  <Switch fallback={<WebUIMain />}>
    <Match when={WebUIPath() === `/${name.toLowerCase()}/${ModeCountdownName.toLowerCase()}`}>
      <ModeCountdownMain />
    </Match>
    <Match when={WebUIPath() === `/${name.toLowerCase()}/${ModeTickerName.toLowerCase()}`}>
      <ModeTickerMain />
    </Match>
  </Switch>
);

export const SidebarThird: Component = () => (
  <SidebarSectionSecondary title={name}>
    <ModeCountdownLink />
    <ModeTickerLink />
  </SidebarSectionSecondary>
);
