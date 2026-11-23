import { render } from "@solidjs/web";

import { WebUI } from "./extensions/WebUI";

import "./style.css";

render(() => <WebUI />, document.getElementById("webui") as HTMLElement);
