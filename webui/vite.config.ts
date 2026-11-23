import { mdiClockDigital } from "@mdi/js";
import solidPlugin from "@solidjs/vite-plugin";
import tailwindcss from "@tailwindcss/vite";
import { defineConfig } from "vite";
import { DEVICE_NAME } from "./src/constants/secrets.ts";

export default defineConfig(() => ({
  build: {
    rolldownOptions: {
      output: {
        entryFileNames: "[name].js",
        assetFileNames: "[name].[ext]",
      },
    },
  },
  plugins: [
    solidPlugin(),
    tailwindcss(),
    {
      name: "minify",
      enforce: "post",
      generateBundle(_, bundle) {
        const decoder = new TextDecoder();
        for (const file of Object.values(bundle)) {
          if (file.type !== "asset") {
            continue;
          }
          const source =
            typeof file.source === "string" ? file.source : decoder.decode(file.source);
          switch (file.fileName) {
            case "favicon.svg":
              file.source = source
                .replace(/\s+/g, " ")
                .replace(/(?<=>)\s+(?=<)|\s+(?=\/?>)/g, "")
                .replace(/(<path\b[^>]*\sd=")[^"]*"/, `$1${mdiClockDigital}"`)
                .trim();
              break;
            case "index.css":
              file.source = source.replace(/\/\*.*?\*\//g, "").trim();
              break;
            case "index.html":
              file.source = source
                .replace(/\s+/g, " ")
                .replace(/(?<=>)\s+(?=<)|\s+(?=\/?>)/g, "")
                .replace(
                  /(<title\b[^>]*>)[\s\S]*?(<\/title>)/,
                  (_, open, close) => `${open}${DEVICE_NAME}${close}`,
                )
                .trim();
              break;
          }
        }
      },
    },
  ],
}));
