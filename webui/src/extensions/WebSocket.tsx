import { createEventSignal } from "@solid-primitives/event-listener";
import { createReconnectingWS, createWSState } from "@solid-primitives/websocket";
import { batch, createEffect, type ParentComponent } from "solid-js";

import { DEVICE_HOSTNAME } from "../constants/secrets";
import { name as ModeAnimationName, receiver as ModeAnimationReceiver } from "../modes/Animation";
import { name as ModeCountdownName, receiver as ModeCountdownReceiver } from "../modes/Countdown";
import { name as ModeDrawName, receiver as ModeDrawReceiver } from "../modes/Draw";
import { name as ModeTickerName, receiver as ModeTickerReceiver } from "../modes/Ticker";
import { name as ModeWeatherName, receiver as ModeWeatherReceiver } from "../modes/Weather";
import {
  name as ConnectivityName,
  receiver as ConnectivityReceiver,
} from "../services/Connectivity";
import { name as DisplayName, receiver as DisplayReceiver } from "../services/Display";
import { name as FontsName, receiver as FontsReceiver } from "../services/Fonts";
import { name as ModesName, receiver as ModesReceiver } from "../services/Modes";
import { name as ExtensionAlarmName, receiver as ExtensionAlarmReceiver } from "./Alarm";
import { name as ExtensionMessageName, receiver as ExtensionMessageReceiver } from "./Message";
import { name as ExtensionMirrorName, receiver as ExtensionMirrorReceiver } from "./Mirror";
import {
  name as ExtensionPhotocellName,
  receiver as ExtensionPhotocellReceiver,
} from "./Photocell";
import { name as ExtensionPlaylistName, receiver as ExtensionPlaylistReceiver } from "./Playlist";
import { receiver as ExtensionWebUIReceiver, Canonical as WebUICanonical } from "./WebUI";

export const name = "WebSocket";
export const WebSocketUrl = `ws://${DEVICE_HOSTNAME}.local/ws`;

export const WebSocketWS = createReconnectingWS(
  `ws://${import.meta.env.PROD ? location.hostname : `${DEVICE_HOSTNAME}.local`}/ws`,
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
      json[ConnectivityName] && ConnectivityReceiver(json[ConnectivityName]);
      json[DisplayName] && DisplayReceiver(json[DisplayName]);
      json[FontsName] && FontsReceiver(json[FontsName]);
      json[ModesName] && ModesReceiver(json[ModesName]);
      json[ExtensionAlarmName] && ExtensionAlarmReceiver(json[ExtensionAlarmName]);
      json[ExtensionMessageName] && ExtensionMessageReceiver(json[ExtensionMessageName]);
      json[ExtensionMirrorName] && ExtensionMirrorReceiver(json[ExtensionMirrorName]);
      json[ExtensionPhotocellName] && ExtensionPhotocellReceiver(json[ExtensionPhotocellName]);
      json[ExtensionPlaylistName] && ExtensionPlaylistReceiver(json[ExtensionPlaylistName]);
      json[ModeAnimationName] && ModeAnimationReceiver(json[ModeAnimationName]);
      json[ModeCountdownName] && ModeCountdownReceiver(json[ModeCountdownName]);
      json[ModeDrawName] && ModeDrawReceiver(json[ModeDrawName]);
      json[ModeTickerName] && ModeTickerReceiver(json[ModeTickerName]);
      json[ModeWeatherName] && ModeWeatherReceiver(json[ModeWeatherName]);
    });
  });
  return props.children;
};
