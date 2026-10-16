import { mdiBriefcase } from "@mdi/js";
import { type Component, Match, Switch } from "solid-js";

import { Icon } from "../components/Icon";
import {
    EXTENSION_MESSAGE,
    EXTENSION_MIRROR,
    EXTENSION_OTA,
    EXTENSION_PLAYLIST,
} from "../config/modules";
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
        {EXTENSION_MESSAGE ? <div class="hr mt-auto mb-3" /> : <div class="mt-auto" />}
        {EXTENSION_MESSAGE && <ExtensionMessageLink />}
    </>
);

export const SidebarSecondary: Component = () => (
    <SidebarSectionSecondary title={name}>
        <div class="space-y-3">
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
        {EXTENSION_OTA && (
            <Match when={WebUIPath() === `/${ExtensionsName.toLowerCase()}/${ExtensionOtaName.toLowerCase()}`}>
                <ExtensionOtaThird />
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
        {EXTENSION_MESSAGE && <ExtensionMessageLink />}
        {EXTENSION_MIRROR && <ExtensionMirrorLink />}
        {EXTENSION_OTA && <ExtensionOtaLink />}
        {EXTENSION_PLAYLIST && <ExtensionPlaylistLink />}
    </SidebarSectionSecondary>
);
