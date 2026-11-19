import { mdiProgressUpload } from "@mdi/js";
import type { Component } from "solid-js";

import { Icon } from "../components/Icon";
import { Tooltip } from "../components/Tooltip";
import { ARDUINO_VARIANT } from "../constants/secrets";
import { name as ExtensionsName } from "../services/Extensions";

export const name = "OTA";

export const Link: Component = () => (
  <Tooltip text="Over The Air updates">
    <a class="link" href={`#/${ExtensionsName.toLowerCase()}/${name.toLowerCase()}`}>
      <Icon class="mr-2" path={mdiProgressUpload} />
      {name}
    </a>
  </Tooltip>
);

export const MainThird: Component = () => (
  <div class="main">
    <MainComponent />
  </div>
);

/**
 * Renders PlatformIO espota setup instructions for the configured board and hostname.
 */
const MainComponent: Component = () => (
  <div class="space-y-3 p-5">
    <h2>{name}</h2>
    <div class="box">
      <div class="space-y-3">
        <div class="grid gap-3">
          <div class="text-sm">
            Configure&nbsp;
            <a
              class="italic"
              href={`https://github.com/VIPnytt/Kortlinje/blob/main/platformio.ini`}
              rel="noopener"
              target="_blank"
            >
              platformio.ini
            </a>
            &nbsp;for <span class="italic">over-the-air</span> updates:
          </div>
          <div class="font-mono text-sm  whitespace-nowrap">
            [env:{ARDUINO_VARIANT}]
            <br />
            board = {ARDUINO_VARIANT}
            <br />
            upload_protocol = espota
          </div>
        </div>
      </div>
    </div>
  </div>
);
