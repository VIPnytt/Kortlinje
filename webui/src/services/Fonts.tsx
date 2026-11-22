import { createSignal } from "solid-js";

export const name = "Fonts";

const [getFonts, setFonts] = createSignal<string[]>([]);

export const FontsCollection = getFonts;

export const receiver = (json: { collection?: string[] }) => {
  json?.collection !== undefined && setFonts(json.collection);
};
