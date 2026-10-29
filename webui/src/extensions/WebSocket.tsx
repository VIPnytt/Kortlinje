import { createEventSignal } from "@solid-primitives/event-listener";
import { createReconnectingWS, createWSState } from "@solid-primitives/websocket";
import { batch, createEffect, type ParentComponent } from "solid-js";

import { HOSTNAME } from "../config/constants";
import {
    EXTENSION_MESSAGE,
    EXTENSION_MIRROR,
    EXTENSION_PLAYLIST,
    EXTENSION_SIGNAL,
    MODE_DRAW,
    MODE_STREAM,
    MODE_WEATHER,
} from "../config/modules";
import { name as ModeDrawName, receiver as ModeDrawReceiver } from "../modes/Draw";
import { name as ModeStreamName, receiver as ModeStreamReceiver } from "../modes/Stream";
import { name as ModeTickerName, receiver as ModeTickerReceiver } from "../modes/Ticker";
import { name as ModeWeatherName, receiver as ModeWeatherReceiver } from "../modes/Weather";
import { name as DisplayName, receiver as DisplayReceiver } from "../services/Display";
import { name as FontsName, receiver as FontsReceiver } from "../services/Fonts";
import { name as ModesName, receiver as ModesReceiver } from "../services/Modes";
import { name as ExtensionMessageName, receiver as ExtensionMessageReceiver } from "./Message";
import { name as ExtensionMirrorName, receiver as ExtensionMirrorReceiver } from "./Mirror";
import { name as ExtensionPhotocellName, receiver as ExtensionPhotocellReceiver } from "./Photocell";
import { name as ExtensionPlaylistName, receiver as ExtensionPlaylistReceiver } from "./Playlist";
import { name as ExtensionSignalName, receiver as ExtensionSignalReceiver } from "./Signal";
import {
    receiver as ExtensionWebUIReceiver,
    Canonical as WebUICanonical,
} from "./WebUI";

export const name = "WebSocket";
export const WebSocketUrl = `ws://${HOSTNAME}.local/ws`;

export const WebSocketWS = createReconnectingWS(
    `ws://${import.meta.env.PROD ? location.hostname : `${HOSTNAME}.local`}/ws`,
);
export const WebSocketState = createWSState(WebSocketWS);

createEffect(() => {
    if (WebSocketState() === WebSocketWS.OPEN) {
        WebUICanonical();
    }
});

export const WebSocketmessages: ParentComponent = (props) => {
    const event = createEventSignal<{
        message: MessageEvent;
    }>(WebSocketWS, "message");
    createEffect(() => {
        const raw = event().data;
        if (raw instanceof Blob) {
            raw.arrayBuffer().then((buffer) => {
                ExtensionWebUIReceiver(new Uint8Array(buffer));
            });
            return;
        }
        const json = JSON.parse(raw);
        batch(() => {
            json[DisplayName] && DisplayReceiver(json[DisplayName]);
            json[ExtensionPhotocellName] && ExtensionPhotocellReceiver(json[ExtensionPhotocellName]);
            json[FontsName] && FontsReceiver(json[FontsName]);
            json[ModesName] && ModesReceiver(json[ModesName]);
            json[ModeTickerName] && ModeTickerReceiver(json[ModeTickerName]);
            EXTENSION_MESSAGE && json[ExtensionMessageName] && ExtensionMessageReceiver(json[ExtensionMessageName]);
            EXTENSION_MIRROR && json[ExtensionMirrorName] && ExtensionMirrorReceiver(json[ExtensionMirrorName]);
            EXTENSION_PLAYLIST && json[ExtensionPlaylistName] && ExtensionPlaylistReceiver(json[ExtensionPlaylistName]);
            EXTENSION_SIGNAL && json[ExtensionSignalName] && ExtensionSignalReceiver(json[ExtensionSignalName]);
            MODE_DRAW && json[ModeDrawName] && ModeDrawReceiver(json[ModeDrawName]);
            MODE_STREAM && json[ModeStreamName] && ModeStreamReceiver(json[ModeStreamName]);
            MODE_WEATHER && json[ModeWeatherName] && ModeWeatherReceiver(json[ModeWeatherName]);
        });
    });
    return props.children;
};
