import { mdiTarget, mdiTimerSand, mdiTimerSandComplete } from "@mdi/js";
import Cookies from "js-cookie";
import { type Component, createSignal } from "solid-js";
import { Temporal } from "temporal-polyfill-lite";

import { Icon } from "../components/Icon";
import { Toast } from "../components/Toast";
import { Tooltip } from "../components/Tooltip";
import { WebSocketWS } from "../extensions/WebSocket";
import { SidebarSection } from "../extensions/WebUI";
import { name as DisplayName, DisplayPower } from "../services/Display";
import { ModesMode, name as ModesName } from "../services/Modes";

export const name = "Countdown";

const [getMinutes, setMinutes] = createSignal<number>(
  parseInt(Cookies.get(`${name}.minutes`) ?? "10", 10),
);
const [getSeconds, setSeconds] = createSignal<number>(
  parseInt(Cookies.get(`${name}.seconds`) ?? "0", 10),
);
const [getTimestamp, setTimestamp] = createSignal<string>(
  Temporal.Now.plainTimeISO()
    .add({
      minutes: getMinutes(),
      seconds: getSeconds(),
    })
    .toString({
      smallestUnit: "minute",
    }),
);

export const receiver = (json: { event?: string; timestamp?: string }) => {
  json?.event !== undefined && event(json.event);
  json?.timestamp !== undefined && setTimestamp(json.timestamp);
};

const { toast } = Toast();

const event = (action: string) => {
  switch (action) {
    case "done":
      toast(`${name} ended`, 60e3);
      break;
  }
};

const handleRelative = () => {
  WebSocketWS.send(
    JSON.stringify({
      [name]: {
        time: getMinutes() * 60 + getSeconds(),
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
  toast(`${name} started`);
  Cookies.set(`${name}.minutes`, getMinutes().toString(), {
    expires: 365,
  });
  Cookies.set(`${name}.seconds`, getSeconds().toString(), {
    expires: 365,
  });
};

const handleRelativeInput = (minutes: number, seconds: number, send = false) => {
  const m = Number.isNaN(minutes) ? 0 : Math.min(99, Math.max(0, minutes));
  const s = Number.isNaN(seconds) ? 0 : Math.min(59, Math.max(0, seconds));
  setMinutes(m);
  setSeconds(s);
  setTimestamp(
    Temporal.Now.plainTimeISO()
      .add({
        minutes: m,
        seconds: s,
      })
      .toString({
        smallestUnit: "second",
      }),
  );
  if (send) {
    handleRelative();
  }
};

const handleAbsoluteInput = (timestamp: string, send = false) => {
  if (!timestamp) {
    return;
  }
  const target = Temporal.PlainTime.from(timestamp);
  const now = Temporal.Now.plainTimeISO().round({
    roundingMode: "trunc",
    smallestUnit: "minute",
  });
  let totalMinutes = now.until(target, {
    largestUnit: "minutes",
  }).minutes;
  if (totalMinutes < 0) {
    totalMinutes += 1440;
  }
  setMinutes(Math.min(99, totalMinutes));
  setSeconds(totalMinutes > 99 ? 59 : 0);
  setTimestamp(target.toString());
  if (send) {
    WebSocketWS.send(
      JSON.stringify({
        [name]: {
          timestamp: target.toString(),
        },
      }),
    );
  }
};

export const Actions: Component = () => (
  <div class="action">
    <a href={`#/${ModesName.toLowerCase()}/${name.toLowerCase()}`}>
      <Icon class="mr-2" path={mdiTimerSand} />
      {name}
    </a>
    <Tooltip text={`Start ${name.toLowerCase()}`}>
      <button
        class={`w-full ${ModesMode() === name ? "action-activated" : "action-deactivated"}`}
        disabled={ModesMode() === name}
        onclick={handleRelative}
        type="button"
      >
        <Icon path={ModesMode() === name ? mdiTimerSandComplete : mdiTimerSand} />
      </button>
    </Tooltip>
  </div>
);

export const Link: Component = () => (
  <Tooltip text="Countdown timer">
    <a class="link" href={`#/${ModesName.toLowerCase()}/${name.toLowerCase()}`}>
      <Icon class="mr-2" path={mdiTimerSand} />
      {name}
    </a>
  </Tooltip>
);

export const Sidebar: Component = () => (
  <SidebarSection>
    <div class="space-y-3">
      <div class="action grid-cols-[--spacing(4)_1fr_1fr]">
        <Icon path={mdiTimerSand} />
        <div class="relative">
          <input
            class="pr-6 text-right w-full"
            max="99"
            min="0"
            name="minutes"
            onchange={(e) => handleRelativeInput(e.currentTarget.valueAsNumber, getSeconds(), true)}
            type="number"
            value={getMinutes()}
          />
          <span class="absolute text-content-alt-light dark:text-content-alt-dark right-3 top-1/2 -translate-y-1/2 text-sm">
            m
          </span>
        </div>
        <div class="relative">
          <input
            class="pr-6 text-right w-full"
            max="59"
            min="0"
            name="seconds"
            onchange={(e) => handleRelativeInput(getMinutes(), e.currentTarget.valueAsNumber, true)}
            type="number"
            value={getSeconds()}
          />
          <span class="absolute text-content-alt-light dark:text-content-alt-dark right-3 top-1/2 -translate-y-1/2 text-sm">
            s
          </span>
        </div>
      </div>
      <div class="action grid-cols-[--spacing(4)_1fr]">
        <Icon path={mdiTarget} />
        <input
          class="w-full"
          min={Temporal.Now.plainTimeISO().toString({
            smallestUnit: "minute",
          })}
          onchange={(e) => handleAbsoluteInput(e.currentTarget.value, true)}
          type="time"
          value={getTimestamp().slice(0, 5)}
        />
      </div>
    </div>
  </SidebarSection>
);

export const Main: Component = () => (
  <div class="main">
    <div class="space-y-3 p-5">
      <h2>{name}</h2>
      <div class="box">
        <div class="space-y-3">
          <div class="action grid-cols-[--spacing(4)_1fr_1fr]">
            <Icon path={mdiTimerSand} />
            <div class="relative">
              <input
                class="pr-6 text-right w-full"
                max="99"
                min="0"
                name="minutes"
                oninput={(e) => handleRelativeInput(e.currentTarget.valueAsNumber, getSeconds())}
                type="number"
                value={getMinutes()}
              />
              <span class="absolute text-content-alt-light dark:text-content-alt-dark right-3 top-1/2 -translate-y-1/2 text-sm">
                m
              </span>
            </div>
            <div class="relative">
              <input
                class="pr-6 text-right w-full"
                max="59"
                min="0"
                name="seconds"
                oninput={(e) => handleRelativeInput(getMinutes(), e.currentTarget.valueAsNumber)}
                type="number"
                value={getSeconds()}
              />
              <span class="absolute text-content-alt-light dark:text-content-alt-dark right-3 top-1/2 -translate-y-1/2 text-sm">
                s
              </span>
            </div>
          </div>
          <div class="action grid-cols-[--spacing(4)_1fr]">
            <Icon path={mdiTarget} />
            <input
              class="w-full"
              min={Temporal.Now.plainTimeISO().toString({
                smallestUnit: "minute",
              })}
              oninput={(e) => handleAbsoluteInput(e.currentTarget.value)}
              type="time"
              value={getTimestamp().slice(0, 5)}
            />
          </div>
          <button
            class="action-positive mt-3 w-full"
            disabled={!getMinutes() && !getSeconds()}
            onclick={handleRelative}
            type="button"
          >
            <Icon class="mr-2" path={mdiTimerSand} />
            Start
          </button>
        </div>
      </div>
    </div>
  </div>
);
