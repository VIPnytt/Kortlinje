import { mdiBriefcase } from "@mdi/js";
import { type Component, Match, Switch } from "solid-js";

import { Icon } from "../components/Icon";
import {
    EXTENSION_MESSAGE,
    EXTENSION_MIRROR,
    EXTENSION_PLAYLIST,
} from "../config/modules";
import {
    Actions as ExtensionAlarmActions,
    Link as ExtensionAlarmLink,
    name as ExtensionAlarmName,
    MainThird as ExtensionAlarmThird
} from "../extensions/Alarm";
import {
    Link as ExtensionMessageLink,
    name as ExtensionMessageName,
    MainThird as ExtensionMessageThird,
} from "../extensions/Message";
import {
    Actions as ExtensionMirrorActions,
    Link as ExtensionMirrorLink,
    name as ExtensionMirrorName,
    MainThird as ExtensionMirrorThird,
} from "../extensions/Mirror";
import { Link as ExtensionOtaLink, name as ExtensionOtaName, MainThird as ExtensionOtaThird } from "../extensions/Ota";
import {
    Actions as ExtensionPhotocellActions,
    Link as ExtensionPhotocellLink,
    name as ExtensionPhotocellName,
    MainThird as ExtensionPhotocellThird
} from "../extensions/Photocell";
import {
    Actions as ExtensionPlaylistActions,
    Link as ExtensionPlaylistLink,
    name as ExtensionPlaylistName,
    MainThird as ExtensionPlaylistThird,
} from "../extensions/Playlist";
import { SidebarSectionSecondary, MainSecondary as WebUIMain, WebUIPath } from "../extensions/WebUI";
import { name as ExtensionsName } from "./Extensions";

export const name = "Extensions";

export const Footer: Component = () => (
    <>
        <div class="hr mt-auto mb-3" />
        <ExtensionAlarmActions />
    </>
);

export const SidebarSecondary: Component = () => (
    <SidebarSectionSecondary title={name}>
        <div class="space-y-3">
            <ExtensionAlarmActions />
            <ExtensionPhotocellActions />
            {EXTENSION_MIRROR && <ExtensionMirrorActions />}
            {EXTENSION_PLAYLIST && <ExtensionPlaylistActions />}
            <a
                class="link"
                href={`#/${name.toLowerCase()}`}
            >
                <Icon
                    class="mr-2"
                    path={mdiBriefcase}
                />
                More {name.toLowerCase()}...
            </a>
        </div>
    </SidebarSectionSecondary>
);

export const MainThird: Component = () => (
    <Switch fallback={<WebUIMain />}>
        <Match when={WebUIPath() === `/${ExtensionsName.toLowerCase()}/${ExtensionAlarmName.toLowerCase()}`}>
            <ExtensionAlarmThird />
        </Match>
        <Match when={WebUIPath() === `/${ExtensionsName.toLowerCase()}/${ExtensionOtaName.toLowerCase()}`}>
            <ExtensionOtaThird />
        </Match>
        <Match when={WebUIPath() === `/${ExtensionsName.toLowerCase()}/${ExtensionPhotocellName.toLowerCase()}`}>
            <ExtensionPhotocellThird />
        </Match>
        {EXTENSION_MESSAGE && (
            <Match when={WebUIPath() === `/${ExtensionsName.toLowerCase()}/${ExtensionMessageName.toLowerCase()}`}>
                <ExtensionMessageThird />
            </Match>
        )}
        {EXTENSION_MIRROR && (
            <Match when={WebUIPath() === `/${ExtensionsName.toLowerCase()}/${ExtensionMirrorName.toLowerCase()}`}>
                <ExtensionMirrorThird />
            </Match>
        )}
        {EXTENSION_PLAYLIST && (
            <Match when={WebUIPath() === `/${ExtensionsName.toLowerCase()}/${ExtensionPlaylistName.toLowerCase()}`}>
                <ExtensionPlaylistThird />
            </Match>
        )}
    </Switch>
);

export const SidebarThird: Component = () => (
    <SidebarSectionSecondary title={name}>
        <ExtensionAlarmLink />
        <ExtensionOtaLink />
        <ExtensionPhotocellLink />
        {EXTENSION_MESSAGE && <ExtensionMessageLink />}
        {EXTENSION_MIRROR && <ExtensionMirrorLink />}
        {EXTENSION_PLAYLIST && <ExtensionPlaylistLink />}
    </SidebarSectionSecondary>
);
