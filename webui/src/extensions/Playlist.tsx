import { mdiDelete, mdiPlay, mdiPlaylistPlay, mdiPlus, mdiStop } from "@mdi/js";
import Cookies from "js-cookie";
import { type Component, createSignal, For, Index } from "solid-js";

import { Icon } from "../components/Icon";
import { Tooltip } from "../components/Tooltip";
import { name as ExtensionsName } from "../services/Extensions";
import { ModesList, name as ModesName } from "../services/Modes";
import { WebSocketWS } from "./WebSocket";

export const name = "Playlist";

interface Item {
    duration: number;
    mode: string;
}

const [getDuration, setDuration] = createSignal<number>(parseInt(Cookies.get(`${name}.duration`) || "60", 10));
const [getEnabled, setEnabled] = createSignal<boolean>(false);
const [getSchedule, setSchedule] = createSignal<Item[]>([]);

export const PlaylistEnabled = getEnabled;

export const receiver = (json: { enabled?: boolean; schedule?: Item[] }) => {
    json?.enabled !== undefined && setEnabled(json.enabled);
    json?.schedule !== undefined && setSchedule(json.schedule);
};

const handleEnabled = () => {
    setEnabled(!getEnabled());
    WebSocketWS.send(
        JSON.stringify({
            [name]: {
                enabled: getEnabled(),
                schedule: getSchedule(),
            },
        }),
    );
};

export const Actions: Component = () => (
    <div class="action">
        <a href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}>
            <Icon
                class="mr-2"
                path={mdiPlaylistPlay}
            />
            {name}
        </a>
        <Tooltip text={`${getEnabled() ? "Stop" : "Play"} ${name.toLowerCase()}`}>
            <button
                class={`w-full ${getEnabled() ? "action-activated" : "action-deactivated"}`}
                disabled={!getSchedule().length}
                onclick={handleEnabled}
                type="button"
            >
                <Icon path={getEnabled() ? mdiStop : mdiPlay} />
            </button>
        </Tooltip>
    </div>
);

export const Link: Component = () => (
    <a
        class="link"
        href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}
    >
        <Icon
            class="mr-2"
            path={mdiPlaylistPlay}
        />
        {name}
    </a>
);

export const MainThird: Component = () => {
    const handleAdd = () => {
        setSchedule([
            ...getSchedule(),
            {
                duration: getDuration(),
                mode: ModesList()[Math.round(Math.random() * (ModesList().length - 1))],
            },
        ]);
    };

    const handleDuration = (index: number, duration: number) => {
        setDuration(duration);
        setSchedule(
            getSchedule().map((item, i) =>
                i === index
                    ? {
                          ...item,
                          duration,
                      }
                    : item,
            ),
        );
        Cookies.set(`${name}.duration`, getDuration().toString(), {
            expires: 365,
        });
    };

    const handleMode = (index: number, name: string) => {
        setSchedule(
            getSchedule().map((item, i) =>
                i === index
                    ? {
                          ...item,
                          mode: name,
                      }
                    : item,
            ),
        );
    };

    const handleRemove = (index: number) => {
        setSchedule(getSchedule().filter((_, idx) => idx !== index));
    };

    return (
        <div class="main">
            <div class="space-y-3 p-5">
                <h2>{name}</h2>
                <div class="box">
                    <div class="space-y-3">
                        {getSchedule().length && (
                            <>
                                <h3>{ModesName}</h3>
                                <Index each={getSchedule()}>
                                    {(item, index) => (
                                        <div class="flex items-center gap-3">
                                            <select
                                                class="w-full"
                                                disabled={getEnabled()}
                                                onChange={(e) => handleMode(index, e.currentTarget.value)}
                                                value={item().mode}
                                            >
                                                <For each={ModesList()}>{(mode) => <option>{mode}</option>}</For>
                                            </select>
                                            <div class="relative">
                                                <input
                                                    class="text-right pr-6 w-24"
                                                    disabled={getEnabled()}
                                                    max={2 ** 16 - 1}
                                                    min="5"
                                                    onInput={(e) =>
                                                        handleDuration(index, e.currentTarget.valueAsNumber)
                                                    }
                                                    step="5"
                                                    type="number"
                                                    value={item().duration}
                                                />
                                                <span class="absolute text-content-alt-light dark:text-content-alt-dark right-3 top-1/2 -translate-y-1/2 text-sm">
                                                    s
                                                </span>
                                            </div>
                                            <div class="flex-none">
                                                <button
                                                    class="action-negative"
                                                    disabled={getEnabled() || getSchedule().length <= 2}
                                                    onclick={() => handleRemove(index)}
                                                    type="button"
                                                >
                                                    <Icon path={mdiDelete} />
                                                </button>
                                            </div>
                                        </div>
                                    )}
                                </Index>
                            </>
                        )}
                        <div class="grid grid-cols-2 gap-3 mt-3">
                            <Tooltip text="Add mode">
                                <button
                                    class={`action-neutral w-full ${getSchedule().length < 2 ? `bg-neutral-light dark:enabled:bg-neutral-dark text-interactive-light dark:text-content-dark` : ""}`}
                                    disabled={getEnabled()}
                                    onclick={handleAdd}
                                    type="button"
                                >
                                    <Icon path={mdiPlus} />
                                </button>
                            </Tooltip>
                            <Tooltip text={`${getEnabled() ? "Stop" : "Play"} ${name.toLowerCase()}`}>
                                <button
                                    class={`w-full ${getEnabled() ? "action-activated" : "action-positive"}`}
                                    disabled={getSchedule().length < 2}
                                    onclick={handleEnabled}
                                    type="button"
                                >
                                    <Icon path={getEnabled() ? mdiStop : mdiPlay} />
                                </button>
                            </Tooltip>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
};
