import { mdiMirrorRectangle, mdiPlay, mdiStop } from "@mdi/js";
import { type Component, createSignal } from "solid-js";

import { Icon } from "../components/Icon";
import { Tooltip } from "../components/Tooltip";
import { name as StreamName } from "../modes/Stream";
import { name as ExtensionsName } from "../services/Extensions";
import { ModesMode } from "../services/Modes";
import { WebSocketWS } from "./WebSocket";

export const name = "Mirror";

const [getEnabled, setEnabled] = createSignal<boolean>(false);

export const receiver = (json: { active?: boolean }) => {
  json?.active !== undefined && setEnabled(json.active);
};

const handleEnable = () => {
  const enable = !getEnabled();
  setEnabled(enable);
  WebSocketWS.send(
    JSON.stringify({
      [name]: {
        enabled: enable,
      },
    }),
  );
};

export const Actions: Component = () => (
  <div class="action">
    <a href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}>
      <Icon class="mr-2" path={mdiMirrorRectangle} />
      {name}
    </a>
    <Tooltip text={`${getEnabled() ? "Stop" : "Start"} ${name.toLowerCase()}ing`}>
      <button
        class={`w-full ${getEnabled() ? "action-activated" : "action-deactivated"}`}
        disabled={!getEnabled() && ModesMode() === StreamName}
        onClick={handleEnable}
        type="button"
      >
        <Icon path={getEnabled() ? mdiStop : mdiPlay} />
      </button>
    </Tooltip>
  </div>
);

export const Link: Component = () => (
  <a class="link" href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}>
    <Icon class="mr-2" path={mdiMirrorRectangle} />
    {name}
  </a>
);

export const MainThird: Component = () => (
  <div class="main">
    <div class="space-y-3 p-5">
      <h2>{name}</h2>
      <div class="box">
        <div class="space-y-3">
          <div class="text-sm">Duplicate the display to other devices.</div>
          <button
            class={`mt-3 w-full ${getEnabled() ? "action-activated" : "action-deactivated"}`}
            disabled={!getEnabled() && ModesMode() === StreamName}
            onClick={handleEnable}
            type="button"
          >
            <Icon class="mr-2" path={getEnabled() ? mdiStop : mdiPlay} />
            {getEnabled() ? `${name.toLowerCase()}ing` : name.toLowerCase()}
          </button>
        </div>
      </div>
    </div>
  </div>
);
