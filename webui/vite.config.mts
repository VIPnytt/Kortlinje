import { mdiClockDigital } from "@mdi/js";
import tailwindcss from "@tailwindcss/vite";
import { defineConfig, loadEnv } from "vite";
import solidPlugin from "vite-plugin-solid";

const favicon = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="currentColor"><path d="${mdiClockDigital}"/><style>:root{color:white}@media(prefers-color-scheme:light){:root{color:black}}</style></svg>`;

export default defineConfig(({ mode }) => ({
    build: {
        rolldownOptions: {
            output: {
                entryFileNames: '[name].js',
                assetFileNames: '[name].[ext]',
            },
        },
    },
    plugins: [
        {
            name: "html-transform",
            enforce: "pre",
            transformIndexHtml: {
                handler() {
                    return [
                        {
                            tag: "title",
                            injectTo: "head",
                            children: loadEnv(mode, ".").VITE_NAME,
                        },
                    ];
                },
            },
            generateBundle() {
                this.emitFile({
                    type: "asset",
                    fileName: "favicon.svg",
                    source: favicon,
                });
            },
        },
        solidPlugin(),
        tailwindcss(),
        {
            name: "minify",
            enforce: "post",
            generateBundle(_, bundle) {
                for (const file of Object.values(bundle)) {
                    if (file.type === "asset") {
                        if (file.fileName === "index.css") {
                            file.source = file.source.toString().replace(/\/\*.*?\*\//g, "").trim();
                        } else if (file.fileName === "index.html") {
                            file.source = file.source.toString().replace(/\r?\n\s*/g, "").trim();
                        }
                    }
                }
            },
        },
    ],
}));
