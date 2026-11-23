import { mdiFormatFont, mdiFormatText, mdiMessage } from "@mdi/js";
import { type Component, createSignal, For } from "solid-js";

import { Icon } from "../components/Icon";
import { Toast } from "../components/Toast";
import { Tooltip } from "../components/Tooltip";
import { WebSocketWS } from "../extensions/WebSocket";
import { SidebarSection } from "../extensions/WebUI";
import { name as DisplayName, DisplayPower } from "../services/Display";
import { FontsCollection } from "../services/Fonts";
import { ModesMode, name as ModesName } from "../services/Modes";

export const name = "Ticker";

const { toast } = Toast();

const [getFont, setFont] = createSignal<string>("");
const [getMessage, setMessage] = createSignal<string>("");

export const receiver = (json: { font?: string; message?: string }) => {
  json?.font !== undefined && setFont(json.font);
  json?.message !== undefined && setMessage(json.message);
};

export const Link: Component = () => (
  <Tooltip text="Text ticker">
    <a class="link" href={`#/${ModesName.toLowerCase()}/${name.toLowerCase()}`}>
      <Icon class="mr-2" path={mdiFormatText} />
      {name}
    </a>
  </Tooltip>
);

export const Sidebar: Component = () => {
  const handleFont = (font: string) => {
    setFont(font);
    WebSocketWS.send(
      JSON.stringify({
        [name]: {
          font,
        },
      }),
    );
  };

  const handleMessage = (message: string) => {
    setMessage(message);
    WebSocketWS.send(
      JSON.stringify({
        [name]: {
          message,
        },
      }),
    );
  };

  return (
    <SidebarSection>
      <div class="action grid-cols-[--spacing(4)_1fr]">
        <Icon path={mdiFormatText} />
        <input
          class="mt-1 w-full"
          maxlength="255"
          onChange={(e) => handleMessage(e.currentTarget.value)}
          placeholder="Message"
          spellcheck="true"
          type="text"
          value={getMessage()}
        />
      </div>
      <div class="action grid-cols-[--spacing(4)_1fr]">
        <Icon path={mdiFormatFont} />
        <Tooltip text="Character availability may vary">
          <select
            class="w-full"
            value={getFont()}
            onChange={(e) => handleFont(e.currentTarget.value)}
          >
            <For each={FontsCollection()}>{(font) => <option>{font}</option>}</For>
          </select>
        </Tooltip>
      </div>
    </SidebarSection>
  );
};

export const Main: Component = () => {
  const handleSave = () => {
    WebSocketWS.send(
      JSON.stringify({
        [name]: {
          font: getFont(),
          message: getMessage(),
        },
        ...(!DisplayPower() && {
          [DisplayName]: {
            power: true,
          },
        }),
        ...(ModesMode() !== name && {
          [ModesName]: {
            mode: name,
          },
        }),
      }),
    );
    toast(`${name} saved`);
  };

  return (
    <div class="main">
      <div class="space-y-3 p-5">
        <h2>{name}</h2>
        <div class="box">
          <div class="space-y-3">
            <div class="action grid-cols-[--spacing(4)_1fr]">
              <Icon path={mdiFormatText} />
              <input
                class="w-full"
                maxlength="255"
                onInput={(e) => setMessage(e.currentTarget.value)}
                placeholder="Message"
                spellcheck="true"
                type="text"
                value={getMessage()}
              />
            </div>
            <div class="action grid-cols-[--spacing(4)_1fr]">
              <Icon path={mdiFormatFont} />
              <Tooltip text="Character availability may vary">
                <select
                  class="w-full"
                  onChange={(e) => setFont(e.currentTarget.value)}
                  value={getFont()}
                >
                  <For each={FontsCollection()}>{(font) => <option>{font}</option>}</For>
                </select>
              </Tooltip>
            </div>
            <button
              class="action-positive mt-3 w-full"
              disabled={!getFont() || !getMessage()}
              onClick={handleSave}
              type="button"
            >
              <Icon class="mr-2" path={mdiMessage} />
              Display
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};
