import {
  mdiAlarm,
  mdiAlarmMultiple,
  mdiAlarmOff,
  mdiAlarmSnooze,
  mdiBellOffOutline,
  mdiNumeric1BoxMultipleOutline,
  mdiNumeric2BoxMultipleOutline,
} from "@mdi/js";
import { type Component, createSignal } from "solid-js";
import { Temporal } from "temporal-polyfill-lite";

import { Icon } from "../components/Icon";
import { Tooltip } from "../components/Tooltip";
import { name as ExtensionsName } from "../services/Extensions";
import { WebSocketWS } from "./WebSocket";

export const name = "Alarm";

type AlarmId = 1 | 2;

const createAlarm = () => {
  const [getEnabled, setEnabled] = createSignal<boolean>(false);
  const [getTime, setTime] = createSignal<Temporal.PlainTime>(
    Temporal.PlainTime.from({
      hour: 0,
      minute: 0,
    }),
  );
  return {
    getEnabled,
    setEnabled,
    getTime,
    setTime,
  };
};

const alarms = {
  1: createAlarm(),
  2: createAlarm(),
};

const icons = {
  1: mdiNumeric1BoxMultipleOutline,
  2: mdiNumeric2BoxMultipleOutline,
};

export const receiver = (json: {
  alarm1?: {
    enabled?: boolean;
    time?: string;
  };
  alarm2?: {
    enabled?: boolean;
    time?: string;
  };
}) => {
  ([1, 2] as const).forEach((id) => {
    const item = json?.[`alarm${id}`];
    if (!item) return;
    item.enabled !== undefined && alarms[id].setEnabled(item.enabled);
    item.time && alarms[id].setTime(Temporal.PlainTime.from(item.time));
  });
};

const getTime = (id: AlarmId) =>
  alarms[id].getTime().toString({
    smallestUnit: "minute",
  });

const handleUpdate = (id: AlarmId, next?: { enabled?: boolean; time?: string }) => {
  WebSocketWS.send(
    JSON.stringify({
      [name]: {
        [`alarm${id}`]: {
          enabled: next?.enabled ?? alarms[id].getEnabled(),
          time: next?.time ?? getTime(id),
        },
      },
    }),
  );
};

const handleEnable = (id: AlarmId) => {
  const enable = !alarms[id].getEnabled();
  alarms[id].setEnabled(enable);
  handleUpdate(id, { enabled: enable });
};

export const Actions: Component = () => (
  <div class="action grid-cols-[1fr_--spacing(12)_--spacing(12)]">
    <a href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}>
      <Icon class="mr-2" path={mdiAlarmMultiple} />
      {name}
    </a>
    {([1, 2] as const).map((id) => (
      <Tooltip
        text={`${alarms[id].getEnabled() ? "Turn off" : "Turn on"} ${getTime(id)} ${name.toLowerCase()}`}
      >
        <button
          class={`w-full ${alarms[id].getEnabled() ? "action-activated" : "action-deactivated"}`}
          onClick={() => handleEnable(id)}
          type="button"
        >
          <Icon path={alarms[id].getEnabled() ? mdiAlarm : mdiAlarmOff} />
        </button>
      </Tooltip>
    ))}
  </div>
);

export const Link: Component = () => (
  <Tooltip text={`${name} manager`}>
    <a class="link" href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}>
      <Icon class="mr-2" path={mdiAlarmMultiple} />
      {name}
    </a>
  </Tooltip>
);

export const MainThird: Component = () => {
  const handleAction = (action: string) => {
    WebSocketWS.send(
      JSON.stringify({
        [name]: {
          action: action,
        },
      }),
    );
  };

  const handleTime = (id: AlarmId, value: string) => {
    if (!value) return;
    const time = Temporal.PlainTime.from(value);
    alarms[id].setTime(time);
    handleUpdate(id, {
      time: time.toString({
        smallestUnit: "minute",
      }),
    });
  };

  return (
    <div class="main">
      <div class="space-y-3 p-5">
        <h2>{name}</h2>
        <div class="box">
          <div class="space-y-3">
            {([1, 2] as const).map((id) => (
              <div class="flex items-center gap-3">
                <Icon path={icons[id]} />
                <input
                  class="cursor-pointer"
                  onChange={(e) => handleTime(id, e.currentTarget.value)}
                  type="time"
                  value={getTime(id)}
                />
                <div class="flex-none">
                  <Tooltip
                    text={`${alarms[id].getEnabled() ? "Turn off" : "Turn on"} ${name.toLowerCase()} ${id}`}
                  >
                    <button
                      class={alarms[id].getEnabled() ? "action-activated" : "action-deactivated"}
                      onClick={() => handleEnable(id)}
                      type="button"
                    >
                      <Icon path={alarms[id].getEnabled() ? mdiAlarm : mdiAlarmOff} />
                    </button>
                  </Tooltip>
                </div>
              </div>
            ))}
          </div>
          <div class="hr mt-3" />
          <div class="flex gap-3">
            <Tooltip text="Snooze">
              <button
                class="mt-3 w-full action-neutral"
                disabled={!alarms[1].getEnabled() && !alarms[2].getEnabled()}
                onClick={() => handleAction("snooze")}
                type="button"
              >
                <Icon path={mdiAlarmSnooze} />
              </button>
            </Tooltip>
            <Tooltip text="Dismiss">
              <button
                class="mt-3 w-full action-negative"
                disabled={!alarms[1].getEnabled() && !alarms[2].getEnabled()}
                onClick={() => handleAction("dismiss")}
                type="button"
              >
                <Icon path={mdiBellOffOutline} />
              </button>
            </Tooltip>
          </div>
        </div>
      </div>
    </div>
  );
};
